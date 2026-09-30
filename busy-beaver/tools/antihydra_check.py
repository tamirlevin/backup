"""Compare the Antihydra machine's tape at its 'epochs' with the Collatz-like map a -> a + a//2 from a=8,
then iterate that map 2,000,000 times with big integers and watch the halting counter.

usage: python3 tools/antihydra_check.py results/antihydra_epochs.txt
"""
import re, sys, time
rows = []
for l in open(sys.argv[1]):
    m = re.match(r't=(\d+) other shape nr=6: 1\^(\d+) 0\^1 1\^(\d+)', l)
    if m: rows.append(tuple(map(int, m.groups())))
a = 8; b = 0; seq = []
for k in range(len(rows) + 2):
    b += 2 if a % 2 == 0 else -1          # counter: +2 on an even value, -1 on an odd one
    seq.append((a, b)); a = a + a // 2
ok_q = sum(q == seq[k + 1][0] - 5 for k, (t, p, q) in enumerate(rows))
ok_p = sum(p == seq[k][1] for k, (t, p, q) in enumerate(rows))
print("epochs read from the machine:", len(rows), "| middle block q == a_{k+1} - 5 :", ok_q, "| left block p == counter b_k :", ok_p)
print("sequence a_k:", [x for x, _ in seq[:10]], "...")
print("last epoch at step %d (%.2e); a = %d" % (rows[-1][0], rows[-1][0], seq[len(rows)][0]))
print("\nmap iterated far past what the machine can reach (halts iff the counter b = 2E - O would go below 0):")
t0 = time.time(); a = 8; E = O = 0; bmin = None; bmin_at = None
N = 2_000_000; marks = {10 ** k: None for k in range(1, 7)}; marks[N] = None
for n in range(1, N + 1):
    if a & 1: O += 1
    else: E += 1
    b = 2 * E - O
    if n > 1 and (bmin is None or b < bmin): bmin, bmin_at = b, n
    a += a >> 1
    if n in marks: marks[n] = (b, E, O)
    if b < 0: print("counter went negative at n =", n); break
for n, v in marks.items():
    if v: print(f"n={n:>8}  b={v[0]:>8}  E={v[1]:>8}  O={v[2]:>8}  b/n={v[0] / n:.5f}")
print("smallest b over n = 2..N:", bmin, "at n =", bmin_at, "| bits in a at the end:", a.bit_length(), "| %.0fs" % (time.time() - t0))
