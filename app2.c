#include "chdefs.h"	/* channel definitions */
#include "route.h"	/* routing definitions */
#include "callp.h"	/* call protocol definitions */

/*
 * This program is executed by the originating process
 * to handle the originating side of a telephone call.
 * The called party address is known when the process
 * is started.
 *
 * For simplicity, no error legs are shown.
 *
 */

main() {

	char chname[STRSIZE];	/* channel name */
	int chend;		/* channel end */
	int chfunc;		/* channel function */
	char prname[STRSIZE];	/* protocol name */
	int prstat;		/* protocol status */
	int mtype;		/* message type */
	char mtext[STRSIZE];	/* message text */
	int talk;		/* talk flag */

	/*
	 * Create a call channel to talk to terminating process, and
	 * start the protocol on it.
	 */

	chcreat(CALLCH);
	chopen(CALLCH,AEND);
	chsend(CALLCH,AEND,STARTP,CALLP,NOMTYPE,NOMTEXT);

	/* 
	 * Request to connect to called party.
	 */
	
	chopen(ROUTCH,AEND);
	strcpy(mtext,strcat(CALLED_ADDR,CALLCH));
	chsend(ROUTCH,AEND,USERMSG,NOPROT,CONNREQ,mtext);
	chclose(ROUTCH,AEND);

	/*
	 * Wait for a reply on the call channel.
	 */

	chrcv(chname,&chend,&chfunc,prname,&prstat,&mtype,mtext);

	/* 
	 * Find out the called party condition.
	 * If busy or bad address, terminate processing.
	 */

	if (mtype == BUSY || mtype == BAD_ADDRESS) {
		talk = FALSE;
	} else {
		talk = TRUE;
	}

	/*
	 * If got answer, then in talking state.  Wait for
	 * calling party to hang up or terminating
	 * end to disconnect or request hold.
	 */

	while (talk == TRUE) {

		/*
		 * Open channel to calling party
		 * to listen for hang up signal
		 */

		chopen(ORIGCH,AEND);
		chrcv(chname,&chend,&chfunc,prname,&prstat,&mtype,mtext);
		chclose(ORIGCH,AEND);

		/*
		 * Who sent message?
		 */

		if (strcmp(chname,ORIGCH) == 0) {

			/*
			 * Caller hung up - send disconnect
			 * and wait for terminator to acknowledge 
			 */
 
			chsend(CALLCH,AEND,USERMSG,CALLP,DISC,NOMTEXT);
			chrcv(chname,&chend,&chfunc,prname,&prstat,&mtype,mtext);

			/*
			 * If did not get acknowledgment, then
			 * race to disconnect or hold occurred -
			 * terminating end should recognize and
			 * acknowledge.
			 */

			if (mtype != ACK) {

				/* wait again for acknowledgment */
				chrcv(chname,&chend,&chfunc,prname,&prstat,&mtype,mtext);
			}

			chsend(CALLCH,AEND,ENDP,CALLP,NOMTYPE,NOMTEXT);
			talk = FALSE;

		} else {	/* got message from terminating end */

			/*
			 * Got hold request?
			 */

			if (mtype == HOLD) {

				/* 
				 * Acknowledge and wait for release 
				 * or disconnect from terminator.
				 */

				chsend(CALLCH,AEND,USERMSG,CALLP,ACK,NOMTEXT);
				chrcv(chname,&chend,&chfunc,prname,&prstat,&mtype,mtext);

				if (mtype == DISC) {

					/*
					 * Acknowledge disconnect from 
					 * terminator.
					 */

					chsend(CALLCH,AEND,USERMSG,CALLP,ACK,NOMTEXT);
					chsend(CALLCH,AEND,ENDP,CALLP,NOMTYPE,NOMTEXT);
					talk = FALSE;

				} else {

					/*
					 * Got release - listen
					 * again to caller and
					 * go back to talking.
					 */

					chopen(ORIGCH,AEND);

				}

			} else {

				/*
				 * Acknowledge disconnect from terminator.
				 */
				 
				chsend(CALLCH,AEND,USERMSG,CALLP,ACK,NOMTEXT);
				chsend(CALLCH,AEND,ENDP,CALLP,NOMTYPE,NOMTEXT);
				talk = FALSE;

			}
		}
	}

	/*
	 * Perform termination functions.
	 */

	chclose(CALLCH,AEND);
	chkill(CALLCH);
		
}
