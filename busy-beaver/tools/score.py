"""Score PREREGISTRATION.md against what was checked. Prints the markdown tables used in RESULTS.md."""
A = [  # id, claim (short), p, outcome (1 true / 0 false), how checked, note
 ("A1","BB(1): S=1, Sigma=1",.99,1,"code","enumerator n=1"),
 ("A2","BB(2): S=6, Sigma=4, one machine does both",.93,1,"code","`1RB1LB_1LA---`; the only Sigma=4 node"),
 ("A3a","BB(3): max S = 21",.97,1,"code","tree search = brute force over 16.7M tables"),
 ("A3b","BB(3): max Sigma = 6",.97,1,"code","same"),
 ("A3c","S=21 machine leaves 5 ones; the Sigma=6 machine takes 11 steps",.75,0,"code","first half right; there are FIVE Sigma=6 machines, taking 11, 12, 13, 13 and 14 steps"),
 ("A4a","BB(4): max S = 107",.97,1,"code","unique node"),
 ("A4b","BB(4): max Sigma = 13",.97,1,"code",""),
 ("A4c","same machine attains both",.90,1,"code","`1RB1LB_1LA0LC_---1LD_1RD0RA`"),
 ("A4d","a second Sigma=13 machine takes 96 steps",.55,1,"code","`1RB0RC_1LA1RA_---1RD_1LD0LB`"),
 ("A5a","BB(5) champion: S = 47,176,870",.93,1,"code + literature","simulated; optimality per the 2024 proof"),
 ("A5b","it leaves 4098 ones",.92,1,"code",""),
 ("A6","its table is `1RB1LC_1RC1RB_1RD0LE_1LA1LD_---0LA`",.75,1,"code","reproduces both numbers exactly"),
 ("A7a","it visits exactly 12,289 cells",.75,1,"code",""),
 ("A7b","final tape: 4098 ones, 8191 zeros",.65,1,"code",""),
 ("A8","BB(5) settled in 2024 by bbchallenge, Coq proof",.90,1,"web (search summaries)","proof announced 2024-07-02; Coq by mxdys"),
 ("A9a","mxdys 2025: BB(6) > 2^^^5, beating Kropitz's 2022 10^^15",.70,1,"web (search summaries)","Kropitz May 2022; mxdys June 2025, via intermediate mxdys bounds"),
 ("A10a","Antihydra: 6-state, Collatz-like, halting open",.85,1,"web + code","its map verified by simulation"),
 ("A10b","its table is `1RB1RA_0LC1LE_1LD0LC_1LA1LB_1LF0LE_---0LA`",.20,0,"code","halts in 10 steps; 4 of 12 entries wrong (C1, D1, E1, F1)"),
 ("A11","bbchallenge seed database has 88,664,064 machines",.75,1,"web (search summaries)","holdouts after the 47,176,870-step run, out of 181,385,789 TNF machines"),
 ("A12","no 4-state machine halts with 107 < S <= 100,000",.99,1,"code","see RESULTS.md"),
]
n=len(A); brier=sum((p-o)**2 for _,_,p,o,_,_ in A)/n
print("| id | claim | my p | outcome | checked by | note |\n|---|---|---|---|---|---|")
for i,c,p,o,how,note in A:
    print(f"| {i} | {c} | {p:.2f} | {'true' if o else 'FALSE'} | {how} | {note} |")
print(f"\nA9b (a larger BB(6) bound has appeared since mid-2025, p=0.40): **unresolved**, not scored.")
print(f"\nBrier score over {n} scored statements: {brier:.4f} (always answering 0.5 scores 0.25).")
exp=sum(a[2] for a in A)
print(f"Expected number true if I were perfectly calibrated: {exp:.1f}; actual: {sum(a[3] for a in A)}.")
hi=[a for a in A if a[2]>=.9]; mid=[a for a in A if .5<=a[2]<.9]
print(f"Bin p>=0.90: {sum(a[3] for a in hi)}/{len(hi)} true (expected {sum(a[2] for a in hi):.1f}). Bin 0.55-0.85: {sum(a[3] for a in mid)}/{len(mid)} true (expected {sum(a[2] for a in mid):.1f}).")
B = [ # id, quantity, lo, hi, point, actual
 ("B1","N_sim, n=2",60,400,150,61),
 ("B2","N_sim, n=3",3000,60000,12000,5417),
 ("B3","N_sim, n=4",5e5,3e7,2e6,858909),
 ("B4","N_halt/N_sim, n=4",.15,.60,.35,249693/858909),
 ("B5","runner-up S below 107, n=4",96,106,96,97),
 ("B6","runner-up S below 21, n=3",12,20,14,20),
 ("C1","X_2 (undecided fraction)",0,.10,0,0/42),
 ("C2","X_3",0,.25,.05,92/3645),
 ("C3","X_4",.02,.40,.12,20560/609216),
]
print("\n| id | quantity | my 80% interval | point guess | actual | inside? | point off by |\n|---|---|---|---|---|---|---|")
hits=0
for i,q,lo,hi_,pt,act in B:
    inside = lo<=act<=hi_; hits+=inside
    off = "" if pt==0 or act==0 else f"{pt/act:.1f}x too {'high' if pt>act else 'low'}" if abs(pt/act-1)>.05 else "about right"
    fmt=lambda x: f"{x:,.0f}" if x>=100 else f"{x:.4g}"
    print(f"| {i} | {q} | {fmt(lo)} to {fmt(hi_)} | {fmt(pt)} | {fmt(act)} | {'yes' if inside else 'NO'} | {off} |")
print(f"\n{hits} of {len(B)} intervals contained the truth (80% intervals should contain about {0.8*len(B):.1f}).")
