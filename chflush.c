#include "chdefs.h"
extern int errno;

/*
 * chflush(channel-name,channel-end) - flush messages out of a given
 * end of a given channel which are waiting to be received at that
 * end.
 * 
 * Returns: SUCCESS - messages flushed
 *	    NOTOPEN - channel not open at channel-end
 *	    NOCHANNEL - the channel does not exist
 *          FAIL - something went wrong
 * 
 */

chflush(chname,chend)
char *chname;
int chend; {

int aqid,bqid,cqid,pid,asem,bsem;
int msgrcv(),semop();
int msgflg,msgtyp,msgsz,retval;
int semflg;
struct sembuf sops;
struct chmsgbuf chbuf;
char buf[BUFSIZE];
FILE *fp,*fopen();


	/* check if the channel is open */
	sprintf(buf,"grep \'%s %d\' /usr/tmp/_CO.%d >/dev/null 2>/dev/null",chname,chend,getpid());
	if (system(buf) != 0) {
		return(NOTOPEN);
	}
	/* get the channel data */
	sprintf(buf,"/usr/tmp/_CH.%s",chname);
	if ((fp = fopen(buf,"r")) == NULL) {
		return(NOCHANNEL);
	}
	fscanf(fp,"%d %d %d %d %d %d",&aqid,&bqid,&cqid,&asem,&bsem,&pid);
	fclose(fp);

	/* seize the channel end */
	sops.sem_num = 0;
	sops.sem_flg = 0;
	sops.sem_op = -1;
	if (chend == AEND) {
		retval = semop(asem,&sops,1);
	} else {
		retval = semop(bsem,&sops,1);
	}
	if (retval == -1) { 
		return(FAIL);
	}

	/* loop to flush all the messages out */
	retval = 0;
	while (retval != -1) {
		msgflg = 0;
		msgflg |= IPC_NOWAIT;
		msgtyp = 0;
		msgsz = CHMSGSZ;
		if (chend == AEND) {
			retval = msgrcv(aqid,&chbuf,msgsz,msgtyp,msgflg);
		} else {
			retval = msgrcv(bqid,&chbuf,msgsz,msgtyp,msgflg);
		}
	}
	if (errno != ENOMSG) {
		return(FAIL);
	}

	/* release the channel end */
	sops.sem_num = 0;
	sops.sem_flg = 0;
	sops.sem_op = 1;
	if (chend == AEND) {
		retval = semop(asem,&sops,1);
	} else {
		retval = semop(bsem,&sops,1);
	}
	if (retval == -1) { 
		return(FAIL);
	}

	return(SUCCESS);
}
