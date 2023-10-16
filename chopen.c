#include "chdefs.h"

/*
 * Chopen(channel-name, channel-end) - open a grammatical channel 
 * having the name channel-name.  Channel-end is the channel end to be opened.
 *
 * Returns: SUCCESS - channel opened successfully
 *          NOCHANNEL - channel does not exist
 *          DUPCHANNEL - channel already open at channel-end
 *          TOOMANY - limit on open channels exists
 *          FAIL - failure
 *
 */

chopen(chname, chend)
char *chname; 
int chend; {

char buf[BUFSIZE],chopen[STRSIZE];
int cend,i;
FILE *fp,*fopen();

	/* check if the channel exists */
	sprintf(buf,"/usr/tmp/_CH.%s",chname);
	if ((fp = fopen(buf,"r")) == NULL) {
		return(NOCHANNEL);
	}
	fclose(fp);

	/* check if the channel already open and for open channel limit */
	sprintf(buf,"/usr/tmp/_CO.%d",getpid());
	if ((fp = fopen(buf,"r")) != NULL) {
		i = 0;
		while ((fscanf(fp,"%s %d",chopen,&cend)) != EOF) {
			if (strcmp(chopen,chname) == 0 && cend == chend) {
				fclose(fp);
				return(DUPCHANNEL);
			}
			i++;
		}
		fclose(fp);
		if (i >= CHOPLIMIT) {
			return(TOOMANY);
		}
	}

	/* add the channel to the process's open channel list */
	sprintf(buf,"/usr/tmp/_CO.%d",getpid());
	if ((fp = fopen(buf,"a")) == NULL) {
		return(FAIL);
	}
	fprintf(fp,"%s %d\n",chname,chend);
	fclose(fp);

	return(SUCCESS);
}
