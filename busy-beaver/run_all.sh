#!/usr/bin/env bash
# Regenerates everything in results/ (about 10 minutes on 4 cores). Run from anywhere.
# The two PNG/SVG figures are made separately: python3 tools/spacetime.py && python3 tools/mkfig.py
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build results
gcc -O2 -march=native -fopenmp -o build/bb bb.c
gcc -O2 -march=native -fopenmp -o build/deciders deciders.c
gcc -O2 -march=native -o build/brute brute.c
gcc -O2 -march=native -o build/sim sim.c -lm
gcc -O2 -o build/epochs_bb5 tools/epochs_bb5.c
gcc -O2 -o build/epochs_antihydra tools/epochs_antihydra.c

echo "== 1. enumeration n=1..4 (CAP = 10^4); the S:maxSigma table is appended after the summary"
BB_FULL=1 build/bb 1 10000 1 1        build/cap1.bin > results/enumeration_n1.txt
BB_FULL=1 build/bb 2 10000 5 3        build/cap2.bin > results/enumeration_n2.txt
BB_FULL=1 build/bb 3 10000 14 6       build/cap3.bin > results/enumeration_n3.txt
BB_FULL=1 build/bb 4 10000 70 12      build/cap4.bin > results/enumeration_n4.txt

echo "== 2. tree search vs brute force over every table (n=2: 20,736 tables, n=3: 16,777,216 tables)"
{
build/brute 2 200 | grep '^S=' | sed 's/ count=.*//' > build/brute2.txt
build/brute 3 300 | grep '^S=' | sed 's/ count=.*//' > build/brute3.txt
for n in 2 3; do
  grep '^S=' results/enumeration_n$n.txt | sed 's/ nodes=.*//' > build/tnf$n.txt
  if diff -q build/tnf$n.txt build/brute$n.txt >/dev/null; then echo "n=$n: identical set of achievable S, and identical max Sigma at every S ($(wc -l < build/tnf$n.txt) values of S)"; else echo "n=$n: MISMATCH"; fi
done
} | tee results/validation_vs_bruteforce.txt

echo "== 2b. the parallel work-queue path vs plain recursion"
tools/validate_parallel.sh | tee results/validation_parallel_path.txt

echo "== 3. cheap deciders over the cap leaves (H = 100,000 steps)"
for n in 2 3 4; do build/deciders $n 100000 build/cap$n.bin results/undecided_n$n.txt > results/deciders_n$n.txt; cat results/deciders_n$n.txt; done
gzip -f results/undecided_n4.txt

echo "== 4. soundness: enumerate again with a much larger cap; counts must not change"
build/bb 3 1000000 > results/soundness_n3_cap1e6.txt
build/bb 4 100000  > results/soundness_n4_cap1e5.txt
head -1 results/soundness_n3_cap1e6.txt results/soundness_n4_cap1e5.txt

echo "== 5. champions and the 5-state machine's rounds"
{ build/sim "1RB1LB_1LA---" 1000; build/sim "1RB---_1LB0RC_1LC1LA" 1000; build/sim "1RB1LB_1LA0LC_---1LD_1RD0RA" 1000; build/sim "1RB1LC_1RC1RB_1RD0LE_1LA1LD_---0LA" 100000000; } > results/champions.txt
build/epochs_bb5 > results/bb5_epochs.txt
python3 tools/theory.py | tee results/bb5_theory.txt

echo "== 6. Antihydra: my recalled table vs the one from the web, epochs to ~1e10 steps, then the map"
build/sim "1RB1RA_0LC1LE_1LD0LC_1LA1LB_1LF0LE_---0LA" 1000000 > results/antihydra_recalled_table.txt
build/epochs_antihydra "1RB1RA_0LC1LE_1LD1LC_1LA0LB_1LF1RE_---0RA" 12000000000 > results/antihydra_epochs.txt
python3 tools/antihydra_check.py results/antihydra_epochs.txt | tee results/antihydra_check.txt

echo "== 7. the 6-state champion reported by the search results: does it survive 4e9 steps?"
build/sim "1RB1RA_1RC---_1LD0RF_1RA0LE_0LD1RC_1RA0RE" 4000000000 > results/bb6_champion_run.txt
cat results/bb6_champion_run.txt

echo "== 8. scorecard"
python3 tools/score.py > results/scorecard.md
echo done
