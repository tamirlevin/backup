# Pre-registration: what I *think* I know about Busy Beaver machines

Written 2026-09-30, **before** running any enumeration code and **before** looking anything
up. The only things done before this file: `ls`, `git status`, checking which Python
packages exist, reading the proxy README. Nothing here has been checked; it is all recall
and estimation. The commit hash of this file is the timestamp.

## Conventions (frozen)

* 2 symbols {0,1}, *n* states, blank tape, start in state A, head at cell 0.
* Every executed transition is one step, **including the final halting one**.
  S = steps at halt. Sigma = number of 1s on the tape after the halting transition, taking
  the halting transition to write a 1 (best case).
* Enumeration ("lazy tree normal form"). A *node* is a partial transition table.
  Simulate it from the blank tape for at most CAP = 10 000 steps.
  * It reaches an undefined entry (state s reading symbol a) after t steps: the node is
    **internal** and doubles as one halting machine, S = t+1, Sigma = ones + (a==0).
  * It survives CAP steps with no undefined entry: it is a **cap leaf** (not halted).
  * Children of an internal node exist only if >= 2 entries are still undefined (a last
    entry that is not "halt" gives a table with no halting transition, which can never
    halt). Each child defines the reached entry as (write in {0,1}, dir in {L,R},
    next in {states already seen} + {the next unseen state, if any}). The very first
    transition is fixed to dir = R (mirror symmetry). Nothing else is pruned; in
    particular A0 is *not* forced to be 1RB.
  * N_sim = nodes simulated, N_halt = internal nodes, N_cap = cap leaves.
* Text format for machines: bbchallenge style, `1RB1LC_...`, `---` = undefined/halt.

## Part A: recalled facts (my probability that the statement is exactly right)

| id | claim | p |
|----|-------|---|
| A1 | BB(1): S=1, Sigma=1 | 0.99 |
| A2 | BB(2): S=6, Sigma=4, one machine (up to mirror) does both | 0.93 |
| A3a | BB(3): max S = 21 | 0.97 |
| A3b | BB(3): max Sigma = 6 | 0.97 |
| A3c | BB(3): the S=21 machine leaves only 5 ones, and the Sigma=6 machine halts after only 11 steps (different machines) | 0.75 |
| A4a | BB(4): max S = 107 | 0.97 |
| A4b | BB(4): max Sigma = 13 | 0.97 |
| A4c | the same 4-state machine attains both | 0.90 |
| A4d | a second, different 4-state machine also has Sigma=13, and it takes 96 steps | 0.55 |
| A5a | BB(5): S = 47,176,870 (Marxen & Buntrock, 1989 champion) | 0.93 |
| A5b | that machine leaves Sigma = 4098 ones | 0.92 |
| A6 | its table is exactly `1RB1LC_1RC1RB_1RD0LE_1LA1LD_---0LA` (halts at E0) | 0.75 |
| A7a | it visits exactly 12,289 distinct cells | 0.75 |
| A7b | final tape: 4098 ones and 8191 zeros inside the visited span | 0.65 |
| A8 | BB(5) was settled in 2024 by the bbchallenge collaboration, with a Coq proof | 0.90 |
| A9a | mxdys (2025) showed BB(6) > 2^^^5 (up-arrows), beating Kropitz's 2022 bound of 10^^15 | 0.70 |
| A9b | a still-larger BB(6) lower bound has appeared since (as of 2026-09-30) | 0.40 |
| A10a | "Antihydra" is a 6-state 2-symbol machine whose halting is equivalent to an open Collatz-like problem | 0.85 |
| A10b | its table is `1RB1RA_0LC1LE_1LD0LC_1LA1LB_1LF0LE_---0LA` | 0.20 |
| A11 | bbchallenge's 5-state seed database has 88,664,064 machines | 0.75 |
| A12 | no 4-state machine halts with 107 < S <= 100,000 (a bug check as much as a memory check) | 0.99 |

## Part B: quantities I have never seen (80% intervals; point guess in brackets)

| id | quantity (using the frozen enumeration, CAP = 10^4) | 80% interval [point] |
|----|------|------|
| B1 | N_sim, n=2 | 60 to 400 [150] |
| B2 | N_sim, n=3 | 3,000 to 60,000 [12,000] |
| B3 | N_sim, n=4 | 5e5 to 3e7 [2e6] |
| B4 | N_halt / N_sim, n=4 | 0.15 to 0.60 [0.35] |
| B5 | second-largest S over distinct halting nodes, n=4 (largest is 107) | 96 to 106 [96] |
| B6 | second-largest S, n=3 (largest is 21) | 12 to 20 [14] |

## Part C: cheap deciders

Take every cap leaf and re-simulate up to H = 100,000 steps. Call it decided if the exact
configuration (state, head, whole tape) repeats (*cycler*), or if the head keeps setting
new right-most (or left-most) records and the tape behind it repeats up to translation
(*translated cycler*, soundness argument in the code). X_n = fraction of cap leaves neither
decides.

| id | quantity | 80% interval [point] |
|----|----------|------|
| C1 | X_2 | 0 to 0.10 [0] |
| C2 | X_3 | 0 to 0.25 [0.05] |
| C3 | X_4 | 0.02 to 0.40 [0.12] |

## How this will be scored

Part A: Brier score over the statements, plus which misses were the confident ones.
Parts B/C: did the truth land inside my 80% interval (a calibrated me hits about 4 of 5).
Web lookups (A8, A9, A10a, A11) happen only after this file is committed.
