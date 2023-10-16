#include "chdefs.h"

/*
 * chrcv(channel-name, channel-end, channel-function, protocol-name, 
 *       protocol-status, user-message-type, user-message-text) -
 * Receive a grammatical channel message from an open channel;
 * suspend the process until one arrives.  Even if more than one 
 * channel has a message ready, only one message is returned.
 * No promises are made about which one.
 *
 * Output: channel-name - the name of the channel from which the 
 *           message was received
 *	   channel-end - the end of the channel from which the message came
 *         channel-function - the function performed by the message sent
 *                            sent from the other end:
 *                            STARTP: start a protocol
 *                            ENDP: end a protocol
 *                            CLEARP: clear request (ends protocol)
 *                            USERMSG: a user message
 *         protocol-name - protocol name in effect
 *         protocol-status - the status of the protocol:
 *                           IDLE, ACTIVE, or ERROR
 *         user-message-type - user message type (set by user process)
 *         user-message-text - user message text
 *
 *         Returns: SUCCESS - message received successfully
 *                  NOCHANNEL - an open channel does not exist
 *                  NOTOPEN - no channel open
 *                  FAIL - something went wrong
 *
 */

int intflg;	/* signal catch flag */

chrcv(chname,chend,chfunc,prname,prstat,usrmtype,usrmtext)
char *chname;
int *chend;
int *chfunc;
char *prname;
int *prstat;
int *usrmtype;
char *usrmtext; {

int aqid,bqid,cqid,pqid;
int cpid,pid,asem,bsem,psem,semflg;
int msgrcv(),msgsnd(),msgget(),msgctl();
int semop(),semget(),semctl();
int msgflg,msgtyp,msgsz,retval;
int child[CHOPLIMIT],cptr,i;
key_t key;
struct chmsgbuf chmsg;
struct sembuf sops;
char buf[BUFSIZE],chopen[STRSIZE];
FILE *fp1,*fp2,*fopen();
int sigusr1();
int cend;

	/* check if any channel open */
	sprintf(buf,"ls /usr/tmp/_CO.%d >/dev/null 2>/dev/null",getpid());
	if (system(buf) != 0) {
		return(NOTOPEN);
	}

	/* create parent semaphore and message queue */
	key = IPC_PRIVATE;
	msgflg = PMODE;
	pqid = msgget(key,msgflg);
	if (pqid == -1) {
		return(FAIL);
	}
	key = IPC_PRIVATE;
	semflg = PMODE;
	psem = semget(key,1,semflg);
	if (psem == -1) {
		return(FAIL);
	}
	sops.sem_num = 0;
	sops.sem_flg = 0;
	sops.sem_op = 1;
	semop(psem,&sops,1);

	/* check each open channel for a message */
	sprintf(buf,"/usr/tmp/_CO.%d",getpid());
	if ((fp1 = fopen(buf,"r")) == NULL) {
		return(FAIL);
	}
	cptr = 0;
	while ((fscanf(fp1,"%s %d",chopen,&cend)) != EOF) {

		/* get channel data */
		sprintf(buf,"/usr/tmp/_CH.%s",chopen);
		if ((fp2 = fopen(buf,"r")) == NULL) {

			/* no channel - kill children and bomb out */
			for (i = 0; i < cptr; i++) {
				kill(child[i],SIGUSR1);
			}
			msgctl(pqid,IPC_RMID);
			semctl(psem,0,IPC_RMID,0L);
			return(NOCHANNEL);
		}
		fscanf(fp2,"%d %d %d %d %d %d",&aqid,&bqid,&cqid,&asem,&bsem,&pid);
		fclose(fp2);

		/* fork child to get message for parent */
		cpid = fork();
		if (cpid == 0) {

			/* catch parent kill signal */
			intflg = FALSE;
			signal(SIGUSR1,sigusr1);

			/* seize the channel end */
			sops.sem_num = 0;
			sops.sem_flg = 0;
			sops.sem_op = -1;
			if (cend == AEND) {
				if (intflg == TRUE) { exit(0); }
				retval = semop(asem,&sops,1);
			} else {
				if (intflg == TRUE) { exit(0); }
				retval = semop(bsem,&sops,1);
			}
			if (retval == -1) {	/* assume got signal */
				exit(0);
			}
			strcpy(chmsg.chname,chopen);
			chmsg.chend = cend;

			/* first check front porch for message */
			if (cend == AEND) {
				sprintf(buf,"/usr/tmp/_FPA.%s",chopen);
			} else {
				sprintf(buf,"/usr/tmp/_FPB.%s",chopen);
			}
			if ((fp2 = fopen(buf,"r")) != NULL) {
				fscanf(fp2,"%d",&chmsg.chfunc);
				fscanf(fp2,"%s",chmsg.prname);
				fscanf(fp2,"%d",&chmsg.prstat);
				fscanf(fp2,"%d",&chmsg.usrmtype);
				fscanf(fp2,"%s",chmsg.usrmtext);
				fclose(fp2);
				if (cend == AEND) {
					sprintf(buf,"rm /usr/tmp/_FPA.%s 2>/dev/null",chopen);
				} else {
					sprintf(buf,"rm /usr/tmp/_FPB.%s 2>/dev/null",chopen);
				}
				system(buf);

				/* seize the parent semaphore */
				sops.sem_num = 0;
				sops.sem_flg = 0;
				sops.sem_flg |= IPC_NOWAIT;
				sops.sem_op = -1;
				retval = semop(psem,&sops,1);

			} else {	/* no front porch message */

				/* read message from queue */
				msgflg = 0;
				msgtyp = 0;
				msgsz = CHMSGSZ;
				if (intflg == TRUE) {
					retval = -1;
				} else {
					if (cend == AEND) {
						retval = msgrcv(aqid,&chmsg,msgsz,msgtyp,msgflg);
					} else {
						retval = msgrcv(bqid,&chmsg,msgsz,msgtyp,msgflg);
					}
				}
				if (retval == -1) {	/* assume got signal */
					sops.sem_num = 0;	/* release channel */
					sops.sem_flg = 0;
					sops.sem_op = 1;
					if (cend == AEND) {
						semop(asem,&sops,1);
					} else {
						semop(bsem,&sops,1);
					}
					exit(0);
				} else {

					/* get parent semaphore */
					sops.sem_num = 0;
					sops.sem_flg = 0;
					sops.sem_flg |= IPC_NOWAIT;
					sops.sem_op = -1;
					retval = semop(psem,&sops,1);
				}
			}

			/* got parent semaphore? */
			if (retval == -1) {

				/* 
				 * did not get parent semaphore -
				 * assume other child got it first
				 */

				/* leave message on front porch */
				if (cend == AEND) {
					sprintf(buf,"/usr/tmp/_FPA.%s",chopen);
				} else {
					sprintf(buf,"/usr/tmp/_FPB.%s",chopen);
				}
				fp2 = fopen(buf,"w");
				fprintf(fp2,"%d\n",chmsg.chfunc);
				fprintf(fp2,"%s\n",chmsg.prname);
				fprintf(fp2,"%d\n",chmsg.prstat);
				fprintf(fp2,"%d\n",chmsg.usrmtype);
				fprintf(fp2,"%s\n",chmsg.usrmtext);
				fclose(fp2);

				/* release channel */
				sops.sem_num = 0;
				sops.sem_flg = 0;
				sops.sem_op = 1;
				if (cend == AEND) {
					retval = semop(asem,&sops,1);
				} else {
					retval = semop(bsem,&sops,1);
				}
				exit(0);

			} else { 	/* got parent semaphore */

				/* send message to parent */
				msgflg = 0;
				msgsz = CHMSGSZ;
				msgsnd(pqid,&chmsg,msgsz,msgflg);

				/* release channel */
				sops.sem_num = 0;
				sops.sem_flg = 0;
				sops.sem_op = 1;
				if (cend == AEND) {
					retval = semop(asem,&sops,1);
				} else {
					retval = semop(bsem,&sops,1);
				}

				/* pause for signal from parent if needed */
				if (intflg == FALSE) {
					pause();
				}
				exit(0);
			}

		} else {	/* not child */

			/* save child pid */
			child[cptr] = cpid;
			cptr++;
		}
	}
	fclose(fp1);

	/* get the first ready message from a child */
	msgflg = 0;
	msgtyp = 0;
	msgsz = CHMSGSZ;
	retval = msgrcv(pqid,&chmsg,msgsz,msgtyp,msgflg);
	if (retval == -1) {
		return(FAIL);
	}

	/* kill any waiting children */
	for (i = 0; i < cptr; i++) {
		kill(child[i],SIGUSR1);
	}
	while (wait(0L) != -1) {}

	/* remove parent semaphore and message queue */
	msgctl(pqid,IPC_RMID);
	semctl(psem,0,IPC_RMID,0L);

	/* copy the message data */
	strcpy(chname,chmsg.chname);
	*chend = chmsg.chend;
	*chfunc = chmsg.chfunc;
	prname[0] = '\0';
	strcpy(prname,chmsg.prname);
	*prstat = chmsg.prstat;
	*usrmtype = chmsg.usrmtype;
	usrmtext[0] = '\0';
	strcpy(usrmtext,chmsg.usrmtext);
	return(SUCCESS);
}

sigusr1() { intflg = TRUE; signal(SIGUSR1,sigusr1); }
