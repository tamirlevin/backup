#!/usr/bin/env bash
# The n=4 enumeration splits the tree into parallel work items; n=2 and n=3 were validated against brute force with
# plain recursion. This checks the parallel path against plain recursion: n=2,3 at several split depths, and n=4
# serial vs the parallel default. Needs build/bb and results/enumeration_n4.txt (run_all.sh makes both).
set -euo pipefail
cd "$(dirname "$0")/.."
for n in 2 3; do
  for d in 1000 2 3 4; do printf "n=%s BB_QDEPTH=%-4s " $n $d; BB_QDEPTH=$d build/bb $n 10000 | head -1; done
done
BB_QDEPTH=1000 BB_FULL=1 build/bb 4 10000 > build/n4_serial.txt
if diff -q <(grep '^S=' build/n4_serial.txt) <(grep '^S=' results/enumeration_n4.txt) >/dev/null; then
  echo "n=4: serial run and parallel run give identical S:maxSigma:nodes tables ($(head -1 build/n4_serial.txt | cut -d' ' -f3-5))"
else echo "n=4: MISMATCH"; fi
