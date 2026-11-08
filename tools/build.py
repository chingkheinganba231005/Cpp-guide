#!/usr/bin/env python3
"""Build the C++ guide: src/*.md -> chapters/*.html, with every C++ example compiled and run.

    python3 tools/build.py            # build every chapter
    python3 tools/build.py 05 06      # build chapters whose file name starts with these prefixes

Code fences (info string after the language):
    ```cpp run                       compile with g++ -std=c++23 and run; show the real output
    ```cpp run stdin="3 4"           feed standard input
    ```cpp run flags="-fsanitize=address -g"   extra compiler flags
    ```cpp run expect=fail           the compile must FAIL; show the compiler's error (teaching)
    ```cpp run expect=crash          the program must exit non-zero; show what happened
    ```cpp predict                   like run, but the output is hidden until the reader guesses
    ```cpp file=vec2.hpp             save as a file for later blocks; shown, not compiled alone
    ```cpp run with=vec2.hpp,vec2.cpp   compile together with saved files (only .cpp are passed to g++)
    ```cpp norun                     show only (e.g. CUDA, or code needing libraries we do not have)
    ```cpp run title="..."           caption above the block
Other blocks:
    :::kind Title  ...  :::          boxes: eli5, python, warn, tip, deep, exercise (solution inside <details>)
    ::quiz ... ::                    "? question", "- wrong", "+ right", "> explanation"
"""
import base64, hashlib, html, json, os, re, shlex, subprocess, sys, tempfile, urllib.parse
import markdown
from pygments import highlight
from pygments.lexers import get_lexer_by_name, TextLexer
from pygments.formatters import HtmlFormatter

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC, OUT, EX, CACHE = (os.path.join(ROOT, d) for d in ("src", "chapters", "examples", ".cache"))
TOC = json.load(open(os.path.join(ROOT, "tools", "toc.json"), encoding="utf-8"))
BASE_FLAGS = ["-std=c++23", "-O2", "-Wall", "-Wextra", "-pthread"]
GODBOLT_COMPILER = "g132"
os.makedirs(CACHE, exist_ok=True)


def chapters():
    out = []
    for part in TOC["parts"]:
        for ch in part["chapters"]:
            out.append(dict(ch, part=part["title"], part_n=part["n"]))
    return out


def parse_info(info):
    toks = shlex.split(info)
    lang = toks[0] if toks else "text"
    opts = {}
    for t in toks[1:]:
        if "=" in t:
            k, v = t.split("=", 1); opts[k] = v
        else:
            opts[t] = True
    return lang, opts


def hl(code, lang):
    try:
        lexer = get_lexer_by_name({"cpp": "cpp", "cu": "cuda", "cuda": "cuda", "sh": "bash", "console": "text"}.get(lang, lang))
    except Exception:
        lexer = TextLexer()
    return highlight(code, lexer, HtmlFormatter(nowrap=True))


def godbolt(code, flags):
    state = {"sessions": [{"id": 1, "language": "c++", "source": code,
                           "compilers": [{"id": GODBOLT_COMPILER, "options": " ".join(flags)}],
                           "executors": [{"compiler": {"id": GODBOLT_COMPILER, "options": " ".join(flags)}, "arguments": "", "stdin": ""}]}]}
    b = base64.b64encode(json.dumps(state).encode()).decode()
    return "https://godbolt.org/clientstate/" + urllib.parse.quote(b, safe="")


