import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from png import write_png
M="1RB1LC_1RC1RB_1RD0LE_1LA1LD_---0LA"
T={}
for si,part in enumerate(M.split('_')):
    for a in (0,1):
        c=part[3*a:3*a+3]
        T[(si,a)] = None if c=='---' else (int(c[0]), 1 if c[1]=='R' else -1, ord(c[2])-65)

def trace(t_end):
    """returns lists: tape snapshots (dict-free: bytearray window) per time, head pos, state"""
    L=-400; R=60; W=R-L+1
    tape=bytearray(W); pos=0; st=0
    snaps=[]; heads=[]; states=[]
    for t in range(t_end+1):
        snaps.append(bytes(tape)); heads.append(pos); states.append(st)
        a=tape[pos-L]; tr=T[(st,a)]
        w,d,q=tr; tape[pos-L]=w; pos+=d; st=q
    return snaps,heads,states,L

def render(t0,t1,cellh,colw,path,ones=(43,58,92),zeros=(250,250,252),
           colors=((228,87,46),(242,169,0),(42,157,143),(106,76,147),(29,126,203))):
    snaps,heads,states,L=trace(t1)
    # visible span of cells over [t0,t1]
    lo=min(min(i for i,v in enumerate(s) if v) for s in snaps[t0:t1+1] if any(s))+L
    lo=min(lo,min(heads[t0:t1+1])); hi=max(heads[t0:t1+1]); 
    hi=max(hi, max(max(i for i,v in enumerate(s) if v) for s in snaps[t0:t1+1] if any(s))+L)
    ncell=hi-lo+1; ntime=t1-t0+1
    Wp=ntime*colw; Hp=ncell*cellh
    rows=[bytearray(3*Wp) for _ in range(Hp)]
    for ti in range(ntime):
        t=t0+ti; s=snaps[t]
        for c in range(ncell):
            pos=lo+c
            if pos==heads[t]: col=colors[states[t]]
            else: col = ones if s[pos-L] else zeros
            for dx in range(colw):
                x=(ti*colw+dx)*3
                for dy in range(cellh):
                    r=rows[c*cellh+dy]; r[x]=col[0]; r[x+1]=col[1]; r[x+2]=col[2]
    write_png(path,Wp,Hp,[bytes(r) for r in rows],'RGB')
    return Wp,Hp,lo,hi
if __name__=='__main__':
    here=os.path.dirname(os.path.abspath(__file__))
    print(render(402,1339,6,2,os.path.join(here,'..','figures','bb5_epoch_37_to_67.png')))
