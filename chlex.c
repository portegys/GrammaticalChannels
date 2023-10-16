int aqid,bqid,cqid,asem,bsem;
struct chmsgbuf chbuf;
char chname[STRSIZE];
char cname[STRSIZE];
char prname[STRSIZE];
int skipflg;
int pid;

/* 
 * Grammatical channel lexical analysis program.
 * This program routes messages passing through the channel into the
 * protocol parsing program.  It also catches errors in message parsing and
 * requests to return to the non-parsing mode.
 *
 * The parser program is a yacc program created from user protocol
 * specfications.
 *
 */

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

	/* get channel data */
	strcpy(prname,*argv);
	strcpy(chname,*++argv);
	sprintf(buf,"/usr/tmp/_CH.%s",chname);
	if ((fp = fopen(buf,"r")) == NULL) {
		printf(stderr,"%s: cannot open channel file %s\n",prname,buf);
		exit(1);
	}

	fscanf(fp,"%d %d %d %d %d %d",&aqid,&bqid,&cqid,&asem,&bsem,&pid);
	fclose(fp);

	skipflg = FALSE;
	strcpy(cname,chname);

	/* call parser */
	yyparse();

	/* parser returned ok - return to no-parse mode */
	chbuf.prstat = IDLE;
	putmsg();
	chreset();

}

sigproc() {	/* process signals */
	chdeath("signal received - channel terminated");
}

yylex() {	/* lexical analysis for channel messages */

	if (skipflg == FALSE) { getmsg(); } 
	chbuf.prstat = ACTIVE;
	strcpy(chbuf.prname,prname);
	switch(chbuf.chfunc) {
		case STARTP:	chbuf.prname[0] = '\0';
				chbuf.prstat = ERROR;
				putmsg();
				chreset();
				break;

		/* end protocol request - return end token */
		case ENDP:	return(0);
				break;

		case CLEARP:	chbuf.prname[0] = '\0';
				chbuf.prstat = IDLE;
				putmsg();
				chreset();
				break;

		case USERMSG:	if (skipflg == FALSE) {
					skipflg = TRUE;
					if (chbuf.msgsrc == AEND) {
						return(_AMSGTOK);
					} else {
						return (_BMSGTOK);
					}
				} else {
					skipflg = FALSE;
					return(chbuf.usrmtype);
				}
				break;

		case KILL:	chdeath("kill request accepted");
				break;

		default:	chdeath("unknown message received");
				break;
	}
}

yyerror(s)	/* catch parsing errors */ 
char *s; {
	chbuf.prname[0] = '\0';
	chbuf.prstat = ERROR;
	putmsg();
	chreset();
}

chreset() {	/* reset to non-parsing mode */
	execlp("chdemon","chdemon",cname,0);
}

getmsg() {	/* get channel message */

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

putmsg() {	/* put channel message */
	
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

	fprintf(stderr,"%s: %s\n",prname,msg);
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
