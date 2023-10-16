#include "chdefs.h"

/*
 * Chclose(channel-name, channel-end) - close a grammatical channel.
 * Channel-name is the channel.  Channel-end is the end to be closed.
 *
 * Returns: SUCCESS - channel closed successfully
 *          FAIL - failure
 *
 */

chclose(chname, chend)
char *chname; 
int chend; {

char buf[BUFSIZE],chopen[STRSIZE];
char chsv[CHOPLIMIT] [STRSIZE];
int cend,endsv[CHOPLIMIT];
int i,j;
FILE *fp,*fopen();

	/* close the channel */
	sprintf(buf,"/usr/tmp/_CO.%d",getpid());
	if ((fp = fopen(buf,"r")) != NULL) {
		i = 0;
		while ((fscanf(fp,"%s %d",chopen,&cend)) != EOF) {
			if (strcmp(chopen,chname) != 0 || cend != chend) {
				strcpy(chsv[i],chopen);
				endsv[i] = cend;
				i++;
			}
		}
		fclose(fp);
		if (i == 0) {
			sprintf(buf,"rm /usr/tmp/_CO.%d 2>/dev/null",getpid());
			system(buf);
		} else {
			if ((fp = fopen(buf,"w")) == NULL) {
				return(FAIL);
			} else {
				for (j = i, i = 0; i < j; i++) {
					fprintf(fp,"%s %d\n",chsv[i],endsv[i]);
				}
				fclose(fp);
			}
		}
	}
	return(SUCCESS);
}
