// Extracts the simulation core from busy-beaver-decoded.html and asserts every number the page states.
// usage: node site/core_test.js [path-to-html]
const fs = require('fs');
const path = process.argv[2] || require('path').join(__dirname, 'busy-beaver-decoded.html');
const html = fs.readFileSync(path, 'utf8');
const core = html.split('// ==CORE-START==')[1].split('// ==CORE-END==')[0];
const api = new Function(core + '; return {FullRun, simRound, traceRound, predict, orbit, survival, antihydra, fmt};')();
let fails = 0;
const check = (name, ok, extra='') => { console.log((ok ? 'PASS ' : 'FAIL ') + name + (extra ? '  ' + extra : '')); if (!ok) fails++; };

// 1. full run
{ const r = new api.FullRun(); let x; const t0 = Date.now(); do { x = r.step(3000000); } while (!x.halted && x.steps < 90000000);
  check('full run: 47,176,870 steps', x.steps === 47176870, `got ${x.steps}`);
  check('full run: 4,098 ones', x.ones === 4098, `got ${x.ones}`);
  check('full run: 12,289 cells', x.cells === 12289, `got ${x.cells}`);
  console.log('     (node time ' + (Date.now() - t0) + ' ms)'); }

// 2. rule vs machine for n = 1..250, and the round explorer's range 1..150 with frames
{ let bad = 0; for (let n = 1; n <= 250; n++) { const s = api.simRound(n), p = api.predict(n);
    const ok = s.kind === p.kind && s.steps === p.steps && (s.kind === 'epoch' ? (s.ones === p.next && s.solid) : s.ones === p.ones);
    if (!ok) { bad++; if (bad < 5) console.log('   mismatch n=' + n, s, p); } }
  check('rule matches machine for all n = 1..250', bad === 0, `${bad} mismatches`); }

// 3. frames for the traced round: first frame is the block, last frame has the expected ones count, head/state arrays sane
{ for (const n of [1, 3, 37, 35, 117, 150]) { const t = api.traceRound(n); const c = t.cells;
    let ones0 = 0, onesN = 0; for (let k = 0; k < c; k++) { ones0 += t.frames[k]; onesN += t.frames[t.steps * c + k]; }
    const expected = t.result.ones;
    check(`trace n=${n}: ${t.steps} steps, ${c} cells, first-frame ones=${ones0}, last-frame ones=${onesN}`, ones0 === n && onesN === expected && t.heads[0] >= 0 && t.heads[0] < c && t.heads[t.steps] >= 0 && t.heads[t.steps] < c); } }
check('trace n=37 is 937 steps and 67 ones', (() => { const t = api.traceRound(37); return t.steps === 937 && t.result.ones === 67 && t.cells === 68; })());

// 4. orbit
{ const o = api.orbit();
  check('orbit has 15 rounds, last n = 12287', o.rows.length === 15 && o.rows[14].n === 12287);
  check('orbit total steps = 47,176,870 and final ones 4,098', o.total === 47176870 && o.finalOnes === 4098, `total ${o.total}, ones ${o.finalOnes}`);
  check('orbit epoch times match the machine (3, 24, 107, 402, ..., 47164581)', [3,24,107,402,1339,4146,12185,35100,99737,280426,785091,2190120,6098283,16963136,47164581].every((t,i)=>o.rows[i].t===t));
  const share = (o.rows[14].t - o.rows[13].t) / o.total; check('last round share is 64%', Math.round(share*100) === 64, share.toFixed(4)); }

// 5. survival
{ const G = api.survival(1000000);
  check('survival: P(>=14) = 0.343% observed', Math.abs(G[14]*100 - 0.343) < 0.0015, (G[14]*100).toFixed(4) + '%');
  check('survival: (2/3)^14 = 0.343%', (Math.pow(2/3,14)*100).toFixed(3) === '0.343');
  check('survival: every k up to 26 has data', [...Array(26)].every((_, i) => G[i+1] > 0)); }

// 6. Antihydra arithmetic
{ const a = api.antihydra(2000000, 500);
  check('antihydra: counter after 2,000,000 rounds = 996,805', a.finalB === 996805, `got ${a.finalB}`);
  check('antihydra: evens/odds = 998,935 / 1,001,065', a.E === 998935 && a.O === 1001065);
  const s = api.antihydra(60, 60); const b10 = s.pts.find(p => p[0] === 10)[1];
  check('antihydra: counter after 10 rounds = 11', b10 === 11, `got ${b10}`);
  check('antihydra: min since round 2 (n>2) is 5', s.bmin === 5, `got ${s.bmin} at ${s.bminAt}`); }

// 7. formatting
check('fmt', api.fmt(47176870) === '47,176,870');
console.log(fails === 0 ? '\nALL PASS' : `\n${fails} FAILURES`);
process.exit(fails ? 1 : 0);