def compile_and_run(code, opts, files, chap_id, idx):
    flags = BASE_FLAGS + (shlex.split(opts["flags"]) if "flags" in opts else [])
    with_files = [f.strip() for f in opts.get("with", "").split(",") if f.strip()]
    stdin = opts.get("stdin", "").encode().decode("unicode_escape") if "stdin" in opts else ""
    key = hashlib.sha256(json.dumps([code, flags, stdin, [files.get(f, "") for f in with_files], opts.get("expect", "")]).encode()).hexdigest()[:20]
    cpath = os.path.join(CACHE, key + ".json")
    if os.path.exists(cpath):
        return json.load(open(cpath))
    with tempfile.TemporaryDirectory() as d:
        for f in with_files:
            open(os.path.join(d, f), "w").write(files[f])
        open(os.path.join(d, "main.cpp"), "w").write(code)
        srcs = ["main.cpp"] + [f for f in with_files if f.endswith(".cpp")]
        cp = subprocess.run(["g++"] + flags + srcs + ["-o", "prog"], cwd=d, capture_output=True, text=True, timeout=180)
        res = {"compile_rc": cp.returncode, "compile_err": cp.stderr.replace(d + "/", ""), "stdout": "", "stderr": "", "rc": None}
        expect = opts.get("expect")
        if cp.returncode != 0:
            if expect != "fail":
                raise SystemExit(f"[{chap_id} #{idx}] compile failed unexpectedly:\n{cp.stderr}\n--- code ---\n{code}")
        else:
            if expect == "fail":
                raise SystemExit(f"[{chap_id} #{idx}] expected a compile error but it compiled:\n{code}")
            try:
                rp = subprocess.run((["./prog"] if "-fsanitize" in " ".join(flags) else ["stdbuf", "-oL", "-eL", "./prog"]), cwd=d, input=stdin, capture_output=True, text=True, timeout=int(opts.get("timeout", 60)),
                                    env=dict(os.environ, ASAN_OPTIONS="detect_leaks=1:abort_on_error=0", UBSAN_OPTIONS="print_stacktrace=0"))
                res.update(stdout=rp.stdout, stderr=rp.stderr.replace(d + "/", ""), rc=rp.returncode)
                imgs = []
                for name in [x for x in opts.get("image", "").split(",") if x]:
                    from PIL import Image
                    src = os.path.join(d, name)
                    if not os.path.exists(src):
                        raise SystemExit(f"[{chap_id} #{idx}] expected the program to write {name}")
                    out_name = f"{chap_id}-{os.path.splitext(name)[0]}.png"
                    os.makedirs(os.path.join(OUT, "img"), exist_ok=True)
                    Image.open(src).save(os.path.join(OUT, "img", out_name))
                    imgs.append(out_name)
                res["images"] = imgs
            except subprocess.TimeoutExpired:
                raise SystemExit(f"[{chap_id} #{idx}] timed out")
            if rp.returncode != 0 and expect != "crash":
                raise SystemExit(f"[{chap_id} #{idx}] program exited with {rp.returncode}:\n{rp.stderr}\n{rp.stdout}\n--- code ---\n{code}")
            if rp.returncode == 0 and expect == "crash":
                raise SystemExit(f"[{chap_id} #{idx}] expected a crash / non-zero exit but it succeeded")
    json.dump(res, open(cpath, "w"))
    return res


def trim(text, n=60):
    lines = text.rstrip("\n").split("\n")
    if len(lines) > n:
        lines = lines[:n] + [f"... ({len(lines) - n} more lines)"]
    return "\n".join(lines)


def render_code(lang, opts, code, files, chap, idx):
    code = code.rstrip("\n") + "\n"
    title = opts.get("title")
    head = f'<div class="code-title">{html.escape(title)}</div>' if title else ""
    if "file" in opts:
        files[opts["file"]] = code
        head = f'<div class="code-title file">📄 {html.escape(opts["file"])}</div>'
        return f'<div class="code">{head}<pre class="src"><code>{hl(code, lang)}</code></pre></div>'
    if lang not in ("cpp",) or opts.get("norun") or not (opts.get("run") or opts.get("predict")):
        tag = ' <span class="tag">not run here</span>' if opts.get("norun") else ""
        cls = "src" if lang in ("cpp", "cuda", "cu") else "src plain"
        return f'<div class="code">{head.replace("</div>", tag + "</div>") if head else (f"<div class=code-title>{tag}</div>" if tag else "")}<pre class="{cls}"><code>{hl(code, lang)}</code></pre></div>'
    res = compile_and_run(code, opts, files, chap["id"], idx)
    # save the example for readers
    exdir = os.path.join(EX, chap["id"]); os.makedirs(exdir, exist_ok=True)
    open(os.path.join(exdir, f"ex{idx:02d}.cpp"), "w").write(code)
    flags = BASE_FLAGS + (shlex.split(opts["flags"]) if "flags" in opts else [])
    cmd = "g++ " + " ".join(f for f in flags if f not in ("-pthread",) or "thread" in code) + " main.cpp" + "".join(" " + f for f in opts.get("with", "").split(",") if f.endswith(".cpp")) + " -o prog"
    links = (f'<a class="gb" href="{godbolt(code, flags)}" target="_blank" rel="noopener">Open in Compiler Explorer ↗</a>'
             if not opts.get("with") else "")
    out = []
    if opts.get("expect") == "fail":
        out.append(f'<div class="out err"><div class="out-h">Compiler error (this is the point of the example)</div><pre>{html.escape(trim(res["compile_err"], 40))}</pre></div>')
    else:
        if res["compile_err"].strip():
            out.append(f'<div class="out warn"><div class="out-h">Compiler warnings</div><pre>{html.escape(trim(res["compile_err"], 25))}</pre></div>')
        body = html.escape(trim(res["stdout"])) if res["stdout"] else '<span class="dim">(no output)</span>'
        if opts.get("stdin"):
            body = f'<span class="dim">input: {html.escape(opts["stdin"])}</span>\n' + body
        if res["stderr"].strip():
            body += f'\n<span class="stderr">{html.escape(trim(res["stderr"], 30))}</span>'
        if res["rc"]:
            sig = {-6: "SIGABRT (the program aborted)", -11: "SIGSEGV (segmentation fault: invalid memory access)", -8: "SIGFPE (arithmetic error)"}
            body += f'\n<span class="dim">[{"killed by signal " + sig.get(res["rc"], str(-res["rc"])) if res["rc"] < 0 else "exit code " + str(res["rc"])}]</span>'
        cls = "out predict" if opts.get("predict") else "out"
        hdr = "Output" if not opts.get("predict") else "Output"
        inner = f'<div class="out-h">{hdr}</div><pre>{body}</pre>'
        if opts.get("predict"):
            inner = f'<button class="reveal" type="button">Predict the output first, then click to check</button><div class="hid" hidden>{inner}</div>'
        out.append(f'<div class="{cls}">{inner}</div>')
        for im in res.get("images", []):
            out.append(f'<div class="out img"><div class="out-h">Image written by the program</div><img src="img/{im}" alt="Image rendered by this example" loading="lazy"></div>')
    return (f'<div class="code run">{head}<pre class="src"><code>{hl(code, "cpp")}</code></pre>'
            f'<div class="code-bar"><span class="cmd">$ {html.escape(cmd)}</span>{links}</div>{"".join(out)}</div>')


