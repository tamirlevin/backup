"""Collatz-like theory of the 5-state champion, tested against the machine on held-out starting blocks.

usage: python3 tools/theory.py
"""
M="1RB1LC_1RC1RB_1RD0LE_1LA1LD_---0LA"
T={}
for si,part in enumerate(M.split('_')):
    for a in (0,1):
        c=part[3*a:3*a+3]
        T[(si,a)] = None if c=='---' else (int(c[0]), 1 if c[1]=='R' else -1, ord(c[2])-65)
D=3
def from_config(n, limit=10**7):
    """state D, block of n ones at [0,n-1], head at n. Run to next epoch or halt."""
    tape=bytearray(4*n+50); off=n+20   # room to the left; block at off..off+n-1
    for i in range(n): tape[off+i]=1
    pos=off+n; st=D; t=0; hi=pos; ones=n
    while t<limit:
        a=tape[pos]; tr=T[(st,a)]
        if tr is None:
            t+=1
            if a==0: ones+=1
            return ('halt', t, ones)
        w,d,q=tr; ones+=w-a; tape[pos]=w; pos+=d; st=q; t+=1
        if pos>=len(tape)-5: raise Exception("tape too short")
        if pos>hi:
            hi=pos
            if hi==off+n+3 and st==D:   # next epoch
                lo=next(i for i,v in enumerate(tape) if v); hi_one=max(i for i,v in enumerate(tape) if v)
                solid = (hi_one-lo+1==ones) and pos==hi_one+1
                return ('epoch', t, ones, solid)
    return ('limit',t,ones)
def pred(n):
    r=n%3
    if r==0: return ('epoch',(5*n*n+33*n+45)//9,(5*n+12)//3, (5*n*n+33*n+45)%9==0)
    if r==1: return ('epoch',(5*n*n+41*n+71)//9,(5*n+16)//3, (5*n*n+41*n+71)%9==0)
    return ('halt',n+2,(n-2)//3+3, True)
bad=0; tot=0; res={0:[0,0],1:[0,0],2:[0,0]}
for n in range(1,251):
    got=from_config(n); p=pred(n); tot+=1
    ok = (got[0]==p[0] and got[1]==p[1] and got[2]==p[2] and (got[0]=='halt' or got[3]))
    res[n%3][0]+=1; res[n%3][1]+=ok
    if not ok:
        bad+=1
        if bad<=10: print("MISMATCH n=%d got=%s predicted=%s"%(n,got,p))
print(f"configs tested: {tot}, mismatches: {bad}")
desc={0:"next block (5n+12)/3, steps (5n^2+33n+45)/9",1:"next block (5n+16)/3, steps (5n^2+41n+71)/9",2:"HALT after n+2 steps, (n-2)/3+3 ones"}
for r in (0,1,2): print("  n mod 3 = %d: %d/%d match   [%s]"%(r,res[r][1],res[r][0],desc[r]))
# now the whole champion from the theory alone
n=3; S=3; ones=None; k=1; seq=[]
while True:
    seq.append(n); p=pred(n)
    if p[0]=='halt':
        S+=p[1]; final_ones=p[2]; break
    S+=p[1]; n=p[2]; k+=1
print("theory-only reconstruction of the champion: orbit", seq)
print("  total steps S =",S," final ones =",final_ones, " -> matches simulation (47176870, 4098):", (S,final_ones)==(47176870,4098))
