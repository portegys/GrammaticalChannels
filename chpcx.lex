%{
/*
 * Grammatical channel protocol compiler lex program - find the 
 * tokens in the user protocol specification and send them to the 
 * parser/translator.
 *
 */
%}

%START body
%%

[ \t\n]*< {
	BEGIN body;
	yyless(yyleng-1);
	return(WS);
}

<body><[_A-Za-z0-9]+>/[ \t\n]* {
	return(NT_STRING);
}

<body>[ \t\n]*\-\>[ \t\n]* {
	return(PTR);
}

<body>[ \t\n]*\;[ \t\n]* {
	return(TERM);
}

<body>[ \t\n]*\|[ \t\n]* {
	return(OR);
}

<body>\([_A-Za-z0-9]+\)/[ \t\n]* {
	return(ST_STRING);
}

<body>\[[_A-Za-z0-9]+\]/[ \t\n]* {
	return(RT_STRING);
}

<body>[ \t\n]* {
	return(WS);
}
