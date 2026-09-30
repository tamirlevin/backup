// Lazy tree-normal-form enumeration of n-state, 2-symbol Turing machines.
// Definitions are frozen in PREREGISTRATION.md.
//
//   ./bb N CAP [thrS thrSigma [capdump.bin]]
//
// Prints N_sim / N_halt / N_cap, the S histogram tail, and every halting node
// with S >= thrS or Sigma >= thrSigma (bbchallenge text format).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>

#define NMAX 6
typedef struct { uint8_t e[2 * NMAX]; } Table;  // entry: 0 undefined; else (q+1) | w<<4 | dir<<5 (dir 1 = R)

static int N, CAP, THR_S, THR_SIG;
static int TAPE_LEN, ORIGIN;

typedef struct {
    uint64_t nsim, nhalt, ncap;
    uint64_t *histS;       // histS[S] = number of halting nodes with that S
    uint64_t *histSig;     // histSig[sigma]
    int *maxSigAtS;        // max Sigma among halting nodes with this S
    uint8_t *tape;
    Table *caps; size_t ncaps, capcap;
    char *out; size_t outlen, outcap;
    int dump;
} Ctx;

static inline uint8_t enc(int q, int w, int d) { return (uint8_t)((q + 1) | (w << 4) | (d << 5)); }

static void table_str(const Table *t, char *buf) {
    char *p = buf;
    for (int s = 0; s < N; s++) {
        for (int a = 0; a < 2; a++) {
            uint8_t e = t->e[2 * s + a];
            if (!e) { memcpy(p, "---", 3); p += 3; }
            else { *p++ = '0' + ((e >> 4) & 1); *p++ = (e & 32) ? 'R' : 'L'; *p++ = 'A' + ((e & 15) - 1); }
        }
        if (s < N - 1) *p++ = '_';
    }
    *p = 0;
}

// Returns 1 if an undefined entry was reached (halting node), 0 if CAP steps survived.
static inline int simulate(const Table *tb, uint8_t *tape, int *t_out, int *ones_out, int *entry_out) {
    int pos = ORIGIN, s = 0, ones = 0, lo = pos, hi = pos, t = 0, res = 0;
    const uint8_t *tab = tb->e;
    for (; t < CAP; t++) {
        int a = tape[pos];
        uint8_t e = tab[2 * s + a];
        if (!e) { *entry_out = 2 * s + a; res = 1; break; }
        int w = (e >> 4) & 1;
        ones += w - a;
        tape[pos] = (uint8_t)w;
        pos += (e & 32) ? 1 : -1;
        s = (e & 15) - 1;
        if (pos < lo) lo = pos;
        if (pos > hi) hi = pos;
    }
    memset(tape + lo, 0, (size_t)(hi - lo + 1));
    *t_out = t; *ones_out = ones;
    return res;
}

static void emit(Ctx *cx, const char *line) {
    size_t l = strlen(line);
    if (cx->outlen + l + 1 > cx->outcap) { cx->outcap = (cx->outcap + l + 1) * 2; cx->out = realloc(cx->out, cx->outcap); }
    memcpy(cx->out + cx->outlen, line, l); cx->outlen += l; cx->out[cx->outlen++] = '\n';
}

static void explore(Table *tb, int ndef, int m, int isfirst, Ctx *cx, int qdepth, Table *queue, int *qn) {
    if (queue && ndef == qdepth) { if (*qn >= 400000) { fprintf(stderr, "work queue full\n"); exit(1); } queue[(*qn)++] = *tb; return; }
    int t, ones, entry;
    int res = simulate(tb, cx->tape, &t, &ones, &entry);
    cx->nsim++;
    if (!res) {
        cx->ncap++;
        if (cx->dump) {
            if (cx->ncaps == cx->capcap) { cx->capcap = cx->capcap ? cx->capcap * 2 : 4096; cx->caps = realloc(cx->caps, cx->capcap * sizeof(Table)); }
            cx->caps[cx->ncaps++] = *tb;
        }
        return;
    }
    cx->nhalt++;
    int a = entry & 1;
    int S = t + 1, sigma = ones + (a == 0);
    cx->histS[S]++; cx->histSig[sigma]++;
    if (sigma > cx->maxSigAtS[S]) cx->maxSigAtS[S] = sigma;
    if (S >= THR_S || sigma >= THR_SIG) {
        char buf[128], line[200]; table_str(tb, buf);
        snprintf(line, sizeof line, "%s S=%d Sigma=%d", buf, S, sigma);
        emit(cx, line);
    }
    if (ndef == 2 * N - 1) return;  // last undefined entry: any non-halting choice can never halt
    int maxq = m < N ? m : N - 1;
    for (int w = 0; w < 2; w++)
        for (int d = isfirst ? 1 : 0; d < 2; d++)
            for (int q = 0; q <= maxq; q++) {
                tb->e[entry] = enc(q, w, d);
                explore(tb, ndef + 1, q == m ? m + 1 : m, 0, cx, qdepth, queue, qn);
            }
    tb->e[entry] = 0;
}

