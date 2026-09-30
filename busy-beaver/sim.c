// General simulator for 2-symbol machines in bbchallenge text format.
//
//   ./sim "1RB1LC_1RC1RB_1RD0LE_1LA1LD_---0LA" MAXSTEPS [snapfile nsnap mode]
//
// `---` = undefined = halt. Convention as in PREREGISTRATION.md: the halting transition counts as a
// step and is taken to write a 1.
// With snapfile: runs twice. Pass 1 finds the final visited span [lo,hi]; pass 2 writes nsnap
// snapshots of that window (bytes 0/1) to snapfile, preceded by a header
//   int64 lo, hi, nsnap, then for each snapshot: int64 time, int64 head, int64 state, bytes[hi-lo+1].
// mode 0 = times uniform on [0,S], mode 1 = times geometric (log-spaced) on [1,S].
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define NST 8
static uint8_t tab[2 * NST];
static int nstates;

static int parse(const char *s) {
    memset(tab, 0, sizeof tab);
    int st = 0; const char *p = s;
    while (*p) {
        for (int a = 0; a < 2; a++) {
            if (p[0] == '-') { tab[2 * st + a] = 0; }
            else { int w = p[0] - '0'; int d = p[1] == 'R'; int q = p[2] - 'A'; tab[2 * st + a] = (uint8_t)((q + 1) | (w << 4) | (d << 5)); }
            p += 3;
        }
        st++;
        if (*p == '_') p++;
    }
    return st;
}

typedef struct { int64_t steps, ones, lo, hi; int halted; int64_t pos; int state; } Result;

#define TAPE_BITS 27
#define TAPE_LEN ((int64_t)1 << TAPE_BITS)

static Result run(int64_t maxsteps, uint8_t *tape, int64_t org, int64_t *times, int64_t ntimes, FILE *out, int64_t wlo, int64_t whi) {
    Result r = {0}; int64_t pos = org, lo = org, hi = org, ones = 0, t = 0; int s = 0; int64_t ti = 0;
    for (;;) {
        while (ti < ntimes && times[ti] == t) {
            int64_t hdr[3] = { t, pos - org, s }; fwrite(hdr, 8, 3, out);
            fwrite(tape + org + wlo, 1, (size_t)(whi - wlo + 1), out); ti++;
        }
        if (t >= maxsteps) break;
        int a = tape[pos]; uint8_t e = tab[2 * s + a];
        if (!e) { r.halted = 1; if (a == 0) { tape[pos] = 1; ones++; } t++; break; }
        int w = (e >> 4) & 1; ones += w - a; tape[pos] = (uint8_t)w;
        pos += (e & 32) ? 1 : -1; s = (e & 15) - 1; t++;
        if (pos < lo) lo = pos;
        if (pos > hi) hi = pos;
        if (pos < 1000 || pos > TAPE_LEN - 1000) { fprintf(stderr, "tape bound hit at t=%lld\n", (long long)t); break; }
    }
    while (ti < ntimes && times[ti] <= t) {
        int64_t hdr[3] = { t, pos - org, s }; fwrite(hdr, 8, 3, out);
        fwrite(tape + org + wlo, 1, (size_t)(whi - wlo + 1), out); ti++;
    }
    r.steps = t; r.ones = ones; r.lo = lo - org; r.hi = hi - org; r.pos = pos - org; r.state = s;
    return r;
}

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "usage: sim MACHINE MAXSTEPS [snapfile nsnap mode]\n"); return 1; }
    nstates = parse(argv[1]);
    int64_t maxsteps = atoll(argv[2]);
    uint8_t *tape = calloc(TAPE_LEN, 1); int64_t org = TAPE_LEN / 2;
    Result r = run(maxsteps, tape, org, NULL, 0, NULL, 0, 0);
    printf("machine=%s states=%d\n", argv[1], nstates);
    printf("halted=%d steps=%lld ones=%lld visited_cells=%lld (lo=%lld hi=%lld) head=%lld state=%c\n", r.halted, (long long)r.steps,
           (long long)r.ones, (long long)(r.hi - r.lo + 1), (long long)r.lo, (long long)r.hi, (long long)r.pos, 'A' + r.state);
    if (r.halted) {
        int64_t zeros = 0, ones = 0; for (int64_t i = r.lo; i <= r.hi; i++) { if (tape[org + i]) ones++; else zeros++; }
        printf("final tape within visited span: ones=%lld zeros=%lld\n", (long long)ones, (long long)zeros);
    }
    if (argc >= 6) {
        int64_t ns = atoll(argv[4]); int mode = atoi(argv[5]);
        int64_t *times = malloc(sizeof(int64_t) * ns); int64_t S = r.steps;
        for (int64_t i = 0; i < ns; i++) {
            if (mode == 0) times[i] = (int64_t)((double)S * i / (ns - 1));
            else { double v = pow((double)S, (double)i / (ns - 1)); times[i] = (int64_t)v; if (i > 0 && times[i] <= times[i - 1]) times[i] = times[i - 1] + 1; if (times[i] > S) times[i] = S; }
        }
        // make non-decreasing & unique-ish
        for (int64_t i = 1; i < ns; i++) if (times[i] < times[i - 1]) times[i] = times[i - 1];
        memset(tape, 0, TAPE_LEN);
        FILE *f = fopen(argv[3], "wb");
        int64_t hdr[3] = { r.lo, r.hi, ns }; fwrite(hdr, 8, 3, f);
        run(maxsteps, tape, org, times, ns, f, r.lo, r.hi);
        fclose(f);
        printf("wrote %lld snapshots (mode %d) of %lld cells to %s\n", (long long)ns, mode, (long long)(r.hi - r.lo + 1), argv[3]);
    }
    return 0;
}
