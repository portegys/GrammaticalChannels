#include "chdefs.h"
#include "callp.h"
#include "route.h"
main() {
int i;
char chan[STRSIZE],pr[STRSIZE],msg[STRSIZE];
int chend;
int func,pstat,mtype;

printf("pid=%d\n",getpid());
i = chcreat(ROUTCH);
printf("route chcreat=%d\n",i);
sleep(15);
i = chcreat(ORIGCH);
printf("orig chcreat=%d\n",i);
sleep(15);
i = system("orig&");
printf("orig = %d\n",i);
sleep(15);
chopen(ROUTCH,BEND);
i = chrcv(chan,&chend,&func,pr,&pstat,&mtype,msg);
printf("chrcv=%d,chan=%s,func=%d,pr=%s,pstat=%d,mtype=%d,msg=%s\n",i,chan,func,pr,pstat,mtype,msg);
i = chopen(CALLCH,BEND);
printf("callch chopen=%d\n",i);
chopen(ORIGCH,BEND);
i = chrcv(chan,&chend,&func,pr,&pstat,&mtype,msg);
printf("chrcv=%d,chan=%s,func=%d,pr=%s,pstat=%d,mtype=%d,msg=%s\n",i,chan,func,pr,pstat,mtype,msg);
i = chsend(CALLCH,BEND,USERMSG,CALLP,ANSWER,"answer msg");
printf("chsend=%d\n",i);
i = chsend(CALLCH,BEND,USERMSG,CALLP,HOLD,"hold msg");
printf("chsend =%d\n",i);
i = chrcv(chan,&chend,&func,pr,&pstat,&mtype,msg);
printf("chrcv=%d,chan=%s,func=%d,pr=%s,pstat=%d,mtype=%d,msg=%s\n",i,chan,func,pr,pstat,mtype,msg);
chsend(CALLCH,BEND,USERMSG,CALLP,DISC,"disc msg");
i = chrcv(chan,&chend,&func,pr,&pstat,&mtype,msg);
printf("chrcv=%d,chan=%s,func=%d,pr=%s,pstat=%d,mtype=%d,msg=%s\n",i,chan,func,pr,pstat,mtype,msg);
i = chclose(ROUTCH,BEND);
printf("route chclose=%d\n",i);
sleep(15);
i = chkill(ROUTCH);
printf("route chkill=%d\n",i);
i = chclose(ORIGCH,BEND);
printf("orig chclose=%d\n",i);
sleep(15);
i = chkill(ORIGCH);
printf("orig chkill=%d\n",i);
chclose(CALLCH,BEND);
}
