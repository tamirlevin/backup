| id | claim | my p | outcome | checked by | note |
|---|---|---|---|---|---|
| A1 | BB(1): S=1, Sigma=1 | 0.99 | true | code | enumerator n=1 |
| A2 | BB(2): S=6, Sigma=4, one machine does both | 0.93 | true | code | `1RB1LB_1LA---`; the only Sigma=4 node |
| A3a | BB(3): max S = 21 | 0.97 | true | code | tree search = brute force over 16.7M tables |
| A3b | BB(3): max Sigma = 6 | 0.97 | true | code | same |
| A3c | S=21 machine leaves 5 ones; the Sigma=6 machine takes 11 steps | 0.75 | FALSE | code | first half right; there are FIVE Sigma=6 machines, taking 11, 12, 13, 13 and 14 steps |
| A4a | BB(4): max S = 107 | 0.97 | true | code | unique node |
| A4b | BB(4): max Sigma = 13 | 0.97 | true | code |  |
| A4c | same machine attains both | 0.90 | true | code | `1RB1LB_1LA0LC_---1LD_1RD0RA` |
| A4d | a second Sigma=13 machine takes 96 steps | 0.55 | true | code | `1RB0RC_1LA1RA_---1RD_1LD0LB` |
| A5a | BB(5) champion: S = 47,176,870 | 0.93 | true | code + literature | simulated; optimality per the 2024 proof |
| A5b | it leaves 4098 ones | 0.92 | true | code |  |
| A6 | its table is `1RB1LC_1RC1RB_1RD0LE_1LA1LD_---0LA` | 0.75 | true | code | reproduces both numbers exactly |
| A7a | it visits exactly 12,289 cells | 0.75 | true | code |  |
| A7b | final tape: 4098 ones, 8191 zeros | 0.65 | true | code |  |
| A8 | BB(5) settled in 2024 by bbchallenge, Coq proof | 0.90 | true | web (search summaries) | proof announced 2024-07-02; Coq by mxdys |
| A9a | mxdys 2025: BB(6) > 2^^^5, beating Kropitz's 2022 10^^15 | 0.70 | true | web (search summaries) | Kropitz May 2022; mxdys June 2025, via intermediate mxdys bounds |
| A10a | Antihydra: 6-state, Collatz-like, halting open | 0.85 | true | web + code | its map verified by simulation |
| A10b | its table is `1RB1RA_0LC1LE_1LD0LC_1LA1LB_1LF0LE_---0LA` | 0.20 | FALSE | code | halts in 10 steps; 4 of 12 entries wrong (C1, D1, E1, F1) |
| A11 | bbchallenge seed database has 88,664,064 machines | 0.75 | true | web (search summaries) | holdouts after the 47,176,870-step run, out of 181,385,789 TNF machines |
| A12 | no 4-state machine halts with 107 < S <= 100,000 | 0.99 | true | code | see RESULTS.md |

A9b (a larger BB(6) bound has appeared since mid-2025, p=0.40): **unresolved**, not scored.

Brier score over 20 scored statements: 0.0634 (always answering 0.5 scores 0.25).
Expected number true if I were perfectly calibrated: 16.4; actual: 18.
Bin p>=0.90: 11/11 true (expected 10.4). Bin 0.55-0.85: 7/8 true (expected 5.7).

| id | quantity | my 80% interval | point guess | actual | inside? | point off by |
|---|---|---|---|---|---|---|
| B1 | N_sim, n=2 | 60 to 400 | 150 | 61 | yes | 2.5x too high |
| B2 | N_sim, n=3 | 3,000 to 60,000 | 12,000 | 5,417 | yes | 2.2x too high |
| B3 | N_sim, n=4 | 500,000 to 30,000,000 | 2,000,000 | 858,909 | yes | 2.3x too high |
| B4 | N_halt/N_sim, n=4 | 0.15 to 0.6 | 0.35 | 0.2907 | yes | 1.2x too high |
| B5 | runner-up S below 107, n=4 | 96 to 106 | 96 | 97 | yes | about right |
| B6 | runner-up S below 21, n=3 | 12 to 20 | 14 | 20 | yes | 0.7x too low |
| C1 | X_2 (undecided fraction) | 0 to 0.1 | 0 | 0 | yes |  |
| C2 | X_3 | 0 to 0.25 | 0.05 | 0.02524 | yes | 2.0x too high |
| C3 | X_4 | 0.02 to 0.4 | 0.12 | 0.03375 | yes | 3.6x too high |

9 of 9 intervals contained the truth (80% intervals should contain about 7.2).
