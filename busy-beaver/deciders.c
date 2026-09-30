// Cheap non-halting deciders over the "cap leaves" written by bb.c.
//
//   ./deciders N H capdump.bin [undecided.txt]
//
// For each leaf table, re-simulate from the blank tape for up to H steps.
//   * reaches an undefined entry      -> "late halter" (halts after the enumeration cap)
//   * exact configuration repeats     -> CYCLER (Brent cycle detection on a 64-bit Zobrist hash of
//                                        state + head position + tape; collision chance is ~1e-19/pair)
//   * translated cycler               -> see the soundness argument below
//   * otherwise                       -> UNDECIDED (bouncers, counters, chaotic, slow ...)
//
// Translated-cycler soundness (right-moving case; left is the mirror image).
//   Call a step a "record" if the head arrives at a cell never visited before, to the right of all
//   earlier cells. At a record at time t1 the head is at p1, in state q, and every cell right of p1 is
//   blank. Take a later record t2 (head at p2 = p1 + D, D > 0) in the same state q, and let m be the
//   leftmost head position during [t1, t2], L = p1 - m + 1. If the L cells ending at the head are equal
//   at t1 and t2 (tape[t1][m..p1] == tape[t2][m+D..p2]) then, since all cells right of the head are
//   blank at both times, the whole tape at or right of m+D at t2 equals the tape at or right of m at t1,
//   shifted by D. The run from t1 to t2 only ever looked at cells >= m, so the run from t2 replays it
//   shifted by D, ends in state q at p2 + D with the same L-cell suffix property, and so on forever.
//   It never reaches an undefined entry because the first run did not. We compare suffixes of at most
//   64 cells (one machine word) against the last J = 48 records with the same state.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>

#define NMAX 6
#define J 48

static int N, H;

typedef struct { int t, x, state; uint64_t snap; } Rec;
typedef struct {
    Rec ring[J]; int head, cnt;
    int *stt, *stx; int top;   // monotonic stack of (time, x) with strictly increasing x
} Det;

