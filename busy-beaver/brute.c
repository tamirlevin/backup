// Independent brute force for validation: every table of n states x 2 symbols, each entry
// (write, dir, next in {states} + HALT). No normal form, no symmetry reduction, no laziness.
// Prints, for each S reached by a halting machine, the max Sigma and how many machines.
//   ./brute N CAP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char **argv) {
    int N = atoi(argv[1]), CAP = atoi(argv[2]);
    int opts = 4 * (N + 1);             // (w, d, q) with q in 0..N (N = HALT)
    int E = 2 * N;
    uint64_t total = 1; for (int i = 0; i < E; i++) total *= opts;
    int len = 2 * CAP + 16, org = CAP + 8;
    uint8_t *tape = calloc(len, 1);
    uint64_t *cnt = calloc(CAP + 2, sizeof(uint64_t));
    int *maxsig = calloc(CAP + 2, sizeof(int));
    uint64_t nhalt = 0;
    for (uint64_t code = 0; code < total; code++) {
        uint64_t c = code; int W[12], D[12], Q[12];
        for (int i = 0; i < E; i++) { int o = c % opts; c /= opts; W[i] = o & 1; D[i] = (o >> 1) & 1; Q[i] = o >> 2; }
        int pos = org, s = 0, ones = 0, lo = pos, hi = pos, t = 0, halted = 0;
        while (t < CAP) {
            int a = tape[pos], i = 2 * s + a;
            ones += W[i] - a; tape[pos] = W[i];
            pos += D[i] ? 1 : -1; t++;
            if (pos < lo) lo = pos;
            if (pos > hi) hi = pos;
            if (Q[i] == N) { halted = 1; break; }
            s = Q[i];
        }
        if (halted) { nhalt++; cnt[t]++; if (ones > maxsig[t]) maxsig[t] = ones; }
        memset(tape + lo, 0, hi - lo + 1);
    }
    printf("N=%d machines=%llu halting=%llu\n", N, (unsigned long long)total, (unsigned long long)nhalt);
    for (int k = CAP; k >= 1; k--) if (cnt[k]) printf("S=%d maxSigma=%d count=%llu\n", k, maxsig[k], (unsigned long long)cnt[k]);
    return 0;
}
