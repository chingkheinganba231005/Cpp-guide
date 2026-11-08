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

  // Data race: step two threads through counter++ = load, add, store
  W.race = function (w) {
    const PROG = ["load counter into register", "add 1 to register", "store register into counter"];
    let st;
    const reset = () => { st = { counter: 0, t: [{ pc: 0, reg: null, done: 0 }, { pc: 0, reg: null, done: 0 }], log: [] }; render(); };
    const step = (k) => {
      const t = st.t[k];
      if (t.pc === 0) t.reg = st.counter;
      else if (t.pc === 1) t.reg += 1;
      else { st.counter = t.reg; t.done++; }
      st.log.push(`${k ? "B" : "A"}: ${PROG[t.pc]}${t.pc === 0 ? ` (reads ${t.reg})` : t.pc === 2 ? ` (writes ${st.counter})` : ` (now ${t.reg})`}`);
      t.pc = (t.pc + 1) % 3; render();
    };
    const box = h("div");
    const render = () => {
      const col = (k) => h("div", { class: "region" }, h("div", { class: "region-h" }, `Thread ${k ? "B" : "A"}: counter++  (done ${st.t[k].done}×)`),
        PROG.map((p, i) => h("div", { class: "cell" + (st.t[k].pc === i ? " hi" : "") }, h("span", { class: "nm" }, (st.t[k].pc === i ? "▶ " : "  ") + p), h("span"), h("span"))),
        h("div", { class: "cell" }, h("span", { class: "nm" }, "register"), h("span"), h("span", { class: "vl" }, st.t[k].reg ?? "–")),
        h("button", { class: "wbtn pri", type: "button", onclick: () => step(k), style: "margin-top:8px" }, `Step thread ${k ? "B" : "A"}`));
      const expected = st.t[0].done + st.t[1].done;
      box.replaceChildren(h("div", { class: "mem" }, col(0), col(1)),
        h("div", { class: "mem-note" }, h("b", null, `Shared counter = ${st.counter}`), `  ·  increments completed = ${expected}`,
          expected && st.counter !== expected ? h("b", { style: "color:var(--red)" }, `  ·  an update was LOST: ${expected - st.counter} increment(s) vanished`) : ""),
        h("div", { class: "mem-out" }, st.log.slice(-8).join("\n") || "Try: A load, B load, A add, B add, A store, B store."));
    };
    w.append(h("div", { class: "widget-h" }, w.dataset.title || "A data race, step by step"), h("div", { class: "wctl" }, h("button", { class: "wbtn", type: "button", onclick: () => reset() }, "Reset")), box);
    reset();
  };

  // Cache simulator: walk a 2D array row by row or column by column
  W.cache = function (w) {
    const R = 8, C = 8, LINE = 4, WAYS = 4;            // 4 elements per cache line, a tiny cache of 4 lines
    let order = "row", i = 0, hits = 0, misses = 0, cache = [], timer = null;
    const grid = h("div", { style: `display:grid;grid-template-columns:repeat(${C},1fr);gap:3px;max-width:360px` });
    const cells = [];
    for (let k = 0; k < R * C; k++) { const c = h("div", { class: "bit", style: "width:auto;height:30px;font-size:11px" }, k); cells.push(c); grid.append(c); }
    const stats = h("div", { class: "mem-note" }), cachev = h("div", { class: "mem-out" });
    const idx = (n) => (order === "row" ? n : (n % R) * C + Math.floor(n / R));
    const paint = () => {
      cells.forEach((c, k) => { const line = Math.floor(k / LINE); c.style.background = cache.includes(line) ? "var(--blue-soft)" : ""; c.style.outline = ""; });
      if (i > 0) { const k = idx(i - 1); cells[k].style.outline = "2px solid var(--accent)"; }
      stats.innerHTML = `Visited ${i} of ${R * C} elements · <b style="color:var(--green)">${hits} cache hits</b> · <b style="color:var(--red)">${misses} misses</b> (each miss = a slow trip to main memory)`;
      cachev.textContent = "cache lines held: " + (cache.map((l) => `[${l * LINE}–${l * LINE + LINE - 1}]`).join(" ") || "none");
    };
    const step = () => {
      if (i >= R * C) { stop(); return; }
      const line = Math.floor(idx(i) / LINE);
      if (cache.includes(line)) { hits++; cache = cache.filter((x) => x !== line).concat(line); }
      else { misses++; cache.push(line); if (cache.length > WAYS) cache.shift(); }
      i++; paint();
    };
    const stop = () => { clearInterval(timer); timer = null; };
    const start = (o) => { stop(); order = o; i = 0; hits = 0; misses = 0; cache = []; paint(); timer = setInterval(step, 90); };
    w.append(h("div", { class: "widget-h" }, w.dataset.title || "Cache simulator"),
      h("div", { class: "wctl" }, h("button", { class: "wbtn pri", type: "button", onclick: () => start("row") }, "Walk row by row"),
        h("button", { class: "wbtn", type: "button", onclick: () => start("col") }, "Walk column by column")),
      h("div", { class: "mem" }, grid, h("div", null, h("p", { class: "dim" }, `An 8×8 array stored row by row. Memory moves into the cache in lines of ${LINE} elements, and this toy cache holds ${WAYS} lines (the least recently used is evicted). Blue = currently cached.`), stats, cachev)));
    paint();
  };

  // Order book: price-time priority matching, the same rules as the chapter's C++ engine
  W.orderbook = function (w) {
    let book = { bid: [], ask: [] }, trades = [], nextId = 1;
    const seed = () => {
      book = { bid: [], ask: [] }; trades = []; nextId = 1;
      [[99, 30], [99, 10], [98, 50], [97, 20]].forEach(([p, q]) => rest("bid", p, q));
      [[101, 25], [101, 15], [102, 40], [104, 60]].forEach(([p, q]) => rest("ask", p, q));
    };
    const rest = (side, price, qty) => book[side].push({ id: nextId++, price, qty, t: nextId });
    const best = (side) => book[side].slice().sort((a, b) => (side === "bid" ? b.price - a.price : a.price - b.price) || a.t - b.t)[0];
    const submit = (side, type, price, qty) => {
      const opp = side === "buy" ? "ask" : "bid", fills = [];
      while (qty > 0) {
        const o = best(opp);
        if (!o) break;
        const crosses = type === "market" || (side === "buy" ? price >= o.price : price <= o.price);
        if (!crosses) break;
        const q = Math.min(qty, o.qty);
        fills.push({ price: o.price, qty: q, maker: o.id });
        o.qty -= q; qty -= q;
        if (!o.qty) book[opp] = book[opp].filter((x) => x !== o);
      }
      fills.forEach((f) => trades.unshift(`${side.toUpperCase()} ${f.qty} @ ${f.price} (against order #${f.maker})`));
      if (qty > 0 && type === "limit") rest(side === "buy" ? "bid" : "ask", price, qty);
      msg.textContent = fills.length ? `Filled ${fills.reduce((a, f) => a + f.qty, 0)} in ${fills.length} trade(s)` + (qty > 0 ? (type === "limit" ? `; ${qty} rests in the book at ${price}` : `; ${qty} unfilled (no more liquidity)`) : "")
                                     : type === "limit" ? `No match: order rests in the book at ${price}` : "No liquidity on the other side";
      render();
    };
    const ladder = h("div"), tradeBox = h("div", { class: "mem-out" }), msg = h("div", { class: "mem-note" });
    const levels = (side) => {
      const m = new Map();
      book[side].forEach((o) => m.set(o.price, (m.get(o.price) || []).concat(o)));
      return [...m.entries()].sort((a, b) => b[0] - a[0]);
    };
    const render = () => {
      const row = (price, orders, side) => h("div", { class: "cell" + (side === "ask" ? "" : " ptr") },
        h("span", { class: "nm", style: `color:var(--${side === "ask" ? "red" : "green"})` }, `${side === "ask" ? "ASK" : "BID"} ${price}`),
        h("span", { class: "ad" }, orders.map((o) => `#${o.id}:${o.qty}`).join("  ")), h("span", { class: "vl" }, orders.reduce((a, o) => a + o.qty, 0)));
      const asks = levels("ask"), bids = levels("bid"), bb = bids[0], ba = asks[asks.length - 1];
      ladder.replaceChildren(h("div", { class: "region" }, h("div", { class: "region-h" }, "Order book (orders queue in time order at each price)"),
        asks.map(([p, os]) => row(p, os, "ask")),
        h("div", { class: "cell" }, h("span", { class: "nm" }, bb && ba ? `spread ${ba[0] - bb[0]}` : "—"), h("span"), h("span")),
        bids.map(([p, os]) => row(p, os, "bid"))));
      tradeBox.textContent = trades.slice(0, 8).join("\n") || "No trades yet.";
    };
    const side = h("select", { "aria-label": "Side" }, h("option", { value: "buy" }, "Buy"), h("option", { value: "sell" }, "Sell"));
    const type = h("select", { "aria-label": "Order type" }, h("option", { value: "limit" }, "Limit"), h("option", { value: "market" }, "Market"));
    const price = h("input", { type: "number", value: "101", step: "1", "aria-label": "Price", style: "width:6em" });
    const qty = h("input", { type: "number", value: "30", min: "1", step: "1", "aria-label": "Quantity", style: "width:6em" });
    w.append(h("div", { class: "widget-h" }, w.dataset.title || "Order book"),
      h("div", { class: "wctl" }, side, type, h("label", null, "price ", price), h("label", null, "qty ", qty),
        h("button", { class: "wbtn pri", type: "button", onclick: () => { const q = Math.max(1, Math.floor(+qty.value)); submit(side.value, type.value, Math.floor(+price.value), q); } }, "Submit order"),
        h("button", { class: "wbtn", type: "button", onclick: () => { seed(); msg.textContent = ""; render(); } }, "Reset")),
      h("div", { class: "mem" }, ladder, h("div", null, msg, h("div", { class: "widget-h", style: "margin-top:10px" }, "Recent trades"), tradeBox)));
    seed(); render();
  };

  window.GUC = { W, h, $, $$ };
  document.addEventListener("DOMContentLoaded", () => {});
  $$("[data-widget]").forEach((w) => { const f = W[w.dataset.widget]; if (f) { w.classList.add("widget"); try { f(w); } catch (e) { console.error(e); w.append(h("p", null, "This interactive part failed to load.")); } } });
})();
