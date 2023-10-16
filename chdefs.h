/*
 * Grammatical channel definitions.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>

#define AEND 0			/* message from A end */
#define BEND 1			/* message from B end */

#define STRSIZE 20		/* string size */
#define BUFSIZE 200		/* buffer size */
#define TRUE 1
#define FALSE 0
#define null -1

/* channel message types */
#define USERMSG 1		/* user message */
#define KILL 2			/* channel kill */
#define CLEARP 3		/* clear protocol */
#define STARTP 4		/* start protocol */
#define ENDP 5			/* end protocol */

/* channel parameters */
#define PMODE 511		/* message queue permission mode */
#define CHOPLIMIT 10		/* open channel limit */

/* protocol statuses */
#define ACTIVE 0
#define IDLE 1
#define ERROR 2

/* return codes */
#define SUCCESS 0
#define FAIL 1
#define NOCHANNEL 2
#define DUPCHANNEL 3
#define TOOMANY 4
#define NOTOPEN 5

struct chmsgbuf {		/* channel message buffer */
	char chname[STRSIZE];	/* channel name */
	int chend;		/* channel end */
	long chfunc;		/* channel function */
	char prname[STRSIZE];	/* protocol in effect */
	int prstat;		/* protocol status message */
	int msgsrc;		/* message source */
	int usrmtype;		/* user message type */
	char usrmtext[STRSIZE];	/* user message text */
};

/* channel message size */
#define CHMSGSZ sizeof(struct chmsgbuf) - sizeof(long) + 1
