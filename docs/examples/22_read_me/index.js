var k;
k || (k = typeof Module !== 'undefined' ? Module : {});
var aa = Object.assign({}, k),
  ba = "./this.program",
  ca = (a, b) => {
    throw b;
  },
  l = "";
"undefined" != typeof document && document.currentScript && (l = document.currentScript.src);
l = 0 !== l.indexOf("blob:") ? l.substr(0, l.replace(/[?#].*/, "").lastIndexOf("/") + 1) : "";
var da = k.print || console.log.bind(console),
  p = k.printErr || console.error.bind(console);
Object.assign(k, aa);
aa = null;
k.thisProgram && (ba = k.thisProgram);
k.quit && (ca = k.quit);
var ea;
k.wasmBinary && (ea = k.wasmBinary);
"object" != typeof WebAssembly && r("no native wasm support detected");
var fa,
  v = !1,
  ha,
  y,
  A,
  ia,
  ja,
  C,
  D,
  E,
  F;
function ka() {
  var a = fa.buffer;
  k.HEAP8 = y = new Int8Array(a);
  k.HEAP16 = ia = new Int16Array(a);
  k.HEAPU8 = A = new Uint8Array(a);
  k.HEAPU16 = ja = new Uint16Array(a);
  k.HEAP32 = C = new Int32Array(a);
  k.HEAPU32 = D = new Uint32Array(a);
  k.HEAPF32 = E = new Float32Array(a);
  k.HEAPF64 = F = new Float64Array(a);
}
var la = [],
  ma = [],
  na = [],
  oa = [];
function pa() {
  var a = k.preRun.shift();
  la.unshift(a);
}
var G = 0,
  qa = null,
  ra = null;
function r(a) {
  var _k$onAbort, _k;
  (_k$onAbort = (_k = k).onAbort) === null || _k$onAbort === void 0 || _k$onAbort.call(_k, a);
  a = "Aborted(" + a + ")";
  p(a);
  v = !0;
  ha = 1;
  throw new WebAssembly.RuntimeError(a + ". Build with -sASSERTIONS for more info.");
}
var sa = a => a.startsWith("data:application/octet-stream;base64,"),
  H;
H = "index.wasm";
if (!sa(H)) {
  var ta = H;
  H = k.locateFile ? k.locateFile(ta, l) : l + ta;
}
function ua(a) {
  if (a == H && ea) return new Uint8Array(ea);
  throw "both async and sync fetching of the wasm failed";
}
function va(a) {
  return ea || "function" != typeof fetch ? Promise.resolve().then(() => ua(a)) : fetch(a, {
    credentials: "same-origin"
  }).then(b => {
    if (!b.ok) throw "failed to load wasm binary file at '" + a + "'";
    return b.arrayBuffer();
  }).catch(() => ua(a));
}
function wa(a, b, c) {
  return va(a).then(d => WebAssembly.instantiate(d, b)).then(d => d).then(c, d => {
    p(`failed to asynchronously prepare wasm: ${d}`);
    r(d);
  });
}
function xa(a, b) {
  var c = H;
  ea || "function" != typeof WebAssembly.instantiateStreaming || sa(c) || "function" != typeof fetch ? wa(c, a, b) : fetch(c, {
    credentials: "same-origin"
  }).then(d => WebAssembly.instantiateStreaming(d, a).then(b, function (e) {
    p(`wasm streaming compile failed: ${e}`);
    p("falling back to ArrayBuffer instantiation");
    return wa(c, a, b);
  }));
}
var I,
  ya,
  za = {
    39046: () => {
      document.getElementById("canvas").style.width = "100%";
    },
    39106: () => {
      document.getElementById("canvas").style.height = "100%";
    },
    39167: () => {
      document.getElementById("canvas").style.aspectRatio = "2 / 1";
    },
    39234: () => {
      document.getElementById("canvas").style.cursor = "none";
    },
    39295: () => {
      document.getElementById("canvas").style.cursor = "auto";
    },
    39356: a => {
      K(window.location.href, A, a, 256);
    }
  };
function Aa(a) {
  this.name = "ExitStatus";
  this.message = `Program terminated with exit(${a})`;
  this.status = a;
}
var Ba = a => {
    for (; 0 < a.length;) a.shift()(k);
  },
  Ca = k.noExitRuntime || !0,
  Da = a => 0 === a % 4 && (0 !== a % 100 || 0 === a % 400),
  Ea = [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335],
  Fa = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334],
  Ga = a => {
    for (var b = 0, c = 0; c < a.length; ++c) {
      var d = a.charCodeAt(c);
      127 >= d ? b++ : 2047 >= d ? b += 2 : 55296 <= d && 57343 >= d ? (b += 4, ++c) : b += 3;
    }
    return b;
  },
  K = (a, b, c, d) => {
    if (!(0 < d)) return 0;
    var e = c;
    d = c + d - 1;
    for (var f = 0; f < a.length; ++f) {
      var g = a.charCodeAt(f);
      if (55296 <= g && 57343 >= g) {
        var m = a.charCodeAt(++f);
        g = 65536 + ((g & 1023) << 10) | m & 1023;
      }
      if (127 >= g) {
        if (c >= d) break;
        b[c++] = g;
      } else {
        if (2047 >= g) {
          if (c + 1 >= d) break;
          b[c++] = 192 | g >> 6;
        } else {
          if (65535 >= g) {
            if (c + 2 >= d) break;
            b[c++] = 224 | g >> 12;
          } else {
            if (c + 3 >= d) break;
            b[c++] = 240 | g >> 18;
            b[c++] = 128 | g >> 12 & 63;
          }
          b[c++] = 128 | g >> 6 & 63;
        }
        b[c++] = 128 | g & 63;
      }
    }
    b[c] = 0;
    return c - e;
  },
  Ha = a => {
    var b = Ga(a) + 1,
      c = M(b);
    c && K(a, A, c, b);
    return c;
  },
  Ia = [],
  La = a => {
    var b = Ja();
    a = a();
    Ka(b);
    return a;
  },
  Ma = 0,
  N = [];
function Na(a) {
  function b(f, g) {
    if (f.length != g.length) return !1;
    for (var m in f) if (f[m] != g[m]) return !1;
    return !0;
  }
  var c = Oa,
    d;
  for (d in N) {
    var e = N[d];
    if (e.ac == c && b(e.ec, a)) return;
  }
  N.push({
    ac: c,
    tc: 1,
    ec: a
  });
  N.sort((f, g) => f.tc < g.tc);
}
function Pa(a) {
  for (var b = 0; b < N.length; ++b) N[b].ac == a && (N.splice(b, 1), --b);
}
function Qa() {
  if (navigator.userActivation ? navigator.userActivation.isActive : Ma && Ra.Ub) for (var a = 0; a < N.length; ++a) {
    var b = N[a];
    N.splice(a, 1);
    --a;
    b.ac.apply(null, b.ec);
  }
}
var Sa = [];
function Ta(a) {
  var b = Sa[a];
  b.target.removeEventListener(b.rb, b.jc, b.zb);
  Sa.splice(a, 1);
}
function Ua(a) {
  if (!a.target) return -4;
  if (a.Ab) a.jc = function (c) {
    ++Ma;
    Ra = a;
    Qa();
    a.Bb(c);
    Qa();
    --Ma;
  }, a.target.addEventListener(a.rb, a.jc, a.zb), Sa.push(a);else for (var b = 0; b < Sa.length; ++b) Sa[b].target == a.target && Sa[b].rb == a.rb && Ta(b--);
  return 0;
}
function Va(a) {
  return a ? a == window ? "#window" : a == screen ? "#screen" : (a === null || a === void 0 ? void 0 : a.nodeName) || "" : "";
}
function Wa() {
  return document.fullscreenEnabled || document.webkitFullscreenEnabled;
}
var Ra,
  Xa,
  Ya,
  Za,
  $a,
  ab,
  bb,
  cb = {},
  db = new TextDecoder("utf8"),
  O = (a, b) => {
    if (!a) return "";
    b = a + b;
    for (var c = a; !(c >= b) && A[c];) ++c;
    return db.decode(A.subarray(a, c));
  },
  eb = [0, document, window],
  P = a => {
    a = 2 < a ? O(a) : a;
    return eb[a] || document.querySelector(a);
  },
  gb = a => {
    var b = Ga(a) + 1,
      c = fb(b);
    K(a, A, c, b);
    return c;
  },
  hb = a => La(() => {
    var b = fb(8),
      c = b + 4,
      d = gb(a.id);
    if (d = P(d)) C[b >> 2] = d.width, C[c >> 2] = d.height;
    return [C[b >> 2], C[c >> 2]];
  }),
  ib = (a, b, c) => {
    a = P(a);
    if (!a) return -4;
    a.width = b;
    a.height = c;
    return 0;
  },
  jb = (a, b, c) => {
    a.ad ? La(() => {
      var d = gb(a.id);
      ib(d, b, c);
    }) : (a.width = b, a.height = c);
  },
  kb = [],
  lb,
  Q = a => {
    var b = kb[a];
    b || (a >= kb.length && (kb.length = a + 1), kb[a] = b = lb.get(a));
    return b;
  },
  mb = a => {
    function b() {
      document.fullscreenElement || document.webkitFullscreenElement || (document.removeEventListener("fullscreenchange", b), document.removeEventListener("webkitfullscreenchange", b), jb(a, d, e), a.style.width = f, a.style.height = g, a.style.backgroundColor = m, n || (document.body.style.backgroundColor = "white"), document.body.style.backgroundColor = n, a.style.paddingLeft = q, a.style.paddingRight = t, a.style.paddingTop = u, a.style.paddingBottom = z, a.style.marginLeft = h, a.style.marginRight = w, a.style.marginTop = x, a.style.marginBottom = B, document.body.style.margin = J, document.documentElement.style.overflow = L, document.body.scroll = od, a.style.tb = pd, a.Gb && a.Gb.Fb.viewport(0, 0, d, e), cb.Hb && Q(cb.Hb)(37, 0, cb.fc));
    }
    var c = hb(a),
      d = c[0],
      e = c[1],
      f = a.style.width,
      g = a.style.height,
      m = a.style.backgroundColor,
      n = document.body.style.backgroundColor,
      q = a.style.paddingLeft,
      t = a.style.paddingRight,
      u = a.style.paddingTop,
      z = a.style.paddingBottom,
      h = a.style.marginLeft,
      w = a.style.marginRight,
      x = a.style.marginTop,
      B = a.style.marginBottom,
      J = document.body.style.margin,
      L = document.documentElement.style.overflow,
      od = document.body.scroll,
      pd = a.style.tb;
    document.addEventListener("fullscreenchange", b);
    document.addEventListener("webkitfullscreenchange", b);
  },
  nb = (a, b, c) => {
    a.style.paddingLeft = a.style.paddingRight = c + "px";
    a.style.paddingTop = a.style.paddingBottom = b + "px";
  },
  ob = a => 0 > eb.indexOf(a) ? a.getBoundingClientRect() : {
    left: 0,
    top: 0
  },
  Oa = (a, b) => {
    if (0 != b.$b || 0 != b.Vb) {
      mb(a);
      var c = b.Lc ? innerWidth : screen.width,
        d = b.Lc ? innerHeight : screen.height,
        e = ob(a),
        f = e.width;
      e = e.height;
      var g = hb(a),
        m = g[0];
      g = g[1];
      3 == b.$b ? (nb(a, (d - e) / 2, (c - f) / 2), c = f, d = e) : 2 == b.$b && (c * g < m * d ? (f = g * c / m, nb(a, (d - f) / 2, 0), d = f) : (f = m * d / g, nb(a, 0, (c - f) / 2), c = f));
      a.style.backgroundColor || (a.style.backgroundColor = "black");
      document.body.style.backgroundColor || (document.body.style.backgroundColor = "black");
      a.style.width = c + "px";
      a.style.height = d + "px";
      1 == b.Ec && (a.style.tb = "optimizeSpeed", a.style.tb = "-moz-crisp-edges", a.style.tb = "-o-crisp-edges", a.style.tb = "-webkit-optimize-contrast", a.style.tb = "optimize-contrast", a.style.tb = "crisp-edges", a.style.tb = "pixelated");
      f = 2 == b.Vb ? devicePixelRatio : 1;
      0 != b.Vb && (c = c * f | 0, d = d * f | 0, jb(a, c, d), a.Gb && a.Gb.Fb.viewport(0, 0, c, d));
    }
    if (a.requestFullscreen) a.requestFullscreen();else if (a.webkitRequestFullscreen) a.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT);else return Wa() ? -3 : -1;
    cb = b;
    b.Hb && Q(b.Hb)(37, 0, b.fc);
    return 0;
  },
  pb = a => {
    if (a.requestPointerLock) a.requestPointerLock();else return document.body.requestPointerLock ? -3 : -1;
    return 0;
  },
  qb = (a, b) => {
    if (!Wa()) return -1;
    a = P(a);
    return a ? a.requestFullscreen || a.webkitRequestFullscreen ? (navigator.userActivation ? navigator.userActivation.isActive : Ma && Ra.Ub) ? Oa(a, b) : b.Cc ? (Na([a, b]), 1) : -2 : -3 : -4;
  },
  rb = (a, b, c, d, e) => {
    Ya || (Ya = M(280));
    return Ua({
      target: a,
      rb: e,
      Ab: d,
      Bb: function () {
        let f = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : event;
        var g = Ya,
          m = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement,
          n = !!m;
        C[g >> 2] = n;
        C[g + 4 >> 2] = Wa();
        var q = n ? m : Xa,
          t = (q === null || q === void 0 ? void 0 : q.id) || "";
        K(Va(q), A, g + 8, 128);
        K(t, A, g + 136, 128);
        C[g + 264 >> 2] = q ? q.clientWidth : 0;
        C[g + 268 >> 2] = q ? q.clientHeight : 0;
        C[g + 272 >> 2] = screen.width;
        C[g + 276 >> 2] = screen.height;
        n && (Xa = m);
        Q(d)(19, g, b) && f.preventDefault();
      },
      zb: c
    });
  },
  sb = (a, b, c, d, e, f) => {
    Za || (Za = M(176));
    a = {
      target: P(a),
      rb: f,
      Ab: d,
      Bb: g => {
        var m = Za;
        F[m >> 3] = g.timeStamp;
        var n = m >> 2;
        C[n + 2] = g.location;
        C[n + 3] = g.ctrlKey;
        C[n + 4] = g.shiftKey;
        C[n + 5] = g.altKey;
        C[n + 6] = g.metaKey;
        C[n + 7] = g.repeat;
        C[n + 8] = g.charCode;
        C[n + 9] = g.keyCode;
        C[n + 10] = g.which;
        K(g.key || "", A, m + 44, 32);
        K(g.code || "", A, m + 76, 32);
        K(g.char || "", A, m + 108, 32);
        K(g.locale || "", A, m + 140, 32);
        Q(d)(e, m, b) && g.preventDefault();
      },
      zb: c
    };
    return Ua(a);
  },
  tb = a => {
    a instanceof Aa || "unwind" == a || ca(1, a);
  },
  ub = (a, b) => {
    for (var c = 0, d = a.length - 1; 0 <= d; d--) {
      var e = a[d];
      "." === e ? a.splice(d, 1) : ".." === e ? (a.splice(d, 1), c++) : c && (a.splice(d, 1), c--);
    }
    if (b) for (; c; c--) a.unshift("..");
    return a;
  },
  vb = a => {
    var b = "/" === a.charAt(0),
      c = "/" === a.substr(-1);
    (a = ub(a.split("/").filter(d => !!d), !b).join("/")) || b || (a = ".");
    a && c && (a += "/");
    return (b ? "/" : "") + a;
  },
  wb = a => {
    var b = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/.exec(a).slice(1);
    a = b[0];
    b = b[1];
    if (!a && !b) return ".";
    b && (b = b.substr(0, b.length - 1));
    return a + b;
  },
  xb = a => {
    if ("/" === a) return "/";
    a = vb(a);
    a = a.replace(/\/$/, "");
    var b = a.lastIndexOf("/");
    return -1 === b ? a : a.substr(b + 1);
  },
  yb = () => {
    if ("object" == typeof crypto && "function" == typeof crypto.getRandomValues) return a => crypto.getRandomValues(a);
    r("initRandomDevice");
  },
  zb = a => (zb = yb())(a);
function Ab() {
  for (var a = "", b = !1, c = arguments.length - 1; -1 <= c && !b; c--) {
    b = 0 <= c ? arguments[c] : "/";
    if ("string" != typeof b) throw new TypeError("Arguments to path.resolve must be strings");
    if (!b) return "";
    a = b + "/" + a;
    b = "/" === b.charAt(0);
  }
  a = ub(a.split("/").filter(d => !!d), !b).join("/");
  return (b ? "/" : "") + a || ".";
}
var Bb = a => {
    for (var b = 0; a[b] && !(NaN <= b);) ++b;
    return db.decode(a.buffer ? a.subarray(0, b) : new Uint8Array(a.slice(0, b)));
  },
  Cb = [];
function Db(a, b) {
  var c = Array(Ga(a) + 1);
  a = K(a, c, 0, c.length);
  b && (c.length = a);
  return c;
}
var Eb = [];
function Fb(a, b) {
  Eb[a] = {
    input: [],
    kb: [],
    yb: b
  };
  Gb(a, Hb);
}
var Hb = {
    open(a) {
      var b = Eb[a.node.Qb];
      if (!b) throw new R(43);
      a.lb = b;
      a.seekable = !1;
    },
    close(a) {
      a.lb.yb.Kb(a.lb);
    },
    Kb(a) {
      a.lb.yb.Kb(a.lb);
    },
    read(a, b, c, d) {
      if (!a.lb || !a.lb.yb.mc) throw new R(60);
      for (var e = 0, f = 0; f < d; f++) {
        try {
          var g = a.lb.yb.mc(a.lb);
        } catch (m) {
          throw new R(29);
        }
        if (void 0 === g && 0 === e) throw new R(6);
        if (null === g || void 0 === g) break;
        e++;
        b[c + f] = g;
      }
      e && (a.node.timestamp = Date.now());
      return e;
    },
    write(a, b, c, d) {
      if (!a.lb || !a.lb.yb.Yb) throw new R(60);
      try {
        for (var e = 0; e < d; e++) a.lb.yb.Yb(a.lb, b[c + e]);
      } catch (f) {
        throw new R(29);
      }
      d && (a.node.timestamp = Date.now());
      return e;
    }
  },
  Ib = {
    mc() {
      a: {
        if (!Cb.length) {
          var a = null;
          "undefined" != typeof window && "function" == typeof window.prompt ? (a = window.prompt("Input: "), null !== a && (a += "\n")) : "function" == typeof readline && (a = readline(), null !== a && (a += "\n"));
          if (!a) {
            a = null;
            break a;
          }
          Cb = Db(a, !0);
        }
        a = Cb.shift();
      }
      return a;
    },
    Yb(a, b) {
      null === b || 10 === b ? (da(Bb(a.kb)), a.kb = []) : 0 != b && a.kb.push(b);
    },
    Kb(a) {
      a.kb && 0 < a.kb.length && (da(Bb(a.kb)), a.kb = []);
    },
    ld() {
      return {
        Yc: 25856,
        $c: 5,
        Xc: 191,
        Zc: 35387,
        Wc: [3, 28, 127, 21, 4, 0, 1, 0, 17, 19, 26, 0, 18, 15, 23, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
      };
    },
    md() {
      return 0;
    },
    nd() {
      return [24, 80];
    }
  },
  Jb = {
    Yb(a, b) {
      null === b || 10 === b ? (p(Bb(a.kb)), a.kb = []) : 0 != b && a.kb.push(b);
    },
    Kb(a) {
      a.kb && 0 < a.kb.length && (p(Bb(a.kb)), a.kb = []);
    }
  };
function Kb(a, b) {
  var c = a.hb ? a.hb.length : 0;
  c >= b || (b = Math.max(b, c * (1048576 > c ? 2 : 1.125) >>> 0), 0 != c && (b = Math.max(b, 256)), c = a.hb, a.hb = new Uint8Array(b), 0 < a.jb && a.hb.set(c.subarray(0, a.jb), 0));
}
var S = {
    mb: null,
    qb() {
      return S.createNode(null, "/", 16895, 0);
    },
    createNode(a, b, c, d) {
      if (24576 === (c & 61440) || 4096 === (c & 61440)) throw new R(63);
      S.mb || (S.mb = {
        dir: {
          node: {
            sb: S.gb.sb,
            nb: S.gb.nb,
            Cb: S.gb.Cb,
            Ob: S.gb.Ob,
            vc: S.gb.vc,
            zc: S.gb.zc,
            wc: S.gb.wc,
            uc: S.gb.uc,
            Rb: S.gb.Rb
          },
          stream: {
            ub: S.ib.ub
          }
        },
        file: {
          node: {
            sb: S.gb.sb,
            nb: S.gb.nb
          },
          stream: {
            ub: S.ib.ub,
            read: S.ib.read,
            write: S.ib.write,
            dc: S.ib.dc,
            pc: S.ib.pc,
            rc: S.ib.rc
          }
        },
        link: {
          node: {
            sb: S.gb.sb,
            nb: S.gb.nb,
            Db: S.gb.Db
          },
          stream: {}
        },
        hc: {
          node: {
            sb: S.gb.sb,
            nb: S.gb.nb
          },
          stream: Lb
        }
      });
      c = Mb(a, b, c, d);
      16384 === (c.mode & 61440) ? (c.gb = S.mb.dir.node, c.ib = S.mb.dir.stream, c.hb = {}) : 32768 === (c.mode & 61440) ? (c.gb = S.mb.file.node, c.ib = S.mb.file.stream, c.jb = 0, c.hb = null) : 40960 === (c.mode & 61440) ? (c.gb = S.mb.link.node, c.ib = S.mb.link.stream) : 8192 === (c.mode & 61440) && (c.gb = S.mb.hc.node, c.ib = S.mb.hc.stream);
      c.timestamp = Date.now();
      a && (a.hb[b] = c, a.timestamp = c.timestamp);
      return c;
    },
    hd(a) {
      return a.hb ? a.hb.subarray ? a.hb.subarray(0, a.jb) : new Uint8Array(a.hb) : new Uint8Array(0);
    },
    gb: {
      sb(a) {
        var b = {};
        b.dd = 8192 === (a.mode & 61440) ? a.id : 1;
        b.kd = a.id;
        b.mode = a.mode;
        b.qd = 1;
        b.uid = 0;
        b.jd = 0;
        b.Qb = a.Qb;
        16384 === (a.mode & 61440) ? b.size = 4096 : 32768 === (a.mode & 61440) ? b.size = a.jb : 40960 === (a.mode & 61440) ? b.size = a.link.length : b.size = 0;
        b.Uc = new Date(a.timestamp);
        b.pd = new Date(a.timestamp);
        b.cd = new Date(a.timestamp);
        b.Ac = 4096;
        b.Vc = Math.ceil(b.size / b.Ac);
        return b;
      },
      nb(a, b) {
        void 0 !== b.mode && (a.mode = b.mode);
        void 0 !== b.timestamp && (a.timestamp = b.timestamp);
        if (void 0 !== b.size && (b = b.size, a.jb != b)) if (0 == b) a.hb = null, a.jb = 0;else {
          var c = a.hb;
          a.hb = new Uint8Array(b);
          c && a.hb.set(c.subarray(0, Math.min(b, a.jb)));
          a.jb = b;
        }
      },
      Cb() {
        throw Nb[44];
      },
      Ob(a, b, c, d) {
        return S.createNode(a, b, c, d);
      },
      vc(a, b, c) {
        if (16384 === (a.mode & 61440)) {
          try {
            var d = Ob(b, c);
          } catch (f) {}
          if (d) for (var e in d.hb) throw new R(55);
        }
        delete a.parent.hb[a.name];
        a.parent.timestamp = Date.now();
        a.name = c;
        b.hb[c] = a;
        b.timestamp = a.parent.timestamp;
        a.parent = b;
      },
      zc(a, b) {
        delete a.hb[b];
        a.timestamp = Date.now();
      },
      wc(a, b) {
        var c = Ob(a, b),
          d;
        for (d in c.hb) throw new R(55);
        delete a.hb[b];
        a.timestamp = Date.now();
      },
      uc(a) {
        var b = [".", ".."],
          c;
        for (c of Object.keys(a.hb)) b.push(c);
        return b;
      },
      Rb(a, b, c) {
        a = S.createNode(a, b, 41471, 0);
        a.link = c;
        return a;
      },
      Db(a) {
        if (40960 !== (a.mode & 61440)) throw new R(28);
        return a.link;
      }
    },
    ib: {
      read(a, b, c, d, e) {
        var f = a.node.hb;
        if (e >= a.node.jb) return 0;
        a = Math.min(a.node.jb - e, d);
        if (8 < a && f.subarray) b.set(f.subarray(e, e + a), c);else for (d = 0; d < a; d++) b[c + d] = f[e + d];
        return a;
      },
      write(a, b, c, d, e, f) {
        b.buffer === y.buffer && (f = !1);
        if (!d) return 0;
        a = a.node;
        a.timestamp = Date.now();
        if (b.subarray && (!a.hb || a.hb.subarray)) {
          if (f) return a.hb = b.subarray(c, c + d), a.jb = d;
          if (0 === a.jb && 0 === e) return a.hb = b.slice(c, c + d), a.jb = d;
          if (e + d <= a.jb) return a.hb.set(b.subarray(c, c + d), e), d;
        }
        Kb(a, e + d);
        if (a.hb.subarray && b.subarray) a.hb.set(b.subarray(c, c + d), e);else for (f = 0; f < d; f++) a.hb[e + f] = b[c + f];
        a.jb = Math.max(a.jb, e + d);
        return d;
      },
      ub(a, b, c) {
        1 === c ? b += a.position : 2 === c && 32768 === (a.node.mode & 61440) && (b += a.node.jb);
        if (0 > b) throw new R(28);
        return b;
      },
      dc(a, b, c) {
        Kb(a.node, b + c);
        a.node.jb = Math.max(a.node.jb, b + c);
      },
      pc(a, b, c, d, e) {
        if (32768 !== (a.node.mode & 61440)) throw new R(43);
        a = a.node.hb;
        if (e & 2 || a.buffer !== y.buffer) {
          if (0 < c || c + b < a.length) a.subarray ? a = a.subarray(c, c + b) : a = Array.prototype.slice.call(a, c, c + b);
          c = !0;
          r();
          b = void 0;
          if (!b) throw new R(48);
          y.set(a, b);
        } else c = !1, b = a.byteOffset;
        return {
          vd: b,
          Sc: c
        };
      },
      rc(a, b, c, d) {
        S.ib.write(a, b, 0, d, c, !1);
        return 0;
      }
    }
  },
  Pb = k.preloadPlugins || [],
  Qb = (a, b) => {
    var c = 0;
    a && (c |= 365);
    b && (c |= 146);
    return c;
  },
  Rb = null,
  Sb = {},
  Tb = [],
  Ub = 1,
  Vb = null,
  Wb = !0,
  R = null,
  Nb = {};
function T(a) {
  let b = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {};
  a = Ab(a);
  if (!a) return {
    path: "",
    node: null
  };
  b = Object.assign({
    lc: !0,
    Zb: 0
  }, b);
  if (8 < b.Zb) throw new R(32);
  a = a.split("/").filter(g => !!g);
  for (var c = Rb, d = "/", e = 0; e < a.length; e++) {
    var f = e === a.length - 1;
    if (f && b.parent) break;
    c = Ob(c, a[e]);
    d = vb(d + "/" + a[e]);
    c.Pb && (!f || f && b.lc) && (c = c.Pb.root);
    if (!f || b.kc) for (f = 0; 40960 === (c.mode & 61440);) if (c = Xb(d), d = Ab(wb(d), c), c = T(d, {
      Zb: b.Zb + 1
    }).node, 40 < f++) throw new R(32);
  }
  return {
    path: d,
    node: c
  };
}
function Yb(a) {
  for (var b;;) {
    if (a === a.parent) return a = a.qb.qc, b ? "/" !== a[a.length - 1] ? `${a}/${b}` : a + b : a;
    b = b ? `${a.name}/${b}` : a.name;
    a = a.parent;
  }
}
function Zb(a, b) {
  for (var c = 0, d = 0; d < b.length; d++) c = (c << 5) - c + b.charCodeAt(d) | 0;
  return (a + c >>> 0) % Vb.length;
}
function Ob(a, b) {
  var c;
  if (c = (c = $b(a, "x")) ? c : a.gb.Cb ? 0 : 2) throw new R(c, a);
  for (c = Vb[Zb(a.id, b)]; c; c = c.Jc) {
    var d = c.name;
    if (c.parent.id === a.id && d === b) return c;
  }
  return a.gb.Cb(a, b);
}
function Mb(a, b, c, d) {
  a = new ac(a, b, c, d);
  b = Zb(a.parent.id, a.name);
  a.Jc = Vb[b];
  return Vb[b] = a;
}
function bc(a) {
  var b = ["r", "w", "rw"][a & 3];
  a & 512 && (b += "w");
  return b;
}
function $b(a, b) {
  if (Wb) return 0;
  if (!b.includes("r") || a.mode & 292) {
    if (b.includes("w") && !(a.mode & 146) || b.includes("x") && !(a.mode & 73)) return 2;
  } else return 2;
  return 0;
}
function cc(a, b) {
  try {
    return Ob(a, b), 20;
  } catch (c) {}
  return $b(a, "wx");
}
function dc() {
  for (var a = 0; 4096 >= a; a++) if (!Tb[a]) return a;
  throw new R(33);
}
function ec(a) {
  a = Tb[a];
  if (!a) throw new R(8);
  return a;
}
function fc(a) {
  var b = -1;
  gc || (gc = function () {
    this.pb = {};
  }, gc.prototype = {}, Object.defineProperties(gc.prototype, {
    object: {
      get() {
        return this.node;
      },
      set(c) {
        this.node = c;
      }
    },
    flags: {
      get() {
        return this.pb.flags;
      },
      set(c) {
        this.pb.flags = c;
      }
    },
    position: {
      get() {
        return this.pb.position;
      },
      set(c) {
        this.pb.position = c;
      }
    }
  }));
  a = Object.assign(new gc(), a);
  -1 == b && (b = dc());
  a.xb = b;
  return Tb[b] = a;
}
var Lb = {
  open(a) {
    var _a$ib$open, _a$ib;
    a.ib = Sb[a.node.Qb].ib;
    (_a$ib$open = (_a$ib = a.ib).open) === null || _a$ib$open === void 0 || _a$ib$open.call(_a$ib, a);
  },
  ub() {
    throw new R(70);
  }
};
function Gb(a, b) {
  Sb[a] = {
    ib: b
  };
}
function hc(a, b) {
  var c = "/" === b,
    d = !b;
  if (c && Rb) throw new R(10);
  if (!c && !d) {
    var e = T(b, {
      lc: !1
    });
    b = e.path;
    e = e.node;
    if (e.Pb) throw new R(10);
    if (16384 !== (e.mode & 61440)) throw new R(54);
  }
  b = {
    type: a,
    td: {},
    qc: b,
    Ic: []
  };
  a = a.qb(b);
  a.qb = b;
  b.root = a;
  c ? Rb = a : e && (e.Pb = b, e.qb && e.qb.Ic.push(b));
}
function ic(a, b, c) {
  var d = T(a, {
    parent: !0
  }).node;
  a = xb(a);
  if (!a || "." === a || ".." === a) throw new R(28);
  var e = cc(d, a);
  if (e) throw new R(e);
  if (!d.gb.Ob) throw new R(63);
  return d.gb.Ob(d, a, b, c);
}
function U(a) {
  return ic(a, 16895, 0);
}
function jc(a, b, c) {
  "undefined" == typeof c && (c = b, b = 438);
  ic(a, b | 8192, c);
}
function kc(a, b) {
  if (!Ab(a)) throw new R(44);
  var c = T(b, {
    parent: !0
  }).node;
  if (!c) throw new R(44);
  b = xb(b);
  var d = cc(c, b);
  if (d) throw new R(d);
  if (!c.gb.Rb) throw new R(63);
  c.gb.Rb(c, b, a);
}
function Xb(a) {
  a = T(a).node;
  if (!a) throw new R(44);
  if (!a.gb.Db) throw new R(28);
  return Ab(Yb(a.parent), a.gb.Db(a));
}
function lc(a, b) {
  if ("" === a) throw new R(44);
  if ("string" == typeof b) {
    var c = {
      r: 0,
      "r+": 2,
      w: 577,
      "w+": 578,
      a: 1089,
      "a+": 1090
    }[b];
    if ("undefined" == typeof c) throw Error(`Unknown file open mode: ${b}`);
    b = c;
  }
  var d = b & 64 ? ("undefined" == typeof d ? 438 : d) & 4095 | 32768 : 0;
  if ("object" == typeof a) var e = a;else {
    a = vb(a);
    try {
      e = T(a, {
        kc: !(b & 131072)
      }).node;
    } catch (f) {}
  }
  c = !1;
  if (b & 64) if (e) {
    if (b & 128) throw new R(20);
  } else e = ic(a, d, 0), c = !0;
  if (!e) throw new R(44);
  8192 === (e.mode & 61440) && (b &= -513);
  if (b & 65536 && 16384 !== (e.mode & 61440)) throw new R(54);
  if (!c && (d = e ? 40960 === (e.mode & 61440) ? 32 : 16384 === (e.mode & 61440) && ("r" !== bc(b) || b & 512) ? 31 : $b(e, bc(b)) : 44)) throw new R(d);
  if (b & 512 && !c) {
    d = e;
    d = "string" == typeof d ? T(d, {
      kc: !0
    }).node : d;
    if (!d.gb.nb) throw new R(63);
    if (16384 === (d.mode & 61440)) throw new R(31);
    if (32768 !== (d.mode & 61440)) throw new R(28);
    if (c = $b(d, "w")) throw new R(c);
    d.gb.nb(d, {
      size: 0,
      timestamp: Date.now()
    });
  }
  b &= -131713;
  e = fc({
    node: e,
    path: Yb(e),
    flags: b,
    seekable: !0,
    position: 0,
    ib: e.ib,
    Qc: [],
    error: !1
  });
  e.ib.open && e.ib.open(e);
  !k.logReadFiles || b & 1 || (mc || (mc = {}), a in mc || (mc[a] = 1));
}
function nc(a, b, c) {
  if (null === a.xb) throw new R(8);
  if (!a.seekable || !a.ib.ub) throw new R(70);
  if (0 != c && 1 != c && 2 != c) throw new R(28);
  a.position = a.ib.ub(a, b, c);
  a.Qc = [];
}
function oc() {
  R || (R = function (a, b) {
    this.name = "ErrnoError";
    this.node = b;
    this.Kc = function (c) {
      this.Jb = c;
    };
    this.Kc(a);
    this.message = "FS error";
  }, R.prototype = Error(), R.prototype.constructor = R, [44].forEach(a => {
    Nb[a] = new R(a);
    Nb[a].stack = "<generic error, no stack>";
  }));
}
var pc;
function qc(a, b, c) {
  a = vb("/dev/" + a);
  var d = Qb(!!b, !!c);
  rc || (rc = 64);
  var e = rc++ << 8 | 0;
  Gb(e, {
    open(f) {
      f.seekable = !1;
    },
    close() {
      var _c$buffer;
      (c === null || c === void 0 || (_c$buffer = c.buffer) === null || _c$buffer === void 0 ? void 0 : _c$buffer.length) && c(10);
    },
    read(f, g, m, n) {
      for (var q = 0, t = 0; t < n; t++) {
        try {
          var u = b();
        } catch (z) {
          throw new R(29);
        }
        if (void 0 === u && 0 === q) throw new R(6);
        if (null === u || void 0 === u) break;
        q++;
        g[m + t] = u;
      }
      q && (f.node.timestamp = Date.now());
      return q;
    },
    write(f, g, m, n) {
      for (var q = 0; q < n; q++) try {
        c(g[m + q]);
      } catch (t) {
        throw new R(29);
      }
      n && (f.node.timestamp = Date.now());
      return q;
    }
  });
  jc(a, d, e);
}
var rc,
  sc = {},
  gc,
  mc,
  tc = a => {
    var _k$onExit, _k2;
    ha = a;
    Ca || ((_k$onExit = (_k2 = k).onExit) !== null && _k$onExit !== void 0 && _k$onExit.call(_k2, a), v = !0);
    ca(a, new Aa(a));
  },
  uc = () => {
    if (!Ca) try {
      var a = ha;
      ha = a;
      tc(a);
    } catch (b) {
      tb(b);
    }
  },
  vc = a => {
    setTimeout(() => {
      if (!v) try {
        a(), uc();
      } catch (b) {
        tb(b);
      }
    }, 1E4);
  },
  wc = !1,
  V = null,
  xc = 0,
  yc = null,
  zc = 0,
  Ac = 0,
  Bc = 0,
  Cc = 0,
  Dc = [],
  Ec = {},
  Fc,
  Gc,
  Hc,
  Ic = !1,
  Jc = !1,
  Kc = [];
function Lc() {
  function a() {
    Jc = document.pointerLockElement === k.canvas || document.mozPointerLockElement === k.canvas || document.webkitPointerLockElement === k.canvas || document.msPointerLockElement === k.canvas;
  }
  if (!Mc) {
    Mc = !0;
    Pb.push({
      canHandle: function (c) {
        return !k.sd && /\.(jpg|jpeg|png|bmp)$/i.test(c);
      },
      handle: function (c, d, e, f) {
        var g = new Blob([c], {
          type: Nc(d)
        });
        g.size !== c.length && (g = new Blob([new Uint8Array(c).buffer], {
          type: Nc(d)
        }));
        var m = URL.createObjectURL(g),
          n = new Image();
        n.onload = () => {
          n.complete || r(`Image ${d} could not be decoded`);
          var q = document.createElement("canvas");
          q.width = n.width;
          q.height = n.height;
          q.getContext("2d").drawImage(n, 0, 0);
          URL.revokeObjectURL(m);
          e === null || e === void 0 || e(c);
        };
        n.onerror = () => {
          p(`Image ${m} could not be decoded`);
          f === null || f === void 0 || f();
        };
        n.src = m;
      }
    });
    Pb.push({
      canHandle: function (c) {
        return !k.rd && c.substr(-4) in {
          ".ogg": 1,
          ".wav": 1,
          ".mp3": 1
        };
      },
      handle: function (c, d, e) {
        function f() {
          g || (g = !0, e === null || e === void 0 ? void 0 : e(c));
        }
        var g = !1,
          m = URL.createObjectURL(new Blob([c], {
            type: Nc(d)
          })),
          n = new Audio();
        n.addEventListener("canplaythrough", () => f(n), !1);
        n.onerror = function () {
          if (!g) {
            p(`warning: browser could not fully decode audio ${d}, trying slower base64 approach`);
            for (var q = "", t = 0, u = 0, z = 0; z < c.length; z++) for (t = t << 8 | c[z], u += 8; 6 <= u;) {
              var h = t >> u - 6 & 63;
              u -= 6;
              q += "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[h];
            }
            2 == u ? (q += "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(t & 3) << 4], q += "==") : 4 == u && (q += "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(t & 15) << 2], q += "=");
            n.src = "data:audio/x-" + d.substr(-3) + ";base64," + q;
            f(n);
          }
        };
        n.src = m;
        vc(() => {
          f(n);
        });
      }
    });
    var b = k.canvas;
    b && (b.requestPointerLock = b.requestPointerLock || b.mozRequestPointerLock || b.webkitRequestPointerLock || b.msRequestPointerLock || (() => {}), b.exitPointerLock = document.exitPointerLock || document.mozExitPointerLock || document.webkitExitPointerLock || document.msExitPointerLock || (() => {}), b.exitPointerLock = b.exitPointerLock.bind(document), document.addEventListener("pointerlockchange", a, !1), document.addEventListener("mozpointerlockchange", a, !1), document.addEventListener("webkitpointerlockchange", a, !1), document.addEventListener("mspointerlockchange", a, !1), k.elementPointerLock && b.addEventListener("click", c => {
      !Jc && k.canvas.requestPointerLock && (k.canvas.requestPointerLock(), c.preventDefault());
    }, !1));
  }
}
var Oc = !1,
  Pc = void 0,
  Qc = void 0;
function Rc() {
  if (!Ic) return !1;
  (document.exitFullscreen || document.cancelFullScreen || document.mozCancelFullScreen || document.msExitFullscreen || document.webkitCancelFullScreen || (() => {})).apply(document, []);
  return !0;
}
var Sc = 0;
function Tc(a) {
  if ("function" == typeof requestAnimationFrame) requestAnimationFrame(a);else {
    var b = Date.now();
    if (0 === Sc) Sc = b + 1E3 / 60;else for (; b + 2 >= Sc;) Sc += 1E3 / 60;
    setTimeout(a, Math.max(Sc - b, 0));
  }
}
function Nc(a) {
  return {
    jpg: "image/jpeg",
    jpeg: "image/jpeg",
    png: "image/png",
    bmp: "image/bmp",
    ogg: "audio/ogg",
    wav: "audio/wav",
    mp3: "audio/mpeg"
  }[a.substr(a.lastIndexOf(".") + 1)];
}
var Uc = [];
function Vc() {
  var a = k.canvas;
  Uc.forEach(b => b(a.width, a.height));
}
function Wc(a, b, c) {
  b && c ? (a.Rc = b, a.Gc = c) : (b = a.Rc, c = a.Gc);
  var d = b,
    e = c;
  k.forcedAspectRatio && 0 < k.forcedAspectRatio && (d / e < k.forcedAspectRatio ? d = Math.round(e * k.forcedAspectRatio) : e = Math.round(d / k.forcedAspectRatio));
  if ((document.fullscreenElement || document.mozFullScreenElement || document.msFullscreenElement || document.webkitFullscreenElement || document.webkitCurrentFullScreenElement) === a.parentNode && "undefined" != typeof screen) {
    var f = Math.min(screen.width / d, screen.height / e);
    d = Math.round(d * f);
    e = Math.round(e * f);
  }
  Qc ? (a.width != d && (a.width = d), a.height != e && (a.height = e), "undefined" != typeof a.style && (a.style.removeProperty("width"), a.style.removeProperty("height"))) : (a.width != b && (a.width = b), a.height != c && (a.height = c), "undefined" != typeof a.style && (d != b || e != c ? (a.style.setProperty("width", d + "px", "important"), a.style.setProperty("height", e + "px", "important")) : (a.style.removeProperty("width"), a.style.removeProperty("height"))));
}
var Mc,
  Xc,
  Zc = (a, b) => {
    Ac = a;
    Bc = b;
    if (yc) if (wc || (wc = !0), 0 == a) V = function () {
      var d = Math.max(0, Gc + b - Yc()) | 0;
      setTimeout(Hc, d);
    };else if (1 == a) V = function () {
      Tc(Hc);
    };else if (2 == a) {
      if ("undefined" == typeof Xc) if ("undefined" == typeof setImmediate) {
        var c = [];
        addEventListener("message", d => {
          if ("setimmediate" === d.data || "setimmediate" === d.data.target) d.stopPropagation(), c.shift()();
        }, !0);
        Xc = function (d) {
          c.push(d);
          postMessage("setimmediate", "*");
        };
      } else Xc = setImmediate;
      V = function () {
        Xc(Hc);
      };
    }
  },
  Yc;
Yc = () => performance.now();
var $c = (a, b, c, d, e) => {
    !yc || r("emscripten_set_main_loop: there can only be one main loop function at once: call emscripten_cancel_main_loop to cancel the previous one before setting a new one with different parameters.");
    yc = a;
    zc = d;
    var f = xc;
    wc = !1;
    Hc = function () {
      if (!v) if (0 < Dc.length) {
        var g = Dc.shift();
        g.gd(g.Tc);
        if (Fc) {
          var m = Fc,
            n = 0 == m % 1 ? m - 1 : Math.floor(m);
          Fc = g.bd ? n : (8 * m + (n + .5)) / 9;
        }
        k.setStatus && (g = k.statusMessage || "Please wait...", m = Fc, n = Ec.fd, m ? m < n ? k.setStatus(g + " (" + (n - m) + "/" + n + ")") : k.setStatus(g) : k.setStatus(""));
        f < xc || setTimeout(Hc, 0);
      } else if (!(f < xc)) if (Cc = Cc + 1 | 0, 1 == Ac && 1 < Bc && 0 != Cc % Bc) V();else {
        var _SDL$audio, _SDL$audio$wd;
        0 == Ac && (Gc = Yc());
        if (!(v || k.preMainLoop && !1 === k.preMainLoop())) {
          var _k$postMainLoop, _k3;
          if (!v) try {
            a(), uc();
          } catch (q) {
            tb(q);
          }
          (_k$postMainLoop = (_k3 = k).postMainLoop) === null || _k$postMainLoop === void 0 || _k$postMainLoop.call(_k3);
        }
        f < xc || ("object" == typeof SDL && (_SDL$audio = SDL.audio) !== null && _SDL$audio !== void 0 && (_SDL$audio$wd = _SDL$audio.wd) !== null && _SDL$audio$wd !== void 0 && _SDL$audio$wd.call(_SDL$audio), V());
      }
    };
    e || (b && 0 < b ? Zc(0, 1E3 / b) : Zc(1, 1), V());
    if (c) throw "unwind";
  },
  ad = (a, b, c, d, e, f) => {
    $a || ($a = M(72));
    a = P(a);
    return Ua({
      target: a,
      Ub: "mousemove" != f && "mouseenter" != f && "mouseleave" != f,
      rb: f,
      Ab: d,
      Bb: function () {
        let g = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : event;
        var m = a,
          n = $a;
        F[n >> 3] = g.timeStamp;
        n >>= 2;
        C[n + 2] = g.screenX;
        C[n + 3] = g.screenY;
        C[n + 4] = g.clientX;
        C[n + 5] = g.clientY;
        C[n + 6] = g.ctrlKey;
        C[n + 7] = g.shiftKey;
        C[n + 8] = g.altKey;
        C[n + 9] = g.metaKey;
        ia[2 * n + 20] = g.button;
        ia[2 * n + 21] = g.buttons;
        C[n + 11] = g.movementX;
        C[n + 12] = g.movementY;
        m = ob(m);
        C[n + 13] = g.clientX - m.left;
        C[n + 14] = g.clientY - m.top;
        Q(d)(e, $a, b) && g.preventDefault();
      },
      zb: c
    });
  },
  bd = (a, b, c, d, e) => {
    ab || (ab = M(260));
    return Ua({
      target: a,
      rb: e,
      Ab: d,
      Bb: function () {
        let f = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : event;
        var g = ab,
          m = document.pointerLockElement || document.pb || document.Hc || document.Fc;
        C[g >> 2] = !!m;
        var n = (m === null || m === void 0 ? void 0 : m.id) || "";
        K(Va(m), A, g + 4, 128);
        K(n, A, g + 132, 128);
        Q(d)(20, g, b) && f.preventDefault();
      },
      zb: c
    });
  },
  cd = (a, b, c, d, e, f) => {
    bb || (bb = M(1696));
    a = P(a);
    return Ua({
      target: a,
      Ub: "touchstart" == f || "touchend" == f,
      rb: f,
      Ab: d,
      Bb: g => {
        for (var m, n = {}, q = g.touches, t = 0; t < q.length; ++t) m = q[t], m.nc = m.sc = 0, n[m.identifier] = m;
        for (t = 0; t < g.changedTouches.length; ++t) m = g.changedTouches[t], m.nc = 1, n[m.identifier] = m;
        for (t = 0; t < g.targetTouches.length; ++t) n[g.targetTouches[t].identifier].sc = 1;
        q = bb;
        F[q >> 3] = g.timeStamp;
        var u = q >> 2;
        C[u + 3] = g.ctrlKey;
        C[u + 4] = g.shiftKey;
        C[u + 5] = g.altKey;
        C[u + 6] = g.metaKey;
        u += 7;
        var z = ob(a),
          h = 0;
        for (t in n) if (m = n[t], C[u] = m.identifier, C[u + 1] = m.screenX, C[u + 2] = m.screenY, C[u + 3] = m.clientX, C[u + 4] = m.clientY, C[u + 5] = m.pageX, C[u + 6] = m.pageY, C[u + 7] = m.nc, C[u + 8] = m.sc, C[u + 9] = m.clientX - z.left, C[u + 10] = m.clientY - z.top, u += 13, 31 < ++h) break;
        C[q + 8 >> 2] = h;
        Q(d)(e, q, b) && g.preventDefault();
      },
      zb: c
    });
  },
  dd = 1,
  ed = [],
  W = [],
  fd = [],
  gd = [],
  X = [],
  hd = [],
  jd = [],
  kd = a => {
    for (var b = dd++, c = a.length; c < b; c++) a[c] = null;
    return b;
  },
  md = (a, b) => {
    a.pb || (a.pb = a.getContext, a.getContext = function (d, e) {
      e = a.pb(d, e);
      return "webgl" == d == e instanceof WebGLRenderingContext ? e : null;
    });
    var c = a.getContext("webgl2", b);
    return c ? ld(c, b) : 0;
  },
  ld = (a, b) => {
    var c = kd(jd);
    b = {
      handle: c,
      attributes: b,
      version: b.oc,
      Fb: a
    };
    a.canvas && (a.canvas.Gb = b);
    jd[c] = b;
    return c;
  },
  qd = a => {
    var _nd;
    nd = jd[a];
    k.Wb = Y = (_nd = nd) === null || _nd === void 0 ? void 0 : _nd.Fb;
    return !(a && !Y);
  },
  rd = {},
  nd,
  sd = ["default", "low-power", "high-performance"],
  td = {},
  vd = () => {
    if (!ud) {
      var a = {
          USER: "web_user",
          LOGNAME: "web_user",
          PATH: "/",
          PWD: "/",
          HOME: "/home/web_user",
          LANG: ("object" == typeof navigator && navigator.languages && navigator.languages[0] || "C").replace("-", "_") + ".UTF-8",
          _: ba || "./this.program"
        },
        b;
      for (b in td) void 0 === td[b] ? delete a[b] : a[b] = td[b];
      var c = [];
      for (b in a) c.push(`${b}=${a[b]}`);
      ud = c;
    }
    return ud;
  },
  ud,
  wd = [],
  xd = (a, b, c, d) => {
    for (var e = 0; e < a; e++) {
      var f = Y[c](),
        g = f && kd(d);
      f && (f.name = g, d[g] = f);
      C[b + 4 * e >> 2] = g;
    }
  },
  yd = (a, b, c, d, e, f, g, m) => {
    b = W[b];
    if (a = Y[a](b, c)) d = m && K(a.name, A, m, d), e && (C[e >> 2] = d), f && (C[f >> 2] = a.size), g && (C[g >> 2] = a.type);
  };
function zd() {
  var a = "EXT_color_buffer_float EXT_disjoint_timer_query_webgl2 EXT_texture_norm16 WEBGL_clip_cull_distance EXT_color_buffer_half_float EXT_float_blend EXT_texture_compression_bptc EXT_texture_compression_rgtc EXT_texture_filter_anisotropic KHR_parallel_shader_compile OES_texture_float_linear WEBGL_compressed_texture_s3tc WEBGL_compressed_texture_s3tc_srgb WEBGL_debug_renderer_info WEBGL_debug_shaders WEBGL_lose_context WEBGL_multi_draw".split(" ");
  return (Y.getSupportedExtensions() || []).filter(b => a.includes(b));
}
var Ad = (a, b) => {
    if (b) {
      var c = void 0;
      switch (a) {
        case 36346:
          c = 1;
          break;
        case 36344:
          return;
        case 34814:
        case 36345:
          c = 0;
          break;
        case 34466:
          var d = Y.getParameter(34467);
          c = d ? d.length : 0;
          break;
        case 33309:
          c = zd().length;
          break;
        case 33307:
        case 33308:
          c = 33307 == a ? 3 : 0;
      }
      if (void 0 === c) switch (d = Y.getParameter(a), typeof d) {
        case "number":
          c = d;
          break;
        case "boolean":
          c = d ? 1 : 0;
          break;
        case "string":
          return;
        case "object":
          if (null === d) switch (a) {
            case 34964:
            case 35725:
            case 34965:
            case 36006:
            case 36007:
            case 32873:
            case 34229:
            case 36662:
            case 36663:
            case 35053:
            case 35055:
            case 36010:
            case 35097:
            case 35869:
            case 32874:
            case 36389:
            case 35983:
            case 35368:
            case 34068:
              c = 0;
              break;
            default:
              return;
          } else {
            if (d instanceof Float32Array || d instanceof Uint32Array || d instanceof Int32Array || d instanceof Array) {
              for (a = 0; a < d.length; ++a) C[b + 4 * a >> 2] = d[a];
              return;
            }
            c = d.name | 0;
          }
      }
      C[b >> 2] = c;
    }
  },
  Bd = a => "]" == a.slice(-1) && a.lastIndexOf("["),
  Cd = a => {
    a -= 5120;
    return 0 == a ? y : 1 == a ? A : 2 == a ? ia : 4 == a ? C : 6 == a ? E : 5 == a || 28922 == a || 28520 == a || 30779 == a || 30782 == a ? D : ja;
  },
  Dd = a => {
    var b = Y.Bc,
      c = b.Eb[a];
    "number" == typeof c && (b.Eb[a] = c = Y.getUniformLocation(b, b.xc[a] + (0 < c ? `[${c}]` : "")));
    return c;
  },
  Ed = [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
  Fd = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
  Gd = (a, b, c, d) => {
    function e(h, w, x) {
      for (h = "number" == typeof h ? h.toString() : h || ""; h.length < w;) h = x[0] + h;
      return h;
    }
    function f(h, w) {
      return e(h, w, "0");
    }
    function g(h, w) {
      function x(J) {
        return 0 > J ? -1 : 0 < J ? 1 : 0;
      }
      var B;
      0 === (B = x(h.getFullYear() - w.getFullYear())) && 0 === (B = x(h.getMonth() - w.getMonth())) && (B = x(h.getDate() - w.getDate()));
      return B;
    }
    function m(h) {
      switch (h.getDay()) {
        case 0:
          return new Date(h.getFullYear() - 1, 11, 29);
        case 1:
          return h;
        case 2:
          return new Date(h.getFullYear(), 0, 3);
        case 3:
          return new Date(h.getFullYear(), 0, 2);
        case 4:
          return new Date(h.getFullYear(), 0, 1);
        case 5:
          return new Date(h.getFullYear() - 1, 11, 31);
        case 6:
          return new Date(h.getFullYear() - 1, 11, 30);
      }
    }
    function n(h) {
      var w = h.vb;
      for (h = new Date(new Date(h.wb + 1900, 0, 1).getTime()); 0 < w;) {
        var x = h.getMonth(),
          B = (Da(h.getFullYear()) ? Ed : Fd)[x];
        if (w > B - h.getDate()) w -= B - h.getDate() + 1, h.setDate(1), 11 > x ? h.setMonth(x + 1) : (h.setMonth(0), h.setFullYear(h.getFullYear() + 1));else {
          h.setDate(h.getDate() + w);
          break;
        }
      }
      x = new Date(h.getFullYear() + 1, 0, 4);
      w = m(new Date(h.getFullYear(), 0, 4));
      x = m(x);
      return 0 >= g(w, h) ? 0 >= g(x, h) ? h.getFullYear() + 1 : h.getFullYear() : h.getFullYear() - 1;
    }
    var q = D[d + 40 >> 2];
    d = {
      Oc: C[d >> 2],
      Nc: C[d + 4 >> 2],
      Sb: C[d + 8 >> 2],
      bc: C[d + 12 >> 2],
      Tb: C[d + 16 >> 2],
      wb: C[d + 20 >> 2],
      ob: C[d + 24 >> 2],
      vb: C[d + 28 >> 2],
      yd: C[d + 32 >> 2],
      Mc: C[d + 36 >> 2],
      Pc: q ? O(q) : ""
    };
    c = O(c);
    q = {
      "%c": "%a %b %d %H:%M:%S %Y",
      "%D": "%m/%d/%y",
      "%F": "%Y-%m-%d",
      "%h": "%b",
      "%r": "%I:%M:%S %p",
      "%R": "%H:%M",
      "%T": "%H:%M:%S",
      "%x": "%m/%d/%y",
      "%X": "%H:%M:%S",
      "%Ec": "%c",
      "%EC": "%C",
      "%Ex": "%m/%d/%y",
      "%EX": "%H:%M:%S",
      "%Ey": "%y",
      "%EY": "%Y",
      "%Od": "%d",
      "%Oe": "%e",
      "%OH": "%H",
      "%OI": "%I",
      "%Om": "%m",
      "%OM": "%M",
      "%OS": "%S",
      "%Ou": "%u",
      "%OU": "%U",
      "%OV": "%V",
      "%Ow": "%w",
      "%OW": "%W",
      "%Oy": "%y"
    };
    for (var t in q) c = c.replace(new RegExp(t, "g"), q[t]);
    var u = "Sunday Monday Tuesday Wednesday Thursday Friday Saturday".split(" "),
      z = "January February March April May June July August September October November December".split(" ");
    q = {
      "%a": h => u[h.ob].substring(0, 3),
      "%A": h => u[h.ob],
      "%b": h => z[h.Tb].substring(0, 3),
      "%B": h => z[h.Tb],
      "%C": h => f((h.wb + 1900) / 100 | 0, 2),
      "%d": h => f(h.bc, 2),
      "%e": h => e(h.bc, 2, " "),
      "%g": h => n(h).toString().substring(2),
      "%G": n,
      "%H": h => f(h.Sb, 2),
      "%I": h => {
        h = h.Sb;
        0 == h ? h = 12 : 12 < h && (h -= 12);
        return f(h, 2);
      },
      "%j": h => {
        for (var w = 0, x = 0; x <= h.Tb - 1; w += (Da(h.wb + 1900) ? Ed : Fd)[x++]);
        return f(h.bc + w, 3);
      },
      "%m": h => f(h.Tb + 1, 2),
      "%M": h => f(h.Nc, 2),
      "%n": () => "\n",
      "%p": h => 0 <= h.Sb && 12 > h.Sb ? "AM" : "PM",
      "%S": h => f(h.Oc, 2),
      "%t": () => "\t",
      "%u": h => h.ob || 7,
      "%U": h => f(Math.floor((h.vb + 7 - h.ob) / 7), 2),
      "%V": h => {
        var w = Math.floor((h.vb + 7 - (h.ob + 6) % 7) / 7);
        2 >= (h.ob + 371 - h.vb - 2) % 7 && w++;
        if (w) 53 == w && (x = (h.ob + 371 - h.vb) % 7, 4 == x || 3 == x && Da(h.wb) || (w = 1));else {
          w = 52;
          var x = (h.ob + 7 - h.vb - 1) % 7;
          (4 == x || 5 == x && Da(h.wb % 400 - 1)) && w++;
        }
        return f(w, 2);
      },
      "%w": h => h.ob,
      "%W": h => f(Math.floor((h.vb + 7 - (h.ob + 6) % 7) / 7), 2),
      "%y": h => (h.wb + 1900).toString().substring(2),
      "%Y": h => h.wb + 1900,
      "%z": h => {
        h = h.Mc;
        var w = 0 <= h;
        h = Math.abs(h) / 60;
        return (w ? "+" : "-") + String("0000" + (h / 60 * 100 + h % 60)).slice(-4);
      },
      "%Z": h => h.Pc,
      "%%": () => "%"
    };
    c = c.replace(/%%/g, "\x00\x00");
    for (t in q) c.includes(t) && (c = c.replace(new RegExp(t, "g"), q[t](d)));
    c = c.replace(/\0\0/g, "%");
    t = Db(c, !1);
    if (t.length > b) return 0;
    y.set(t, a);
    return t.length - 1;
  };
k.requestFullscreen = function (a, b) {
  function c() {
    var _k$onFullScreen, _k4, _k$onFullscreen, _k5;
    Ic = !1;
    var f = d.parentNode;
    (document.fullscreenElement || document.mozFullScreenElement || document.msFullscreenElement || document.webkitFullscreenElement || document.webkitCurrentFullScreenElement) === f ? (d.exitFullscreen = Rc, Pc && d.requestPointerLock(), Ic = !0, Qc ? ("undefined" != typeof SDL && (C[SDL.screen >> 2] = D[SDL.screen >> 2] | 8388608), Wc(k.canvas), Vc()) : Wc(d)) : (f.parentNode.insertBefore(d, f), f.parentNode.removeChild(f), Qc ? ("undefined" != typeof SDL && (C[SDL.screen >> 2] = D[SDL.screen >> 2] & -8388609), Wc(k.canvas), Vc()) : Wc(d));
    (_k$onFullScreen = (_k4 = k).onFullScreen) === null || _k$onFullScreen === void 0 || _k$onFullScreen.call(_k4, Ic);
    (_k$onFullscreen = (_k5 = k).onFullscreen) === null || _k$onFullscreen === void 0 || _k$onFullscreen.call(_k5, Ic);
  }
  Pc = a;
  Qc = b;
  "undefined" == typeof Pc && (Pc = !0);
  "undefined" == typeof Qc && (Qc = !1);
  var d = k.canvas;
  Oc || (Oc = !0, document.addEventListener("fullscreenchange", c, !1), document.addEventListener("mozfullscreenchange", c, !1), document.addEventListener("webkitfullscreenchange", c, !1), document.addEventListener("MSFullscreenChange", c, !1));
  var e = document.createElement("div");
  d.parentNode.insertBefore(e, d);
  e.appendChild(d);
  e.requestFullscreen = e.requestFullscreen || e.mozRequestFullScreen || e.msRequestFullscreen || (e.webkitRequestFullscreen ? () => e.webkitRequestFullscreen(Element.ALLOW_KEYBOARD_INPUT) : null) || (e.webkitRequestFullScreen ? () => e.webkitRequestFullScreen(Element.ALLOW_KEYBOARD_INPUT) : null);
  e.requestFullscreen();
};
k.requestAnimationFrame = Tc;
k.setCanvasSize = function (a, b, c) {
  Wc(k.canvas, a, b);
  c || Vc();
};
k.pauseMainLoop = function () {
  V = null;
  xc++;
};
k.resumeMainLoop = function () {
  xc++;
  var a = Ac,
    b = Bc,
    c = yc;
  yc = null;
  $c(c, 0, !1, zc, !0);
  Zc(a, b);
  V();
};
k.getUserMedia = function (a) {
  let b;
  (b = window).getUserMedia || (b.getUserMedia = navigator.getUserMedia || navigator.mozGetUserMedia);
  window.getUserMedia(a);
};
k.createContext = function (a, b, c, d) {
  if (b && k.Wb && a == k.canvas) return k.Wb;
  var e;
  if (b) {
    var f = {
      antialias: !1,
      alpha: !1,
      oc: 2
    };
    if (d) for (var g in d) f[g] = d[g];
    if ("undefined" != typeof rd && (e = md(a, f))) var m = jd[e].Fb;
  } else m = a.getContext("2d");
  if (!m) return null;
  c && (b || "undefined" == typeof Y || r("cannot set in module if GLctx is used, but we are a non-GL context that would replace it"), k.Wb = m, b && qd(e), k.zd = b, Kc.forEach(n => n()), Lc());
  return m;
};
function ac(a, b, c, d) {
  a || (a = this);
  this.parent = a;
  this.qb = a.qb;
  this.Pb = null;
  this.id = Ub++;
  this.name = b;
  this.mode = c;
  this.gb = {};
  this.ib = {};
  this.Qb = d;
}
Object.defineProperties(ac.prototype, {
  read: {
    get: function () {
      return 365 === (this.mode & 365);
    },
    set: function (a) {
      a ? this.mode |= 365 : this.mode &= -366;
    }
  },
  write: {
    get: function () {
      return 146 === (this.mode & 146);
    },
    set: function (a) {
      a ? this.mode |= 146 : this.mode &= -147;
    }
  }
});
oc();
Vb = Array(4096);
hc(S, "/");
U("/tmp");
U("/home");
U("/home/web_user");
(function () {
  U("/dev");
  Gb(259, {
    read: () => 0,
    write: (d, e, f, g) => g
  });
  jc("/dev/null", 259);
  Fb(1280, Ib);
  Fb(1536, Jb);
  jc("/dev/tty", 1280);
  jc("/dev/tty1", 1536);
  var a = new Uint8Array(1024),
    b = 0,
    c = () => {
      0 === b && (b = zb(a).byteLength);
      return a[--b];
    };
  qc("random", c);
  qc("urandom", c);
  U("/dev/shm");
  U("/dev/shm/tmp");
})();
(function () {
  U("/proc");
  var a = U("/proc/self");
  U("/proc/self/fd");
  hc({
    qb() {
      var b = Mb(a, "fd", 16895, 73);
      b.gb = {
        Cb(c, d) {
          var e = ec(+d);
          c = {
            parent: null,
            qb: {
              qc: "fake"
            },
            gb: {
              Db: () => e.path
            }
          };
          return c.parent = c;
        }
      };
      return b;
    }
  }, "/proc/self/fd");
})();
for (var Y, Hd = 0; 32 > Hd; ++Hd) wd.push(Array(Hd));
var Ld = {
    Ga: function (a, b, c) {
      var d = new XMLHttpRequest();
      d.timeout = c;
      d.open("GET", O(a), !0);
      d.responseType = "arraybuffer";
      d.ontimeout = function () {
        Id(b, d.status, !0);
      };
      d.onload = function () {
        if (200 == d.status) if (d.response) {
          var e = new Uint8Array(d.response),
            f = M(e.length);
          f ? (A.set(e, f), Jd(b, f, e.length), Kd(f)) : Id(b, 0, !1);
        } else Jd(b, 0, 0);
      };
      d.onerror = function () {
        Id(b, d.status, !1);
      };
      d.send(null);
    },
    e: function () {
      return performance.now();
    },
    Sa: function (a, b, c) {
      a = new Date(1E3 * (b + 2097152 >>> 0 < 4194305 - !!a ? (a >>> 0) + 4294967296 * b : NaN));
      C[c >> 2] = a.getSeconds();
      C[c + 4 >> 2] = a.getMinutes();
      C[c + 8 >> 2] = a.getHours();
      C[c + 12 >> 2] = a.getDate();
      C[c + 16 >> 2] = a.getMonth();
      C[c + 20 >> 2] = a.getFullYear() - 1900;
      C[c + 24 >> 2] = a.getDay();
      C[c + 28 >> 2] = (Da(a.getFullYear()) ? Ea : Fa)[a.getMonth()] + a.getDate() - 1 | 0;
      C[c + 36 >> 2] = -(60 * a.getTimezoneOffset());
      b = new Date(a.getFullYear(), 6, 1).getTimezoneOffset();
      var d = new Date(a.getFullYear(), 0, 1).getTimezoneOffset();
      C[c + 32 >> 2] = (b != d && a.getTimezoneOffset() == Math.min(d, b)) | 0;
    },
    F: (a, b, c) => {
      function d(n) {
        return (n = n.toTimeString().match(/\(([A-Za-z ]+)\)$/)) ? n[1] : "GMT";
      }
      var e = new Date().getFullYear(),
        f = new Date(e, 0, 1),
        g = new Date(e, 6, 1);
      e = f.getTimezoneOffset();
      var m = g.getTimezoneOffset();
      D[a >> 2] = 60 * Math.max(e, m);
      C[b >> 2] = Number(e != m);
      a = d(f);
      b = d(g);
      a = Ha(a);
      b = Ha(b);
      m < e ? (D[c >> 2] = a, D[c + 4 >> 2] = b) : (D[c >> 2] = b, D[c + 4 >> 2] = a);
    },
    f: () => {
      r("");
    },
    i: (a, b, c) => {
      Ia.length = 0;
      for (var d; d = A[b++];) {
        var e = 105 != d;
        e &= 112 != d;
        c += e && c % 8 ? 4 : 0;
        Ia.push(112 == d ? D[c >> 2] : 105 == d ? C[c >> 2] : F[c >> 3]);
        c += e ? 8 : 4;
      }
      return za[a].apply(null, Ia);
    },
    K: () => Date.now(),
    da: () => {
      if (!Wa()) return -1;
      Pa(Oa);
      var a = eb[1];
      if (a.exitFullscreen) a.fullscreenElement && a.exitFullscreen();else if (a.webkitExitFullscreen) a.webkitFullscreenElement && a.webkitExitFullscreen();else return -1;
      return 0;
    },
    Ha: () => {
      Pa(pb);
      if (document.exitPointerLock) document.exitPointerLock();else return -1;
      return 0;
    },
    Aa: () => devicePixelRatio,
    Ia: (a, b, c) => {
      a = P(a);
      if (!a) return -4;
      a = ob(a);
      F[b >> 3] = a.width;
      F[c >> 3] = a.height;
      return 0;
    },
    G: (a, b, c) => A.copyWithin(a, b, b + c),
    la: (a, b, c) => qb(a, {
      $b: C[c >> 2],
      Vb: C[c + 4 >> 2],
      Ec: C[c + 8 >> 2],
      Cc: b,
      Hb: C[c + 12 >> 2],
      fc: C[c + 16 >> 2]
    }),
    E: a => {
      var b = A.length;
      a >>>= 0;
      if (2147483648 < a) return !1;
      for (var c = 1; 4 >= c; c *= 2) {
        var d = b * (1 + .2 / c);
        d = Math.min(d, a + 100663296);
        var e = Math;
        d = Math.max(a, d);
        a: {
          e = (e.min.call(e, 2147483648, d + (65536 - d % 65536) % 65536) - fa.buffer.byteLength + 65535) / 65536;
          try {
            fa.grow(e);
            ka();
            var f = 1;
            break a;
          } catch (g) {}
          f = void 0;
        }
        if (f) return !0;
      }
      return !1;
    },
    N: ib,
    Ka: (a, b, c, d) => {
      if (!Wa()) return -1;
      a = P(a);
      if (!a) return -4;
      rb(a, b, c, d, "webkitfullscreenchange");
      return rb(a, b, c, d, "fullscreenchange");
    },
    Y: (a, b, c, d) => sb(a, b, c, d, 2, "keydown"),
    R: (a, b, c, d) => sb(a, b, c, d, 3, "keyup"),
    M: (a, b, c, d) => {
      $c(() => Q(a)(b), c, d, b);
    },
    L: (a, b, c, d) => ad(a, b, c, d, 5, "mousedown"),
    Qa: (a, b, c, d) => ad(a, b, c, d, 33, "mouseenter"),
    Pa: (a, b, c, d) => ad(a, b, c, d, 34, "mouseleave"),
    Ra: (a, b, c, d) => ad(a, b, c, d, 8, "mousemove"),
    Va: (a, b, c, d) => ad(a, b, c, d, 6, "mouseup"),
    Ja: (a, b, c, d) => {
      if (!document || !document.body || !(document.body.requestPointerLock || document.body.pb || document.body.Hc || document.body.Fc)) return -1;
      a = P(a);
      if (!a) return -4;
      bd(a, b, c, d, "mozpointerlockchange");
      bd(a, b, c, d, "webkitpointerlockchange");
      bd(a, b, c, d, "mspointerlockchange");
      return bd(a, b, c, d, "pointerlockchange");
    },
    La: (a, b, c, d) => cd(a, b, c, d, 25, "touchcancel"),
    Na: (a, b, c, d) => cd(a, b, c, d, 23, "touchend"),
    Ma: (a, b, c, d) => cd(a, b, c, d, 24, "touchmove"),
    Oa: (a, b, c, d) => cd(a, b, c, d, 22, "touchstart"),
    ta: a => document.title = O(a),
    P: (a, b) => {
      b >>= 2;
      b = {
        alpha: !!C[b],
        depth: !!C[b + 1],
        stencil: !!C[b + 2],
        antialias: !!C[b + 3],
        premultipliedAlpha: !!C[b + 4],
        preserveDrawingBuffer: !!C[b + 5],
        powerPreference: sd[C[b + 6]],
        failIfMajorPerformanceCaveat: !!C[b + 7],
        oc: C[b + 8],
        od: C[b + 9],
        ed: C[b + 10],
        Dc: C[b + 11],
        ud: C[b + 12],
        xd: C[b + 13]
      };
      a = P(a);
      return !a || b.Dc ? 0 : md(a, b);
    },
    Q: a => {
      a >>= 2;
      for (var b = 0; 14 > b; ++b) C[a + b] = 0;
      C[a] = C[a + 1] = C[a + 3] = C[a + 4] = C[a + 8] = C[a + 10] = 1;
    },
    p: a => qd(a) ? 0 : -5,
    C: (a, b) => {
      var c = 0;
      vd().forEach((d, e) => {
        var f = b + c;
        e = D[a + 4 * e >> 2] = f;
        for (f = 0; f < d.length; ++f) y[e++ >> 0] = d.charCodeAt(f);
        y[e >> 0] = 0;
        c += d.length + 1;
      });
      return 0;
    },
    D: (a, b) => {
      var c = vd();
      D[a >> 2] = c.length;
      var d = 0;
      c.forEach(e => d += e.length + 1);
      D[b >> 2] = d;
      return 0;
    },
    J: function (a) {
      try {
        var b = ec(a);
        if (null === b.xb) throw new R(8);
        b.Xb && (b.Xb = null);
        try {
          b.ib.close && b.ib.close(b);
        } catch (c) {
          throw c;
        } finally {
          Tb[b.xb] = null;
        }
        b.xb = null;
        return 0;
      } catch (c) {
        if ("undefined" == typeof sc || "ErrnoError" !== c.name) throw c;
        return c.Jb;
      }
    },
    I: function (a, b, c, d) {
      try {
        a: {
          var e = ec(a);
          a = b;
          for (var f, g = b = 0; g < c; g++) {
            var m = D[a >> 2],
              n = D[a + 4 >> 2];
            a += 8;
            var q = e,
              t = m,
              u = n,
              z = f,
              h = y;
            if (0 > u || 0 > z) throw new R(28);
            if (null === q.xb) throw new R(8);
            if (1 === (q.flags & 2097155)) throw new R(8);
            if (16384 === (q.node.mode & 61440)) throw new R(31);
            if (!q.ib.read) throw new R(28);
            var w = "undefined" != typeof z;
            if (!w) z = q.position;else if (!q.seekable) throw new R(70);
            var x = q.ib.read(q, h, t, u, z);
            w || (q.position += x);
            var B = x;
            if (0 > B) {
              var J = -1;
              break a;
            }
            b += B;
            if (B < n) break;
            "undefined" !== typeof f && (f += B);
          }
          J = b;
        }
        D[d >> 2] = J;
        return 0;
      } catch (L) {
        if ("undefined" == typeof sc || "ErrnoError" !== L.name) throw L;
        return L.Jb;
      }
    },
    Ta: function (a, b, c, d, e) {
      b = c + 2097152 >>> 0 < 4194305 - !!b ? (b >>> 0) + 4294967296 * c : NaN;
      try {
        if (isNaN(b)) return 61;
        var f = ec(a);
        nc(f, b, d);
        ya = [f.position >>> 0, (I = f.position, 1 <= +Math.abs(I) ? 0 < I ? +Math.floor(I / 4294967296) >>> 0 : ~~+Math.ceil((I - +(~~I >>> 0)) / 4294967296) >>> 0 : 0)];
        C[e >> 2] = ya[0];
        C[e + 4 >> 2] = ya[1];
        f.Xb && 0 === b && 0 === d && (f.Xb = null);
        return 0;
      } catch (g) {
        if ("undefined" == typeof sc || "ErrnoError" !== g.name) throw g;
        return g.Jb;
      }
    },
    H: function (a, b, c, d) {
      try {
        a: {
          var e = ec(a);
          a = b;
          for (var f, g = b = 0; g < c; g++) {
            var m = D[a >> 2],
              n = D[a + 4 >> 2];
            a += 8;
            var q = e,
              t = m,
              u = n,
              z = f,
              h = y;
            if (0 > u || 0 > z) throw new R(28);
            if (null === q.xb) throw new R(8);
            if (0 === (q.flags & 2097155)) throw new R(8);
            if (16384 === (q.node.mode & 61440)) throw new R(31);
            if (!q.ib.write) throw new R(28);
            q.seekable && q.flags & 1024 && nc(q, 0, 2);
            var w = "undefined" != typeof z;
            if (!w) z = q.position;else if (!q.seekable) throw new R(70);
            var x = q.ib.write(q, h, t, u, z, void 0);
            w || (q.position += x);
            var B = x;
            if (0 > B) {
              var J = -1;
              break a;
            }
            b += B;
            "undefined" !== typeof f && (f += B);
          }
          J = b;
        }
        D[d >> 2] = J;
        return 0;
      } catch (L) {
        if ("undefined" == typeof sc || "ErrnoError" !== L.name) throw L;
        return L.Jb;
      }
    },
    z: function (a) {
      Y.activeTexture(a);
    },
    B: (a, b) => {
      Y.attachShader(W[a], X[b]);
    },
    a: (a, b) => {
      35051 == a ? Y.ic = b : 35052 == a && (Y.Ib = b);
      Y.bindBuffer(a, ed[b]);
    },
    c: (a, b) => {
      Y.bindFramebuffer(a, fd[b]);
    },
    m: (a, b) => {
      Y.bindTexture(a, gd[b]);
    },
    g: a => {
      Y.bindVertexArray(hd[a]);
    },
    ua: function (a, b) {
      Y.blendFunc(a, b);
    },
    va: function (a, b, c, d) {
      Y.blendFuncSeparate(a, b, c, d);
    },
    k: (a, b, c, d) => {
      c && b ? Y.bufferData(a, A, d, c, b) : Y.bufferData(a, b, d);
    },
    v: (a, b, c, d) => {
      c && Y.bufferSubData(a, b, A, d, c);
    },
    Fa: function (a) {
      return Y.checkFramebufferStatus(a);
    },
    q: function (a) {
      Y.clear(a);
    },
    s: function (a, b, c, d) {
      Y.clearColor(a, b, c, d);
    },
    r: function (a) {
      Y.clearDepth(a);
    },
    ha: a => {
      Y.compileShader(X[a]);
    },
    Ea: () => {
      var a = kd(W),
        b = Y.createProgram();
      b.name = a;
      b.Nb = b.Lb = b.Mb = 0;
      b.cc = 1;
      W[a] = b;
      return a;
    },
    ja: a => {
      var b = kd(X);
      X[b] = Y.createShader(a);
      return b;
    },
    w: (a, b) => {
      for (var c = 0; c < a; c++) {
        var d = C[b + 4 * c >> 2],
          e = ed[d];
        e && (Y.deleteBuffer(e), e.name = 0, ed[d] = null, d == Y.ic && (Y.ic = 0), d == Y.Ib && (Y.Ib = 0));
      }
    },
    T: (a, b) => {
      for (var c = 0; c < a; ++c) {
        var d = C[b + 4 * c >> 2],
          e = fd[d];
        e && (Y.deleteFramebuffer(e), e.name = 0, fd[d] = null);
      }
    },
    wa: a => {
      if (a) {
        var b = W[a];
        b && (Y.deleteProgram(b), b.name = 0, W[a] = null);
      }
    },
    A: a => {
      if (a) {
        var b = X[a];
        b && (Y.deleteShader(b), X[a] = null);
      }
    },
    b: (a, b) => {
      for (var c = 0; c < a; c++) {
        var d = C[b + 4 * c >> 2],
          e = gd[d];
        e && (Y.deleteTexture(e), e.name = 0, gd[d] = null);
      }
    },
    ea: (a, b) => {
      for (var c = 0; c < a; c++) {
        var d = C[b + 4 * c >> 2];
        Y.deleteVertexArray(hd[d]);
        hd[d] = null;
      }
    },
    sa: function (a) {
      Y.depthFunc(a);
    },
    n: a => {
      Y.depthMask(!!a);
    },
    h: function (a) {
      Y.disable(a);
    },
    _: (a, b, c) => {
      Y.drawArrays(a, b, c);
    },
    S: (a, b) => {
      for (var c = wd[a], d = 0; d < a; d++) c[d] = C[b + 4 * d >> 2];
      Y.drawBuffers(c);
    },
    $: (a, b, c, d) => {
      Y.drawElements(a, b, c, d);
    },
    o: function (a) {
      Y.enable(a);
    },
    ca: a => {
      Y.enableVertexAttribArray(a);
    },
    u: (a, b, c, d, e) => {
      Y.framebufferTexture2D(a, b, c, gd[d], e);
    },
    ra: function (a) {
      Y.frontFace(a);
    },
    l: (a, b) => {
      xd(a, b, "createBuffer", ed);
    },
    U: (a, b) => {
      xd(a, b, "createFramebuffer", fd);
    },
    Z: (a, b) => {
      xd(a, b, "createTexture", gd);
    },
    fa: function (a, b) {
      xd(a, b, "createVertexArray", hd);
    },
    Ba: (a, b, c, d, e, f, g) => {
      yd("getActiveAttrib", a, b, c, d, e, f, g);
    },
    ya: (a, b, c, d, e, f, g) => {
      yd("getActiveUniform", a, b, c, d, e, f, g);
    },
    za: (a, b) => Y.getAttribLocation(W[a], O(b)),
    O: (a, b) => Ad(a, b),
    Ca: (a, b, c, d) => {
      a = Y.getProgramInfoLog(W[a]);
      b = 0 < b && d ? K(a, A, d, b) : 0;
      c && (C[c >> 2] = b);
    },
    d: (a, b, c) => {
      if (c && !(a >= dd)) if (a = W[a], 35716 == b) C[c >> 2] = Y.getProgramInfoLog(a).length + 1;else if (35719 == b) {
        if (!a.Nb) for (b = 0; b < Y.getProgramParameter(a, 35718); ++b) a.Nb = Math.max(a.Nb, Y.getActiveUniform(a, b).name.length + 1);
        C[c >> 2] = a.Nb;
      } else if (35722 == b) {
        if (!a.Lb) for (b = 0; b < Y.getProgramParameter(a, 35721); ++b) a.Lb = Math.max(a.Lb, Y.getActiveAttrib(a, b).name.length + 1);
        C[c >> 2] = a.Lb;
      } else if (35381 == b) {
        if (!a.Mb) for (b = 0; b < Y.getProgramParameter(a, 35382); ++b) a.Mb = Math.max(a.Mb, Y.getActiveUniformBlockName(a, b).length + 1);
        C[c >> 2] = a.Mb;
      } else C[c >> 2] = Y.getProgramParameter(a, b);
    },
    ga: (a, b, c, d) => {
      a = Y.getShaderInfoLog(X[a]);
      b = 0 < b && d ? K(a, A, d, b) : 0;
      c && (C[c >> 2] = b);
    },
    x: (a, b, c) => {
      c && (35716 == b ? (a = Y.getShaderInfoLog(X[a]), C[c >> 2] = a ? a.length + 1 : 0) : 35720 == b ? (a = Y.getShaderSource(X[a]), C[c >> 2] = a ? a.length + 1 : 0) : C[c >> 2] = Y.getShaderParameter(X[a], b));
    },
    xa: (a, b) => {
      b = O(b);
      if (a = W[a]) {
        var c = a,
          d = c.Eb,
          e = c.yc,
          f;
        if (!d) for (c.Eb = d = {}, c.xc = {}, f = 0; f < Y.getProgramParameter(c, 35718); ++f) {
          var g = Y.getActiveUniform(c, f);
          var m = g.name;
          g = g.size;
          var n = Bd(m);
          n = 0 < n ? m.slice(0, n) : m;
          var q = c.cc;
          c.cc += g;
          e[n] = [g, q];
          for (m = 0; m < g; ++m) d[q] = m, c.xc[q++] = n;
        }
        c = a.Eb;
        d = 0;
        e = b;
        f = Bd(b);
        0 < f && (d = parseInt(b.slice(f + 1)) >>> 0, e = b.slice(0, f));
        if ((e = a.yc[e]) && d < e[0] && (d += e[1], c[d] = c[d] || Y.getUniformLocation(a, b))) return d;
      }
      return -1;
    },
    Da: a => {
      a = W[a];
      Y.linkProgram(a);
      a.Eb = 0;
      a.yc = {};
    },
    X: (a, b) => {
      Y.pixelStorei(a, b);
    },
    ia: (a, b, c, d) => {
      for (var e = "", f = 0; f < b; ++f) {
        var g = d ? C[d + 4 * f >> 2] : -1;
        e += O(C[c + 4 * f >> 2], 0 > g ? void 0 : g);
      }
      Y.shaderSource(X[a], e);
    },
    W: (a, b, c, d, e, f, g, m, n) => {
      if (Y.Ib) Y.texImage2D(a, b, c, d, e, f, g, m, n);else if (n) {
        var q = Cd(m);
        Y.texImage2D(a, b, c, d, e, f, g, m, q, n >> 31 - Math.clz32(q.BYTES_PER_ELEMENT));
      } else Y.texImage2D(a, b, c, d, e, f, g, m, null);
    },
    j: function (a, b, c) {
      Y.texParameteri(a, b, c);
    },
    V: (a, b, c, d, e, f, g, m, n) => {
      if (Y.Ib) Y.texSubImage2D(a, b, c, d, e, f, g, m, n);else if (n) {
        var q = Cd(m);
        Y.texSubImage2D(a, b, c, d, e, f, g, m, q, n >> 31 - Math.clz32(q.BYTES_PER_ELEMENT));
      } else Y.texSubImage2D(a, b, c, d, e, f, g, m, null);
    },
    pa: (a, b, c) => {
      b && Y.uniform1fv(Dd(a), E, c >> 2, b);
    },
    y: (a, b) => {
      Y.uniform1i(Dd(a), b);
    },
    oa: (a, b, c) => {
      b && Y.uniform2fv(Dd(a), E, c >> 2, 2 * b);
    },
    na: (a, b, c) => {
      b && Y.uniform3fv(Dd(a), E, c >> 2, 3 * b);
    },
    ma: (a, b, c) => {
      b && Y.uniform4fv(Dd(a), E, c >> 2, 4 * b);
    },
    ka: (a, b, c, d) => {
      b && Y.uniformMatrix4fv(Dd(a), !!c, E, d >> 2, 16 * b);
    },
    qa: a => {
      a = W[a];
      Y.useProgram(a);
      Y.Bc = a;
    },
    aa: (a, b) => {
      Y.vertexAttribDivisor(a, b);
    },
    ba: (a, b, c, d, e, f) => {
      Y.vertexAttribPointer(a, b, c, !!d, e, f);
    },
    t: function (a, b, c, d) {
      Y.viewport(a, b, c, d);
    },
    Wa: Gd,
    Ua: (a, b, c, d) => Gd(a, b, c, d)
  },
  Z = function (_k$monitorRunDependen2, _k7) {
    function a(c) {
      var _k$monitorRunDependen, _k6;
      Z = c.exports;
      fa = Z.Xa;
      ka();
      lb = Z.cb;
      ma.unshift(Z.Ya);
      G--;
      (_k$monitorRunDependen = (_k6 = k).monitorRunDependencies) === null || _k$monitorRunDependen === void 0 || _k$monitorRunDependen.call(_k6, G);
      0 == G && (null !== qa && (clearInterval(qa), qa = null), ra && (c = ra, ra = null, c()));
      return Z;
    }
    var b = {
      a: Ld
    };
    G++;
    (_k$monitorRunDependen2 = (_k7 = k).monitorRunDependencies) === null || _k$monitorRunDependen2 === void 0 || _k$monitorRunDependen2.call(_k7, G);
    if (k.instantiateWasm) try {
      return k.instantiateWasm(b, a);
    } catch (c) {
      return p(`Module.instantiateWasm callback failed with error: ${c}`), !1;
    }
    xa(b, function (c) {
      a(c.instance);
    });
    return {};
  }(),
  M = a => (M = Z.Za)(a),
  Jd = k.__ae_FileSystem_ReadSuccess = (a, b, c) => (Jd = k.__ae_FileSystem_ReadSuccess = Z._a)(a, b, c),
  Id = k.__ae_FileSystem_ReadFail = (a, b, c) => (Id = k.__ae_FileSystem_ReadFail = Z.$a)(a, b, c),
  Kd = k.__ae_em_free = a => (Kd = k.__ae_em_free = Z.ab)(a),
  Md = k._main = (a, b) => (Md = k._main = Z.bb)(a, b),
  Ja = () => (Ja = Z.db)(),
  Ka = a => (Ka = Z.eb)(a),
  fb = a => (fb = Z.fb)(a);
k.___start_em_js = 38216;
k.___stop_em_js = 39046;
var Nd;
ra = function Od() {
  Nd || Pd();
  Nd || (ra = Od);
};
function Pd() {
  function a() {
    if (!Nd && (Nd = !0, k.calledRun = !0, !v)) {
      k.noFSInit || pc || (pc = !0, oc(), k.stdin = k.stdin, k.stdout = k.stdout, k.stderr = k.stderr, k.stdin ? qc("stdin", k.stdin) : kc("/dev/tty", "/dev/stdin"), k.stdout ? qc("stdout", null, k.stdout) : kc("/dev/tty", "/dev/stdout"), k.stderr ? qc("stderr", null, k.stderr) : kc("/dev/tty1", "/dev/stderr"), lc("/dev/stdin", 0), lc("/dev/stdout", 1), lc("/dev/stderr", 1));
      Wb = !1;
      Ba(ma);
      Ba(na);
      if (k.onRuntimeInitialized) k.onRuntimeInitialized();
      if (Qd) {
        var b = Md;
        try {
          var c = b(0, 0);
          ha = c;
          tc(c);
        } catch (d) {
          tb(d);
        }
      }
      if (k.postRun) for ("function" == typeof k.postRun && (k.postRun = [k.postRun]); k.postRun.length;) b = k.postRun.shift(), oa.unshift(b);
      Ba(oa);
    }
  }
  if (!(0 < G)) {
    if (k.preRun) for ("function" == typeof k.preRun && (k.preRun = [k.preRun]); k.preRun.length;) pa();
    Ba(la);
    0 < G || (k.setStatus ? (k.setStatus("Running..."), setTimeout(function () {
      setTimeout(function () {
        k.setStatus("");
      }, 1);
      a();
    }, 1)) : a());
  }
}
if (k.preInit) for ("function" == typeof k.preInit && (k.preInit = [k.preInit]); 0 < k.preInit.length;) k.preInit.pop()();
var Qd = !0;
k.noInitialRun && (Qd = !1);
Pd();
