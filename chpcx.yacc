%{
#include <stdio.h>

/*
 * Grammatical channel protocol compiler parser/translator - translate 
 * a user protocol specification into a yacc program which will parse
 * the protocol in a grammatical channel.
 *
 */

#define MAXTOKEN 100
#define MAXRULE 100
#define STRSIZE 100
#define GROUND -1

char tokentab[MAXTOKEN] [STRSIZE];
int tokenA[MAXTOKEN];
int tokenB[MAXTOKEN];
int tokenptr;

struct {
	int sibptr;
	int chptr;
	char text[STRSIZE];
} ruletab[MAXRULE];
int freeptr,ruleptr,disjptr,conjptr,lastptr;

%}

%token	PTR TERM NT_STRING ST_STRING RT_STRING OR WS

%start	protocol

%%	/* rules */

protocol	:	WS body
		;

body		:	rule
		|	rule body
		;

rule		:	lhs PTR rhs
		;

lhs		:	NT_STRING
			{ 
				if (ruleptr != GROUND) {
					ruletab[ruleptr].sibptr = freeptr;
				}
				ruleptr = freeptr;
				freeptr++;
				strcpy(ruletab[ruleptr].text,yytext); 
				ruletab[ruleptr].chptr = freeptr;
				disjptr = freeptr;
				conjptr = freeptr;
				lastptr = freeptr;
				freeptr++;
			}	
		;

rhs		:	rhst TERM
		|	rhst OR 
			{
				ruletab[disjptr].sibptr = freeptr;
				disjptr = freeptr;
				conjptr = freeptr;
				lastptr = freeptr;
				freeptr++;
			}
			rhs
		;

rhst		:	rhstr 
		|	rhstr WS rhst
		;

rhstr		:	NT_STRING
			{
				if (conjptr != lastptr) {
					ruletab[lastptr].chptr = conjptr;
					lastptr = conjptr;
				}
				strcpy(ruletab[conjptr].text,yytext);
				conjptr = freeptr;
				freeptr++;
			}
		| 	ST_STRING
			{
				if (conjptr != lastptr) {
					ruletab[lastptr].chptr = conjptr;
					lastptr = conjptr;
				}
				strcpy(ruletab[conjptr].text,yytext);
				conjptr = freeptr;
				freeptr++;
				tokencpy(yytext); 
			}
		| 	RT_STRING
			{
				if (conjptr != lastptr) {
					ruletab[lastptr].chptr = conjptr;
					lastptr = conjptr;
				}
				strcpy(ruletab[conjptr].text,yytext);
				conjptr = freeptr;
				freeptr++;
				tokencpy(yytext); 
			}
		;

%%	/* programs */

main() {
	yyinit();
	return(yyparse());
}

yyinit() {
int i;
	for (i = 0; i < MAXTOKEN; i++) {
		tokentab[i] [0] = '\0';
		tokenA[i] = 0;
		tokenB[i] = 0;
	}
	tokenptr = 0;
	for (i = 0; i < MAXRULE; i++) {
		ruletab[i].sibptr = GROUND;
		ruletab[i].chptr = GROUND;
		ruletab[i].text[0] = '\0';
	}
	freeptr = 0;
	ruleptr = GROUND;
}

#include "lex.yy.c"

tokencpy(s)
char *s; {
int i,k,flg;
char buf[STRSIZE];

	for (k = 1; k < STRSIZE && s[k] != '\0'; k++) {
		buf[k-1] = s[k];
	}
	buf[k-2] = '\0';
	for (flg = 0, i = 0; flg == 0 && i < MAXTOKEN && 
	   tokentab[i] [0] != '\0'; i++) {
		if (strcmp(buf,tokentab[i]) == 0) {
			flg = 1;
		}
	}
	if (flg == 0) {
		strcpy(tokentab[tokenptr],buf);
		tokenptr++;
	}
}

yywrap() {	/* parse wrapup - write out user yacc program */
int i,j,k,stpos;

	for (i = 0; ruleptr != GROUND && i != GROUND && strcmp(ruletab[i].text,"<start>") != 0; i = ruletab[i].sibptr) {}
	if (i == GROUND) {
		fprintf(stderr,"chpcx: start symbol not found\n"); exit(1);
	} else {
		stpos = i;		
	}
	printf("%%{\n");
	printf("#include <stdio.h>\n");
	printf("#include \"chdefs.h\"\n");
	printf("%%}\n\n");
	tokenout();
	printf("%%start ");
	strpr(ruletab[stpos].text);
	printf("\n\n%%%%\n\n");
	for (i = 0; ruleptr != GROUND && i != GROUND; i = ruletab[i].sibptr) {
		strpr(ruletab[i].text);
		printf("	: ");
		for (j = ruletab[i].chptr; j != GROUND; j = ruletab[j].sibptr) {
			if (ruletab[j].text[0] == '(' || ruletab[j].text[0] == '[') {
				tokenpr(ruletab[j].text);
			} else {
				strpr(ruletab[j].text);
			}
			for (k = ruletab[j].chptr; k != GROUND; k = ruletab[k].chptr) {
				if (ruletab[k].text[0] == '(' || ruletab[k].text[0] == '[') {
					tokenpr(ruletab[k].text);
				} else {
					strpr(ruletab[k].text);
				}
			}
			printf("\n");
			if (ruletab[j].sibptr != GROUND) {
				printf("		| ");
			}
		}
		printf("		;\n\n");
	}
	tokend();
	printf("\n%%%%\n\n");
	printf("#include \"chlex.c\"\n");
	return(1);
}

tokenout() {
int k;

	printf("%%token ");
	for (k = 0; k < MAXTOKEN && tokentab[k] [0] != '\0'; k++) {
		printf("%s ",tokentab[k]);
	}
	printf("_AMSGTOK _BMSGTOK");
	printf("\n\n");
}

tokenpr(s) 
char *s; {
int k,i,flg;
char buf[STRSIZE];

	for (i = 0, flg = 0; i < MAXTOKEN && flg == 0 &&
	   tokentab[i] [0] != '\0'; i++) {
		for (k = 1; k < STRSIZE && s[k] != '\0'; k++) {
			buf[k-1] = s[k];
		}
		buf[k-2] = '\0';
		if (strcmp(buf,tokentab[i]) == 0) {
			flg = 1;
		}
	}
	if (s[0] == '(') {
		printf("_amsg%d ",i-1);
		tokenA[i-1] = 1;
	} else {
		printf("_bmsg%d ",i-1);
		tokenB[i-1] = 1;
	}
}

tokend() {
int i;

	printf("\n\n");
	for (i = 0; i < MAXTOKEN && tokentab[i] [0] != '\0'; i++) {
		if (tokenA[i] == 1) {
			printf("_amsg%d 	:	_AMSGTOK ",i);
			printf("%s ",tokentab[i]);
			printf("{ putmsg(); } ;\n\n");
		}
		if (tokenB[i] == 1) {
			printf("_bmsg%d 	:	_BMSGTOK ",i);
			printf("%s ",tokentab[i]);
			printf("{ putmsg(); } ;\n\n");
		}
	}
}

strpr(s) 
char *s; {
int k;
char buf[STRSIZE];

	for (k = 1; k < STRSIZE && s[k] != '\0'; k++) {
		buf[k-1] = s[k];
	}
	buf[k-2] = '\0';
	printf("%s ",buf);
}

yyerror(s)
char *s; {
	fprintf(stderr,"%s\n",s);
}
