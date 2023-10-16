#include "chdefs.h"

/*
 * chkill(channel-name) - kill a grammatical channel named
 * channel-name.
 *
 * Returns: SUCCESS - channel successfully killed.
 *          FAIL - an error occurred.
 *
 */

chkill(chname)
char *chname; {

char buf[BUFSIZE];
FILE *fp,*fopen();
int msgctl(),semctl();
int aqid,bqid,cqid,asem,bsem,msgflg,pid;


	/* get the channel data */
	sprintf(buf,"/usr/tmp/_CH.%s",chname);
	if ((fp = fopen(buf,"r")) == NULL) {
		return(SUCCESS);	/* channel does not exists */
	}
	fscanf(fp,"%d %d %d %d %d %d",&aqid,&bqid,&cqid,&asem,&bsem,&pid);
	fclose(fp);

	/* remove the channel data file and front porch */
	sprintf(buf,"rm /usr/tmp/_CH.%s 2>/dev/null",chname);
	system(buf);
	sprintf(buf,"rm /usr/tmp/_FP.%s 2>/dev/null",chname);
	system(buf);

	/* kill the channel demon */
	sprintf(buf,"kill -9 %d 2>/dev/null",pid);
	system(buf);

	/* remove the message queues */
	msgctl(aqid,IPC_RMID);
	msgctl(bqid,IPC_RMID);
	msgctl(cqid,IPC_RMID);

	/* remove the semaphores */
	semctl(asem,0,IPC_RMID,0L);
	semctl(bsem,0,IPC_RMID,0L);
	return(SUCCESS);
}
