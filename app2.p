
/*
 * A protocol to model the messages in a telephone call
 * between a process handling the originating and 
 * terminating end.  The originator sends to the A end
 * of the channel, and the terminator sends to the B
 * end.  The terminator is able to hold the originator,
 * i.e. prevent from disconnecting, with a HOLD signal.
 *
 * Message tokens:  	ACK - 		acknowledgement
 *	    		ANSWER - 	answer 
 *	    		BAD_ADDRESS -	invalid called party address
 *	    		BUSY - 		busy 
 *			DISC - 		disconnect 
 *			HOLD - 		hold request
 *			RELEASE - 	release of hold
 *
 */

	/* caller to find out the called party condition */

	<start> -> [BUSY] |		/* called is busy */
		   [BAD_ADDRESS] |	/* invalid called party */
		   [ANSWER] <talk>;	/* called answers */

	/* talk state */
	<talk> -> <disconnect> |	/* disconnect */
		     <hold>;		/* hold request */

	/* disconnect state */
	<disconnect> -> (DISC) [ACK] |	/* caller disconnect and acknowledge */
			[DISC] (ACK) |	/* called disconnect and ack */
			(DISC) [DISC] [ACK] |	/* race conditions */
			[DISC] (DISC) [ACK];

	/* hold request state */
	<hold> -> [HOLD] (ACK) [RELEASE] <talk> |  /* hold, then return to talk */
		  [HOLD] (ACK) [DISC] (ACK) |	/* hold, then disconnect */
		  [HOLD] (DISC) [ACK] |		/* race for hold */
		  (DISC) [HOLD] [ACK];
