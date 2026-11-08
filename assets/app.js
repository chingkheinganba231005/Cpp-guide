/* Ground Up C++ — page behaviour and interactive widgets. Plain JS, no libraries. */
(function () {
  "use strict";
  const $ = (s, r) => (r || document).querySelector(s), $$ = (s, r) => [...(r || document).querySelectorAll(s)];
  const store = { get(k, d) { try { const v = localStorage.getItem(k); return v == null ? d : JSON.parse(v); } catch (e) { return d; } },
                  set(k, v) { try { localStorage.setItem(k, JSON.stringify(v)); } catch (e) {} } };
  function h(tag, attrs, ...kids) {
    const e = document.createElement(tag);
    for (const [k, v] of Object.entries(attrs || {})) {
      if (v == null || v === false) continue;
      if (k === "class") e.className = v; else if (k.startsWith("on")) e.addEventListener(k.slice(2), v); else e.setAttribute(k, v === true ? "" : v);
    }
    kids.flat(Infinity).forEach((c) => { if (c != null && c !== false) e.append(c.nodeType ? c : document.createTextNode(String(c))); });
    return e;
  }

  // theme
  const tb = $("#theme-btn");
  if (tb) tb.addEventListener("click", () => {
    const cur = document.documentElement.dataset.theme || (matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light");
    const nxt = cur === "dark" ? "light" : "dark"; document.documentElement.dataset.theme = nxt; try { localStorage.setItem("guc-theme", nxt); } catch (e) {}
  });
  // mobile chapter menu
  const mb = $("#menu-btn"), rail = $("#rail");
  if (mb && rail) {
    mb.addEventListener("click", () => { const o = rail.classList.toggle("open"); mb.setAttribute("aria-expanded", o); });
    rail.addEventListener("click", (e) => { if (e.target.closest("a")) rail.classList.remove("open"); });
  }
  // reading progress
  const pb = $("#progress");
  if (pb) addEventListener("scroll", () => { const d = document.documentElement; pb.style.width = (100 * d.scrollTop / Math.max(1, d.scrollHeight - d.clientHeight)) + "%"; }, { passive: true });
  // copy buttons
  $$("pre.src").forEach((pre) => {
    const b = h("button", { class: "copy", type: "button" }, "Copy");
    b.addEventListener("click", () => {
      const text = pre.innerText.replace(/\nCopy$/, "");
      const done = () => { b.textContent = "Copied"; setTimeout(() => (b.textContent = "Copy"), 1200); };
      if (navigator.clipboard) navigator.clipboard.writeText(text).then(done, () => {}); });
    pre.append(b);
  });
  // predict-the-output
  $$(".reveal").forEach((b) => b.addEventListener("click", () => { b.nextElementSibling.hidden = false; b.remove(); }));
  // quizzes
  $$(".q").forEach((q) => {
    const why = $(".why", q);
    $$(".qopt", q).forEach((o) => o.addEventListener("click", () => {
      $$(".qopt", q).forEach((x) => x.classList.remove("ok", "no"));
      const ok = o.dataset.ok === "1"; o.classList.add(ok ? "ok" : "no");
      if (why) why.hidden = !ok;
    }));
  });
  // chapter progress
  const done = store.get("guc-done", {}), ch = document.body.dataset.ch, cb = $("#done");
  if (cb && ch) { cb.checked = !!done[ch]; cb.addEventListener("change", () => { done[ch] = cb.checked; store.set("guc-done", done); paintDone(); }); }
  function paintDone() {
    $$(".rail a").forEach((a) => { const id = (a.getAttribute("href") || "").replace(".html", ""); a.classList.toggle("done", !!done[id]); });
    $$(".card[data-ch]").forEach((c) => c.classList.toggle("done", !!done[c.dataset.ch]));
  }
  paintDone();

  // ---------------------------------------------------------------- widgets
  const W = {};

  // Bits: see how an integer or a float is stored
  W.bits = function (w) {
    const inp = h("input", { type: "text", value: w.dataset.value || "13", "aria-label": "Number", size: 14 });
    const out = h("div");
    const row = (lab, bits, groups) => h("div", { class: "bitrow" }, h("div", { class: "lab" }, lab),
      h("div", { class: "bits" }, bits.split("").map((b, i) => h("span", { class: "bit" + (b === "1" ? " one" : "") + (groups && groups(i) ? " " + groups(i) : "") + (i && i % 8 === 0 ? " gap" : "") }, b))));
    const render = () => {
      const s = inp.value.trim(), x = Number(s);
      if (s === "" || Number.isNaN(x)) { out.replaceChildren(h("p", null, "Type a number, e.g. 13, -1, 300 or 0.1")); return; }
      const parts = [];
      if (Number.isInteger(x) && Math.abs(x) < 2 ** 31) {
        const i8 = ((x % 256) + 256) % 256, u = (x >>> 0).toString(2).padStart(32, "0");
        parts.push(row(`int8_t  (1 byte): stores ${i8 > 127 ? i8 - 256 : i8}${(i8 > 127 ? i8 - 256 : i8) !== x ? "  ← it does not fit, so it wraps!" : ""}`, i8.toString(2).padStart(8, "0"), (i) => (i === 0 ? "sign" : "")));
        parts.push(row(`int32_t (4 bytes): stores ${x}`, u, (i) => (i === 0 ? "sign" : "")));
      }
      const f = new Float32Array([x]), b = new Uint32Array(f.buffer)[0].toString(2).padStart(32, "0");
      parts.push(row(`float (4 bytes): stores ${f[0].toPrecision(9)}${f[0] !== x ? "  ← not exactly " + s + "!" : ""}`, b, (i) => (i === 0 ? "sign" : i < 9 ? "exp" : "")));
      parts.push(h("p", { class: "dim" }, "Red outline = sign bit. Blue underline = exponent bits of the float. Negative integers use two's complement: −1 is all ones."));
      out.replaceChildren(...parts);
    };
    inp.addEventListener("input", render);
    w.append(h("div", { class: "widget-h" }, w.dataset.title || "How numbers are stored"), h("div", { class: "wctl" }, h("label", null, "Number ", inp)), out);
    render();
  };

  // Memory stepper: walk through a program and watch the stack and the heap
  W.memstep = function (w) {
    const data = JSON.parse($("script", w).textContent);
    let i = 0;
    const code = h("div", { class: "mem-code" }), state = h("div", { class: "mem-state" }), note = h("div", { class: "mem-note" }), outp = h("div", { class: "mem-out" });
    const prev = h("button", { class: "wbtn", type: "button" }, "◀ Back"), next = h("button", { class: "wbtn pri", type: "button" }, "Step ▶"), reset = h("button", { class: "wbtn", type: "button" }, "Restart");
    const counter = h("span", { class: "dim" });
    data.code.forEach((ln, k) => code.append(h("div", { "data-l": k + 1 }, h("span", { class: "ln" }, k + 1), ln)));
    const cell = (v) => h("div", { class: "cell" + (v.to ? " ptr" : "") + (v.hi ? " hi" : "") + (v.dead ? " dead" : "") + (v.leak ? " leak" : "") },
      h("span", { class: "nm" }, v.name + (v.type ? " : " + v.type : "")), h("span", { class: "ad" }, v.addr || ""), h("span", { class: "vl" }, v.val + (v.to ? "  → " + v.to : "")));
    const render = () => {
      const s = data.steps[i];
      $$("div", code).forEach((d) => d.classList.toggle("on", +d.dataset.l === s.line));
      const stack = h("div", { class: "region" }, h("div", { class: "region-h" }, "Stack (automatic, freed when a function returns)"),
        (s.stack || []).length ? s.stack.slice().reverse().map((f) => h("div", { class: "frame" }, h("div", { class: "frame-h" }, f.fn + "()"), f.vars.map(cell))) : h("div", { class: "dim" }, "empty"));
      const parts = [stack];
      if (data.heap !== false) parts.push(h("div", { class: "region" }, h("div", { class: "region-h" }, "Heap (manual: you ask, you give back)"), (s.heap || []).length ? s.heap.map(cell) : h("div", { class: "dim" }, "empty")));
      state.replaceChildren(...parts);
      note.innerHTML = s.note || "";
      outp.textContent = s.out || "";
      counter.textContent = `step ${i + 1} of ${data.steps.length}`;
      prev.disabled = i === 0; next.disabled = i === data.steps.length - 1;
    };
    prev.addEventListener("click", () => { if (i > 0) { i--; render(); } });
    next.addEventListener("click", () => { if (i < data.steps.length - 1) { i++; render(); } });
    reset.addEventListener("click", () => { i = 0; render(); });
    w.append(h("div", { class: "widget-h" }, w.dataset.title || "Step through memory"), h("div", { class: "wctl" }, prev, next, reset, counter),
      h("div", { class: "mem" }, code, h("div", null, state)), note, h("div", { class: "widget-h", style: "margin-top:10px" }, "Program output"), outp);
    render();
  };

  window.GUC = { W, h, $, $$ };
  document.addEventListener("DOMContentLoaded", () => {});
  $$("[data-widget]").forEach((w) => { const f = W[w.dataset.widget]; if (f) { w.classList.add("widget"); try { f(w); } catch (e) { console.error(e); w.append(h("p", null, "This interactive part failed to load.")); } } });
})();
