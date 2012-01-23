%{
#include <stdio.h>
%}
cifra	[0-9]
numero 	[0-9][0-9]+
ident	[a-zA-Z][a-zA-Z0-9_]*
%%
"<?php"		return "inizio_listato";
"?>"		return "termine_listato";
for		return "for";
if		return "if";
else		return "else";
$[ident]	return yytext+1;	//+1 perchè ritorno solo il nome dell'identificatore, poichè il primo carattere è il DOLLORO
die		return "die";
echo		return "echo";
printf		return "printf";

%%

