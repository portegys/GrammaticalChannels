#include "chdefs.h"

/*
 * Chdemon - grammatical channel demon.
 *
 * This program passes messages through the a channel in a passive way,
 * watching for a start protocol message.  When it sees one, it starts
 * up the requested protocol checking program.  The channel it is attached
 * to is given in the argc/argv format.
 *
 */

int aqid,bqid,cqid,asem,bsem;
struct chmsgbuf chbuf;
char chname[STRSIZE],prname[STRSIZE];
int pid;

main(argc,argv)
int argc;
char *argv[]; {

FILE *fp,*fopen();
char buf[BUFSIZE];
int sigproc();

	/* catch signals */
	signal(SIGHUP,sigproc);
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,sigproc);
	signal(SIGTERM,sigproc);

	/* find the channel data */
	strcpy(chname,*++argv);
	sprintf(buf,"/usr/tmp/_CH.%s",chname);
	if ((fp = fopen(buf,"r")) == NULL) {
		fprintf(stderr,"chdemon: cannot open channel file %s\n",buf);
		exit(1);
	}

	fscanf(fp,"%d %d %d %d %d %d",&aqid,&bqid,&cqid,&asem,&bsem,&pid);
	fclose(fp);

	/* process channel messages */
	msgproc();

}

sigproc() {	/* process signals */
	chdeath("signal received - channel terminated");
}

msgproc() {	/* process channel messages */

char buf[BUFSIZE],cname[STRSIZE];

	strcpy(cname,chname);
	while (1) {

		getmsg();
		chbuf.prstat = IDLE;
		switch(chbuf.chfunc) {

			/* start protocol request */
			case STARTP:	sprintf(buf,"where %s >/dev/null",chbuf.prname);
					if (system(buf) == 0) {
						chbuf.prstat = ACTIVE;
						putmsg(&chbuf);
						execlp(chbuf.prname,chbuf.prname,cname,0);
					} else {
						chbuf.prstat = ERROR;
						putmsg(&chbuf);
					}
					break;

			case ENDP:	chbuf.prstat = ERROR;
					putmsg(&chbuf);
					break;

			case CLEARP:	putmsg(&chbuf);
					break;

			case USERMSG:	putmsg(&chbuf);
					break;

			case KILL:	chdeath("kill request accepted");
					break;

			default:	chdeath("unknown message received");
					break;
		}
	}
}

getmsg() {	/* get a channel message */

int msgrcv();
int msgflg,msgsz,retval;
long msgtyp;

	msgflg = 0;
	msgtyp = 0;
	msgsz = CHMSGSZ;
	retval = msgrcv(cqid,&chbuf,msgsz,msgtyp,msgflg);
	if (retval == -1) {
		chdeath("bad return from msgrcv");
	}
}

putmsg() {	/* put a channel message */
	
int msgsnd();
int msgflg,msgsz,retval;
struct chmsgblk *chmsg;

	msgflg = 0;
	msgsz = CHMSGSZ;
	if (chbuf.msgsrc == AEND) {
		retval = msgsnd(bqid,&chbuf,msgsz,msgflg);
	} else {
		retval = msgsnd(aqid,&chbuf,msgsz,msgflg);
	}
	if (retval == -1) {
		chdeath("bad return from msgsnd");
	}
}

chdeath(msg)	/* channel death */
char *msg; {

char buf[BUFSIZE];

	fprintf(stderr,"chdemon: %s\n",msg);
	sprintf(buf,"rm /usr/tmp/_CH.%s 2>/dev/null",chname);
	sprintf(buf,"rm /usr/tmp/_FPA.%s 2>/dev/null",chname);
	sprintf(buf,"rm /usr/tmp/_FPB.%s 2>/dev/null",chname);
	system(buf);
	msgctl(aqid,IPC_RMID);
	msgctl(bqid,IPC_RMID);
	msgctl(cqid,IPC_RMID);
	semctl(asem,0,IPC_RMID,0L);
	semctl(bsem,0,IPC_RMID,0L);
	exit(1);
}