static Ctx *new_ctx(int dump) {
    Ctx *c = calloc(1, sizeof(Ctx));
    c->histS = calloc(CAP + 3, sizeof(uint64_t));
    c->histSig = calloc(CAP + 3, sizeof(uint64_t));
    c->maxSigAtS = calloc(CAP + 3, sizeof(int));
    c->tape = calloc(TAPE_LEN, 1);
    c->dump = dump;
    return c;
}

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "usage: bb N CAP [thrS thrSigma [capdump.bin]]\n"); return 1; }
    N = atoi(argv[1]); CAP = atoi(argv[2]);
    THR_S = argc > 3 ? atoi(argv[3]) : 1 << 30;
    THR_SIG = argc > 4 ? atoi(argv[4]) : 1 << 30;
    const char *dumpfile = argc > 5 ? argv[5] : NULL;
    TAPE_LEN = 2 * CAP + 16; ORIGIN = CAP + 8;

    // phase 1: expand the top of the tree serially into a work queue
    Ctx *root = new_ctx(dumpfile != NULL);
    Table start; memset(&start, 0, sizeof start);
    // BB_QDEPTH=d splits the tree into parallel work items at depth d (default 4 for N >= 4; 1000 = plain recursion)
    int qdepth = N >= 4 ? 4 : 1000;
    if (getenv("BB_QDEPTH")) qdepth = atoi(getenv("BB_QDEPTH"));
    Table *queue = NULL; int qn = 0;
    if (qdepth < 1000) queue = malloc(sizeof(Table) * 400000);
    // queue items remember m: recompute from table (max target + 1)
    explore(&start, 0, 1, 1, root, qdepth, queue, &qn);

    int nthreads = omp_get_max_threads();
    Ctx **ctxs = malloc(sizeof(Ctx *) * nthreads);
    for (int i = 0; i < nthreads; i++) ctxs[i] = new_ctx(dumpfile != NULL);
    if (qn) {
        #pragma omp parallel for schedule(dynamic, 1)
        for (int i = 0; i < qn; i++) {
            Ctx *cx = ctxs[omp_get_thread_num()];
            Table tb = queue[i];
            int ndef = 0, mx = 0;
            for (int k = 0; k < 2 * N; k++) if (tb.e[k]) { ndef++; int q = (tb.e[k] & 15); if (q > mx) mx = q; }
            explore(&tb, ndef, mx ? mx : 1, 0, cx, -1, NULL, NULL);
        }
    }

    Ctx *tot = new_ctx(0);
    Ctx *all[64]; int na = 0; all[na++] = root; for (int i = 0; i < nthreads; i++) all[na++] = ctxs[i];
    FILE *df = dumpfile ? fopen(dumpfile, "wb") : NULL;
    for (int i = 0; i < na; i++) {
        Ctx *c = all[i];
        tot->nsim += c->nsim; tot->nhalt += c->nhalt; tot->ncap += c->ncap;
        for (int k = 0; k < CAP + 3; k++) { tot->histS[k] += c->histS[k]; tot->histSig[k] += c->histSig[k]; if (c->maxSigAtS[k] > tot->maxSigAtS[k]) tot->maxSigAtS[k] = c->maxSigAtS[k]; }
        if (c->out) fwrite(c->out, 1, c->outlen, stdout);
        if (df) for (size_t k = 0; k < c->ncaps; k++) fwrite(c->caps[k].e, 1, 2 * N, df);
    }
    if (df) fclose(df);
    printf("N=%d CAP=%d N_sim=%llu N_halt=%llu N_cap=%llu\n", N, CAP,
           (unsigned long long)tot->nsim, (unsigned long long)tot->nhalt, (unsigned long long)tot->ncap);
    printf("S histogram tail (largest 12 distinct S: S:count):");
    int shown = 0; for (int k = CAP + 2; k >= 0 && shown < 12; k--) if (tot->histS[k]) { printf(" %d:%llu", k, (unsigned long long)tot->histS[k]); shown++; }
    printf("\nSigma histogram tail (largest 8 distinct: Sigma:count):");
    shown = 0; for (int k = CAP + 2; k >= 0 && shown < 8; k--) if (tot->histSig[k]) { printf(" %d:%llu", k, (unsigned long long)tot->histSig[k]); shown++; }
    printf("\n");
    if (getenv("BB_FULL")) for (int k = CAP + 2; k >= 1; k--) if (tot->histS[k]) printf("S=%d maxSigma=%d nodes=%llu\n", k, tot->maxSigAtS[k], (unsigned long long)tot->histS[k]);
    return 0;
}
