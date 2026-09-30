#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
// record times of new right-most head positions for the recalled BB(5) champion, plus the number of ones then.
int main(){
    const char *M="1RB1LC_1RC1RB_1RD0LE_1LA1LD_---0LA";
    uint8_t tab[10]; int st=0; const char*p=M;
    while(*p){ for(int a=0;a<2;a++){ if(p[0]=='-') tab[2*st+a]=0; else tab[2*st+a]=(p[2]-'A'+1)|((p[0]-'0')<<4)|((p[1]=='R')<<5); p+=3;} st++; if(*p=='_')p++; }
    int64_t L=1<<26; uint8_t*tape=calloc(L,1); int64_t pos=L/2,org=pos,hi=pos,lo=pos,ones=0,t=0; int s=0;
    for(;;){
        int a=tape[pos]; uint8_t e=tab[2*s+a]; if(!e){t++;break;}
        int w=(e>>4)&1; ones+=w-a; tape[pos]=w; pos+=(e&32)?1:-1; s=(e&15)-1; t++;
        if(pos>hi){hi=pos; printf("hi=%lld t=%lld ones=%lld lo=%lld state=%c\n",(long long)(hi-org),(long long)t,(long long)ones,(long long)(lo-org),'A'+s);}
        if(pos<lo)lo=pos;
    }
    printf("END t=%lld\n",(long long)t);
}
