#include "chdefs.h"

/*
 * chsend(channel-name, channel-end, channel-function, protocol-name, 
 *       user-message-type, user-message-text) -
 * Send a grammatical channel message.
 *
 * Input: channel-name
 *        channel-end - which end to send the message to (AEND or BEND)
 *        channel-function - the function performed by the message:
 *                            STARTP: start a protocol
 *                            ENDP: end a protocol
 *                            CLEARP: clear request (ends protocol)
 *                            USERMSG: a user message
 *        protocol-name - requested protocol name (program having
 *			  this name will be started)
 *        user-message-type - user message type
 *        user-message-text - user message text
 *
 * Returns: SUCCESS - message sent successfully
 *          NOCHANNEL - channel does not exist
 * 	    NOTOPEN - channel not open at channel-end
 *          FAIL - something went wrong
 *
 */

chsend(chname,chend,chfunc,prname,usrmtype,usrmtext)
char *chname;
int chend;
int chfunc;
char *prname;
int usrmtype;
char *usrmtext; {

int aqid,bqid,cqid,pid,asem,bsem;
int msgsnd();
int msgflg,msgsz,retval;
struct chmsgbuf chmsg;
char buf[BUFSIZE];
FILE *fp,*fopen();

	/* check if channel open */
	sprintf(buf,"grep \'%s %d\' /usr/tmp/_CO.%d >/dev/null 2>/dev/null",chname,chend,getpid());
	if (system(buf) != 0) {
		return(NOTOPEN);
	}

	/* get channel data */
	sprintf(buf,"/usr/tmp/_CH.%s",chname);
	if ((fp = fopen(buf,"r")) == NULL) {
		return(NOCHANNEL);
	}
	fscanf(fp,"%d %d %d %d %d %d",&aqid,&bqid,&cqid,&asem,&bsem,&pid);
	fclose(fp);
	
	/* send message */
	msgflg = 0;
	strcpy(chmsg.chname,chname);
	chmsg.chfunc = chfunc;
	strcpy(chmsg.prname,prname);
	chmsg.msgsrc = chend;
	chmsg.usrmtype = usrmtype;
	strcpy(chmsg.usrmtext,usrmtext);
	msgsz = CHMSGSZ;
	retval = msgsnd(cqid,&chmsg,msgsz,msgflg);
	if (retval == -1) {
		return(FAIL);
	}
	return(SUCCESS);
}
