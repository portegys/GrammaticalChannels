#include "chdefs.h"

/*
 * chinit - create the channel data file and exec the channel demon.
 * The channel name, message queue ids, and semaphore ids are
 * given via argc/argv.
 * 
 */

main(argc,argv)
int argc;
char *argv[]; {

char chname[STRSIZE],buf[BUFSIZE];
int aqid,bqid,cqid,asem,bsem;

	/* get the channel name and message queue ids */
	strcpy(chname,*++argv);
	aqid = atoi(*++argv);
	bqid = atoi(*++argv);
	cqid = atoi(*++argv);
	asem = atoi(*++argv);
	bsem = atoi(*++argv);

	sprintf(buf,"echo %d %d %d %d %d %d>/usr/tmp/_CH.%s",aqid,bqid,cqid,asem,bsem,getpid(),chname);
	system(buf);

	/* exec the channel demon */
	execlp("chdemon","chdemon",chname,0);
}
