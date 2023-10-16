#include "chdefs.h"

/*
 * Chcreat(channel-name) - create a grammatical channel having the
 * name channel-name.  
 *
 * Returns: SUCCESS - channel created successfully
 *          FAIL - failure to create channel
 *          DUPCHANNEL - channel already exists
 *
 */

chcreat(chname)
char *chname; {

char buf[BUFSIZE];
int msgget(),semget(),semop();
key_t key;
FILE *fp,*fopen();
struct sembuf sops;
int asem,bsem,semflg;
int aqid,bqid,cqid,msgflg;

	/* check if the channel already exists */
	sprintf(buf,"/usr/tmp/_CH.%s",chname);
	if ((fp = fopen(buf,"r")) != NULL) {
		fclose(fp);
		return(DUPCHANNEL);
	}

	/* create the message queues */
	key = IPC_PRIVATE;
	msgflg = PMODE;
	aqid = msgget(key,msgflg);
	if (aqid == -1) { 
		return(FAIL);
	}
	key = IPC_PRIVATE;
	msgflg = PMODE;
	bqid = msgget(key,msgflg);
	if (bqid == -1) { 
		return(FAIL);
	}
	key = IPC_PRIVATE;
	msgflg = PMODE;
	cqid = msgget(key,msgflg);
	if (cqid == -1) { 
		return(FAIL);
	}

	/* get semaphores for the channel ends */
	key = IPC_PRIVATE;
	semflg = PMODE;
	asem = semget(key,1,semflg);
	if (asem == -1) { 
		return(FAIL);
	}
	sops.sem_num = 0;
	sops.sem_flg = 0;
	sops.sem_op = 1;
	semop(asem,&sops,1);
	key = IPC_PRIVATE;
	semflg = PMODE;
	bsem = semget(key,1,semflg);
	if (bsem == -1) { 
		return(FAIL);
	}
	sops.sem_num = 0;
	sops.sem_flg = 0;
	sops.sem_op = 1;
	semop(bsem,&sops,1);

	/* create the channel data file */
	sprintf(buf,"echo %d %d %d %d %d X>/usr/tmp/_CH.%s",aqid,bqid,cqid,asem,bsem,chname);
	system(buf);

	/* start up the channel demon initialization process */
	sprintf(buf,"chinit %s %d %d %d %d %d&",chname,aqid,bqid,cqid,asem,bsem);
	if (system(buf) == 0) {
		return(SUCCESS);
	} else {
		return(FAIL);
	}
}