def render_quiz(body):
    qs, cur = [], None
    for line in body.strip().split("\n"):
        line = line.rstrip()
        if line.startswith("? "):
            cur = {"q": line[2:], "opts": [], "why": ""}; qs.append(cur)
        elif line.startswith(("- ", "+ ")) and cur is not None:
            cur["opts"].append((line[0] == "+", line[2:]))
        elif line.startswith("> ") and cur is not None:
            cur["why"] += line[2:] + " "
    md = lambda s: markdown.markdown(s, extensions=["md_in_html"])[3:-4]
    parts = []
    for q in qs:
        opts = "".join(f'<li><button type="button" class="qopt" data-ok="{1 if ok else 0}">{md(t)}</button></li>' for ok, t in q["opts"])
        parts.append(f'<div class="q"><p class="qq">{md(q["q"])}</p><ol>{opts}</ol><p class="why" hidden>{md(q["why"])}</p></div>')
    return f'<div class="quiz"><div class="quiz-h">Check yourself</div>{"".join(parts)}</div>'


BOX = {"eli5": "In plain words", "python": "Coming from Python", "warn": "Watch out", "tip": "Tip", "deep": "Under the hood", "exercise": "Exercise"}


def md_to_html(text, chap):
    files, store = {}, []
    idx = [0]

    def put(s):
        store.append(s); return f"\n\nXXPLACEHOLDER{len(store) - 1}XX\n\n"

    def code_sub(m):
        lang, opts = parse_info(m.group(1))
        idx[0] += 1
        return put(render_code(lang, opts, m.group(2), files, chap, idx[0]))
    text = re.sub(r"^```([^\n]*)\n(.*?)^```\s*$", code_sub, text, flags=re.S | re.M)
    text = re.sub(r"^::quiz\s*\n(.*?)^::\s*$", lambda m: put(render_quiz(m.group(1))), text, flags=re.S | re.M)

    def box_sub(m):
        kind, title = m.group(1), (m.group(2) or "").strip()
        inner = m.group(3)
        if kind == "exercise" and "---solution---" in inner:
            q, a = inner.split("---solution---", 1)
            inner = q + '\n<details markdown="1"><summary>Show a solution</summary>\n\n' + a + "\n\n</details>\n"
        lab = BOX.get(kind, kind) + (": " + title if title else "")
        return f'\n<div class="box {kind}" markdown="1">\n<p class="box-h">{html.escape(lab)}</p>\n\n{inner}\n</div>\n'
    for _ in range(2):
        text = re.sub(r"^:::(\w+)([^\n]*)\n(.*?)^:::\s*$", box_sub, text, flags=re.S | re.M)
    body = markdown.markdown(text, extensions=["tables", "md_in_html", "attr_list", "sane_lists"])
    for i, s in enumerate(store):
        body = body.replace(f"<p>XXPLACEHOLDER{i}XX</p>", s).replace(f"XXPLACEHOLDER{i}XX", s)
    return body, idx[0]


