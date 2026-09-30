import base64, math, os
here = os.path.dirname(os.path.abspath(__file__))
png = base64.b64encode(open(os.path.join(here,'..','figures','bb5_epoch_37_to_67.png'),'rb').read()).decode()
epochs=[(3,3),(9,24),(19,107),(37,402),(67,1339),(117,4146),(199,12185),(337,35100),(567,99737),(949,280426),(1587,785091),(2649,2190120),(4419,6098283),(7369,16963136),(12287,47164581)]
def compact(t):
    if t<1000: return str(t)
    if t<1e6: return f"{t/1000:.0f}k" if t>=1e4 else f"{t/1000:.1f}k"
    return f"{t/1e6:.1f}M" if t<1e7 else f"{t/1e6:.0f}M"
W,H=1200,980
INK='#1b2333'; MUTE='#5d677a'; GRID='#e3e6ec'
C0='#2b6cb0'; C1='#d9822b'; C2='#c53030'   # residue classes 0,1,2
o=[]
o.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" font-family="system-ui, -apple-system, Segoe UI, Roboto, sans-serif">')
o.append(f'<rect width="{W}" height="{H}" fill="#ffffff"/>')
o.append(f'<text x="50" y="52" font-size="25" font-weight="650" fill="{INK}">The 5-state champion is a counter that halts the first time n ≡ 2 (mod 3)</text>')
o.append(f'<text x="50" y="80" font-size="14" fill="{MUTE}" font-family="ui-monospace, SFMono-Regular, Menlo, Consolas, monospace">1RB1LC_1RC1RB_1RD0LE_1LA1LD_---0LA   ·   47,176,870 steps   ·   4,098 ones</text>')
# ---- panel A
ax,ay,aw=50,150,1100; ah=aw*408/1876
o.append(f'<text x="50" y="112" font-size="17" font-weight="600" fill="{INK}">One round, every step drawn: a block of 37 ones becomes a block of 67</text>')
o.append(f'<text x="50" y="132" font-size="13" fill="{MUTE}">time runs left to right (937 steps); the tape runs top (left end) to bottom (right end); dark = 1, white = 0</text>')
o.append(f'<image x="{ax}" y="{ay}" width="{aw}" height="{ah:.1f}" preserveAspectRatio="none" href="data:image/png;base64,{png}"/>')
o.append(f'<rect x="{ax}" y="{ay}" width="{aw}" height="{ah:.1f}" fill="none" stroke="{GRID}"/>')
ly=ay+ah+24
def key(x,color,label):
    return f'<rect x="{x}" y="{ly-11}" width="14" height="4" fill="{color}"/><text x="{x+22}" y="{ly-4}" font-size="13" fill="{INK}">{label}</text>'
o.append(key(50,'#f2a900','head in state B, sweeping right through the block'))
o.append(key(420,'#6a4c93','head in state D, sweeping back left'))
o.append(key(720,'#e4572e','turning points (states A, C, E) are single steps'))
o.append(f'<text x="{ax+4}" y="{ay+ah+50:.0f}" font-size="13" fill="{INK}">start: 37 ones</text>')
o.append(f'<text x="{ax+aw-4}" y="{ay+ah+50:.0f}" font-size="13" fill="{INK}" text-anchor="end">end: 67 ones, and the next round starts</text>')
# ---- panel B
by0=ay+ah+96
o.append(f'<text x="50" y="{by0}" font-size="17" font-weight="600" fill="{INK}">The whole run: 15 rounds, each starting from a solid block of n ones</text>')
o.append(f'<text x="50" y="{by0+20}" font-size="13" fill="{MUTE}">log scale; a straight line means growth by a constant factor (5/3 per round). Colour = n mod 3.</text>')
px0,px1=110,1130; py0,py1=by0+270, by0+70   # bottom(n=1), top(n=20000)
def X(k): return px0+(k-1)*(px1-px0)/14
def Y(n): return py0-(py0-py1)*math.log10(n)/math.log10(20000)
for n in (1,10,100,1000,10000):
    o.append(f'<line x1="{px0-10}" y1="{Y(n):.1f}" x2="{px1+20}" y2="{Y(n):.1f}" stroke="{GRID}"/><text x="{px0-16}" y="{Y(n)+4:.1f}" font-size="12" fill="{MUTE}" text-anchor="end">{n:,}</text>')
pts=[]
for k,(n,t) in enumerate(epochs,1):
    pts.append(f'{X(k):.1f},{Y(n):.1f}')
o.append(f'<polyline points="{" ".join(pts)}" fill="none" stroke="#9aa3b2" stroke-width="1.5"/>')
for k,(n,t) in enumerate(epochs,1):
    r=n%3; col=[C0,C1,C2][r]
    o.append(f'<circle cx="{X(k):.1f}" cy="{Y(n):.1f}" r="6" fill="{col}"/>')
    dy=-14 if k%2 else 22
    o.append(f'<text x="{X(k):.1f}" y="{Y(n)+dy:.1f}" font-size="12" fill="{INK}" text-anchor="middle">{n:,}</text>')
    o.append(f'<text x="{X(k):.1f}" y="{py0+26}" font-size="12" fill="{MUTE}" text-anchor="middle">{compact(t)}</text>')
o.append(f'<text x="{px0-16}" y="{py0+26}" font-size="12" fill="{MUTE}" text-anchor="end">step:</text>')
# legend
lx=px0; lyy=py0+56
for i,(c,l) in enumerate(((C0,'n ≡ 0 (mod 3):  n → (5n+12)/3'),(C1,'n ≡ 1 (mod 3):  n → (5n+16)/3'),(C2,'n ≡ 2 (mod 3):  halt'))):
    x=lx+i*330
    o.append(f'<circle cx="{x}" cy="{lyy-4}" r="6" fill="{c}"/><text x="{x+14}" y="{lyy}" font-size="14" fill="{INK}">{l}</text>')
# annotation for halt
hx,hy=X(15),Y(12287)
o.append(f'<text x="{hx-12:.0f}" y="{hy-30:.0f}" font-size="13" fill="{C2}" text-anchor="end" font-weight="600">12,287 ≡ 2: the machine sweeps left for 12,289 more steps and halts</text>')
# footer
fy=lyy+52
o.append(f'<text x="50" y="{fy}" font-size="14" fill="{INK}" font-weight="600">Checked, not assumed</text>')
lines=['From n ones the machine reaches the next block after (5n² + 33n + 45)/9 steps if n ≡ 0, or (5n² + 41n + 71)/9 if n ≡ 1 (mod 3); if n ≡ 2 it halts after n + 2 steps.',
       'I fitted these constants on the champion\'s own orbit, then ran the machine from every block n = 1…250: 250 of 250 matched exactly.',
       'The orbit 3 → 9 → 19 → … → 12,287 then reproduces S = 47,176,870 and Σ = 4,098. Verified by simulation, not proved for all n by this code.',
       'Why it is a lottery win: of the first million starting blocks, 0.343% survive 14 rounds, and (2/3)^14 = 0.343%. This start dodged residue 2 fourteen times.']
for i,l in enumerate(lines): o.append(f'<text x="50" y="{fy+22+i*20}" font-size="13" fill="{MUTE}">{l}</text>')
o.append('</svg>')
open(os.path.join(here,'..','figures','bb5_champion.svg'),'w').write('\n'.join(o))
print("ok", W, H, "panel A height", round(ah), "footer y", fy+62)