static inline uint64_t mix64(uint64_t z) {
    z += 0x9e3779b97f4a7c15ULL;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static inline void det_push_pos(Det *d, int t, int x) {
    while (d->top > 0 && d->stx[d->top - 1] >= x) d->top--;
    d->stt[d->top] = t; d->stx[d->top] = x; d->top++;
}
static inline int det_min_since(const Det *d, int t1) {
    int lo = 0, hi = d->top - 1;
    while (lo < hi) { int mid = (lo + hi) >> 1; if (d->stt[mid] >= t1) hi = mid; else lo = mid + 1; }
    return d->stx[lo];
}

// returns 1 if a translated cycler is proven
static inline int det_record(Det *d, int t2, int x2, int state2, uint64_t snap2) {
    int proven = 0;
    for (int k = 0; k < d->cnt && !proven; k++) {
        const Rec *r = &d->ring[(d->head - 1 - k + 2 * J) % J];
        if (r->state != state2) continue;
        int m = det_min_since(d, r->t);
        int L = r->x - m + 1;
        if (L > 64) continue;
        uint64_t mask = L == 64 ? ~0ULL : ((1ULL << L) - 1);
        if (((r->snap ^ snap2) & mask) == 0) proven = 1;
    }
    Rec *w = &d->ring[d->head]; w->t = t2; w->x = x2; w->state = state2; w->snap = snap2;
    d->head = (d->head + 1) % J; if (d->cnt < J) d->cnt++;
    return proven;
}

enum { LATE = 0, CYCLER = 1, TRANSL = 2, UNDEC = 3 };

int main(int argc, char **argv) {
    if (argc < 4) { fprintf(stderr, "usage: deciders N H capdump.bin [undecided.txt]\n"); return 1; }
    N = atoi(argv[1]); H = atoi(argv[2]);
    FILE *f = fopen(argv[3], "rb"); if (!f) { perror("open"); return 1; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    long nleaf = sz / (2 * N);
    uint8_t *data = malloc(sz); if (fread(data, 1, sz, f) != (size_t)sz) return 1; fclose(f);

    int ORG = H + 100, TL = 2 * H + 200;
    uint64_t cnt[4] = {0}, cycler_t_sum = 0, transl_t_sum = 0; long cycler_tmax = 0, transl_tmax = 0;
    uint64_t hist_tdec[8] = {0};   // decided-at-time buckets: <100, <1e3, <1e4, <1e5, ...
    int nthreads = omp_get_max_threads();
    char **undec = calloc(nthreads, sizeof(char *)); size_t *undlen = calloc(nthreads, sizeof(size_t)), *undcap = calloc(nthreads, sizeof(size_t));
    char **late = calloc(nthreads, sizeof(char *)); size_t *latelen = calloc(nthreads, sizeof(size_t)), *latecap = calloc(nthreads, sizeof(size_t));

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        uint8_t *tape = calloc(TL, 1);
        uint64_t *key = malloc(sizeof(uint64_t) * TL);
        for (int i = 0; i < TL; i++) key[i] = mix64((uint64_t)i * 0x2545F4914F6CDD1DULL + 12345);
        Det dr, dl;
        for (Det *d = &dr; d; d = (d == &dr ? &dl : NULL)) { d->stt = malloc(sizeof(int) * (H + 4)); d->stx = malloc(sizeof(int) * (H + 4)); }
        uint64_t lc[4] = {0}, lct[8] = {0}; uint64_t ls_c = 0, ls_t = 0; long lmc = 0, lmt = 0;

        #pragma omp for schedule(dynamic, 256)
        for (long li = 0; li < nleaf; li++) {
            const uint8_t *tab = data + li * 2 * N;
            int pos = ORG, s = 0, lo = pos, hi = pos, t = 0, kind = UNDEC, entry = -1, ones = 0;
            uint64_t th = 0;
            int saved_t = 0, power = 1;   // Brent cycle detection
            uint64_t saved = th ^ mix64((uint64_t)pos * 8 + (uint64_t)s + 0xabcdef);
            dr.top = dl.top = 0; dr.head = dr.cnt = 0; dl.head = dl.cnt = 0;
            det_push_pos(&dr, 0, pos); det_push_pos(&dl, 0, -pos);
            for (t = 0; t < H; t++) {
                int a = tape[pos];
                uint8_t e = tab[2 * s + a];
                if (!e) { kind = LATE; entry = 2 * s + a; break; }
                int w = (e >> 4) & 1;
                if (w != a) { th ^= key[pos]; ones += w - a; }
                tape[pos] = (uint8_t)w;
                pos += (e & 32) ? 1 : -1;
                s = (e & 15) - 1;
                int tt = t + 1;
                // Brent exact-repeat check
                uint64_t ch = th ^ mix64((uint64_t)pos * 8 + (uint64_t)s + 0xabcdef);
                if (ch == saved) { kind = CYCLER; t = tt; break; }
                if (tt - saved_t == power) { saved = ch; saved_t = tt; power <<= 1; }
                det_push_pos(&dr, tt, pos); det_push_pos(&dl, tt, -pos);
                if (pos > hi) {
                    hi = pos;
                    uint64_t snap = 0; for (int i = 0; i < 64; i++) snap |= (uint64_t)tape[pos - i] << i;
                    if (det_record(&dr, tt, pos, s, snap)) { kind = TRANSL; t = tt; break; }
                }
                if (pos < lo) {
                    lo = pos;
                    uint64_t snap = 0; for (int i = 0; i < 64; i++) snap |= (uint64_t)tape[pos + i] << i;
                    if (det_record(&dl, tt, -pos, s, snap)) { kind = TRANSL; t = tt; break; }
                }
            }
            lc[kind]++;
            if (kind == CYCLER) { ls_c += t; if (t > lmc) lmc = t; }
            if (kind == TRANSL) { ls_t += t; if (t > lmt) lmt = t; }
            if (kind == CYCLER || kind == TRANSL) { int b = t < 100 ? 0 : t < 1000 ? 1 : t < 10000 ? 2 : 3; lct[b + (kind == TRANSL ? 4 : 0)]++; }
            if (kind == UNDEC || kind == LATE) {
                char buf[160]; char *p = buf;
                for (int st = 0; st < N; st++) { for (int aa = 0; aa < 2; aa++) { uint8_t ee = tab[2 * st + aa]; if (!ee) { memcpy(p, "---", 3); p += 3; } else { *p++ = '0' + ((ee >> 4) & 1); *p++ = (ee & 32) ? 'R' : 'L'; *p++ = 'A' + ((ee & 15) - 1); } } if (st < N - 1) *p++ = '_'; }
                if (kind == LATE) p += sprintf(p, " late-halt S=%d Sigma=%d", t + 1, ones + ((entry & 1) == 0));
                *p++ = '\n'; *p = 0;
                size_t l = p - buf;
                char ***arr = kind == UNDEC ? &undec : &late; size_t *len = kind == UNDEC ? &undlen[tid] : &latelen[tid]; size_t *capp = kind == UNDEC ? &undcap[tid] : &latecap[tid];
                char **slot = &(*arr)[tid];
                if (*len + l + 1 > *capp) { *capp = (*capp + l + 1) * 2; *slot = realloc(*slot, *capp); }
                memcpy(*slot + *len, buf, l); *len += l;
            }
            memset(tape + lo, 0, hi - lo + 1);
        }
        #pragma omp critical
        {
            for (int k = 0; k < 4; k++) cnt[k] += lc[k];
            for (int k = 0; k < 8; k++) hist_tdec[k] += lct[k];
            cycler_t_sum += ls_c; transl_t_sum += ls_t;
            if (lmc > cycler_tmax) cycler_tmax = lmc;
            if (lmt > transl_tmax) transl_tmax = lmt;
        }
    }
    printf("N=%d H=%d leaves=%ld\n", N, H, nleaf);
    printf("late halters : %llu\n", (unsigned long long)cnt[LATE]);
    printf("cyclers      : %llu  (decided by step: <100:%llu <1e3:%llu <1e4:%llu >=1e4:%llu, latest %ld)\n", (unsigned long long)cnt[CYCLER],
           (unsigned long long)hist_tdec[0], (unsigned long long)hist_tdec[1], (unsigned long long)hist_tdec[2], (unsigned long long)hist_tdec[3], cycler_tmax);
    printf("transl.cycl. : %llu  (decided by step: <100:%llu <1e3:%llu <1e4:%llu >=1e4:%llu, latest %ld)\n", (unsigned long long)cnt[TRANSL],
           (unsigned long long)hist_tdec[4], (unsigned long long)hist_tdec[5], (unsigned long long)hist_tdec[6], (unsigned long long)hist_tdec[7], transl_tmax);
    printf("undecided    : %llu  (%.4f%% of leaves)\n", (unsigned long long)cnt[UNDEC], 100.0 * cnt[UNDEC] / nleaf);
    FILE *uo = argc > 4 ? fopen(argv[4], "w") : NULL;
    for (int i = 0; i < nthreads; i++) { if (uo && undec[i]) fwrite(undec[i], 1, undlen[i], uo); if (late[i]) fwrite(late[i], 1, latelen[i], stdout); }
    if (uo) fclose(uo);
    return 0;
}