def slugify(s):
    return re.sub(r"[^a-z0-9]+", "-", re.sub(r"<[^>]+>", "", s).lower()).strip("-")


def build_chapter(ch, all_ch, template):
    src = open(os.path.join(SRC, ch["id"] + ".md"), encoding="utf-8").read()
    body, n_code = md_to_html(src, ch)
    toc = []
    def hid(m):
        s = slugify(m.group(2)); toc.append((m.group(1), s, re.sub(r"<[^>]+>", "", m.group(2))))
        return f'<h{m.group(1)} id="{s}">{m.group(2)}</h{m.group(1)}>'
    body = re.sub(r"<h([23])>(.*?)</h\1>", hid, body)
    i = all_ch.index(ch)
    prev, nxt = (all_ch[i - 1] if i else None), (all_ch[i + 1] if i + 1 < len(all_ch) else None)
    side = []
    for part in TOC["parts"]:
        items = "".join(f'<li><a href="{c["id"]}.html"{" class=cur" if c["id"] == ch["id"] else ""}><span class="num">{c["n"]}</span>{html.escape(c["title"])}</a></li>' for c in part["chapters"])
        side.append(f'<details{" open" if part["title"] == ch["part"] else ""}><summary>{html.escape(part["n"] + " · " + part["title"])}</summary><ol>{items}</ol></details>')
    onpage = "".join(f'<li class="l{l}"><a href="#{s}">{html.escape(t)}</a></li>' for l, s, t in toc if l == "2")
    page = template
    for k, v in {
        "TITLE": html.escape(ch["title"]), "NUM": ch["n"], "PART": html.escape(ch["part_n"] + " · " + ch["part"]), "SUMMARY": html.escape(ch["summary"]),
        "BODY": body, "SIDE": "".join(side), "ONPAGE": onpage,
        "PREV": f'<a class="prev" href="{prev["id"]}.html"><small>Previous</small>{prev["n"]} · {html.escape(prev["title"])}</a>' if prev else '<a class="prev" href="../index.html"><small>Back to</small>Contents</a>',
        "NEXT": f'<a class="next" href="{nxt["id"]}.html"><small>Next</small>{nxt["n"]} · {html.escape(nxt["title"])}</a>' if nxt else '<a class="next" href="../index.html"><small>Back to</small>Contents</a>',
        "CHID": ch["id"],
    }.items():
        page = page.replace("%%" + k + "%%", v)
    open(os.path.join(OUT, ch["id"] + ".html"), "w", encoding="utf-8").write(page)
    return n_code


def build_index(all_ch):
    tpl = open(os.path.join(ROOT, "tools", "index_template.html"), encoding="utf-8").read()
    parts = []
    for part in TOC["parts"]:
        cards = []
        for c in part["chapters"]:
            ready = os.path.exists(os.path.join(SRC, c["id"] + ".md"))
            inner = f'<span class="cn">{c["n"]}</span><span class="ct">{html.escape(c["title"])}</span><span class="cs">{html.escape(c["summary"])}</span>'
            cards.append(f'<li><a class="card" href="chapters/{c["id"]}.html" data-ch="{c["id"]}">{inner}</a></li>' if ready else f'<li><div class="card soon">{inner}<span class="soon-t">coming soon</span></div></li>')
        parts.append(f'<section class="part" id="part-{part["n"].lower()}"><div class="part-h"><span class="pn">Part {part["n"]}</span><h2>{html.escape(part["title"])}</h2><p>{html.escape(part["blurb"])}</p></div><ol class="cards">{"".join(cards)}</ol></section>')
    first = next(c for c in all_ch if os.path.exists(os.path.join(SRC, c["id"] + ".md")))
    page = tpl.replace("%%PARTS%%", "".join(parts)).replace("%%FIRST%%", f'chapters/{first["id"]}.html')
    open(os.path.join(ROOT, "index.html"), "w", encoding="utf-8").write(page)


def main():
    os.makedirs(OUT, exist_ok=True)
    template = open(os.path.join(ROOT, "tools", "template.html"), encoding="utf-8").read()
    all_ch = [c for c in chapters() if os.path.exists(os.path.join(SRC, c["id"] + ".md"))]
    want = sys.argv[1:]
    total = 0
    for ch in all_ch:
        if want and not any(ch["id"].startswith(w) for w in want):
            continue
        n = build_chapter(ch, all_ch, template); total += n
        print(f"built {ch['id']}: {n} code blocks")
    build_index(all_ch)
    print("index built;", total, "code blocks")


if __name__ == "__main__":
    main()
