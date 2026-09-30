// Antihydra epoch check: at each new right-record head position in state A, parse tape as 1^p 0 1^q 0 1 (RLE) and print p,q,t.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
int main(int argc,char**argv){
    const char *M=argv[1]; int64_t maxsteps=atoll(argv[2]);
    uint8_t tab[16]; int st=0; const char*p=M;
    while(*p){ for(int a=0;a<2;a++){ if(p[0]=='-') tab[2*st+a]=0; else tab[2*st+a]=(p[2]-'A'+1)|((p[0]-'0')<<4)|((p[1]=='R')<<5); p+=3;} st++; if(*p=='_')p++; }
    int64_t L=1<<24; uint8_t*tape=calloc(L,1); int64_t pos=L/2,org=pos,hi=pos,lo=pos,t=0; int s=0;
    for(;t<maxsteps;){
        int a=tape[pos]; uint8_t e=tab[2*s+a]; if(!e){printf("HALT at t=%lld\n",(long long)(t+1));return 0;}
        tape[pos]=(e>>4)&1; pos+=(e&32)?1:-1; s=(e&15)-1; t++;
        if(pos>hi){hi=pos;
            if(s==0){ // parse RLE of tape[lo..hi]
                int64_t runs[64]; char sym[64]; int nr=0; int64_t i=lo;
                while(i<=hi && nr<64){ int64_t j=i; while(j<=hi&&tape[j]==tape[i]) j++; runs[nr]=j-i; sym[nr]=tape[i]; nr++; i=j; }
                // expect: 1^p 0 1^q 0 1
                if(nr==5 && sym[0]==1&&sym[1]==0&&runs[1]==1&&sym[2]==1&&sym[3]==0&&runs[3]==1&&sym[4]==1&&runs[4]==1)
                    printf("t=%lld p=%lld q=%lld\n",(long long)t,(long long)runs[0],(long long)runs[2]);
                else { printf("t=%lld other shape nr=%d:",(long long)t,nr); for(int k=0;k<nr&&k<8;k++)printf(" %d^%lld",sym[k],(long long)runs[k]); printf("\n"); }
                fflush(stdout);
            }
        }
        if(pos<lo)lo=pos;
        if(pos<100||pos>L-100){printf("tape bound\n");return 1;}
    }
    printf("stopped at t=%lld\n",(long long)t);
}
