%{

/**
+-----------------------------------------------------------------------+
| PHP2C -- scanner.l 						|
+-----------------------------------------------------------------------+
|									|
|  Autori: 	Vito Manghisi 						|
| 		Gianluca Grasso						|
+-----------------------------------------------------------------------+
*	
* Input file per Flex, generatore dello scanner per il traduttore da PHP a C
*
*/

#include <stdio.h>
#include "parser.tab.h"

%}

%x ST_IN_SCRIPTING
%x ST_DOUBLE_QUOTES
%x ST_SINGLE_QUOTE
%x ST_COMMENT
%x ST_DOC_COMMENT
%x ST_ONE_LINE_COMMENT
%option stack

%{
/** variabile che tiene traccia della linea di input corrente */
int lineno = 1;
/** indica la riga dove è stato aperto un commento di tipo /* o /** che non è stato chiuso ( si veda la riga 499 )*/
int comment_start_line = 0;
%}

/** Definizione della regex che matcha uno o più digit numeriche*/
LNUM [0-9]+
/** Definizione della regex che matcha una o più digit numerica negativa, nella forma (-n), con n intero da 0 a 9 */
NLNUM ("(-")[0-9]+(")")
/** Definizione della regex che matcha una floating del tipo [n].n oppure n.[n], 
    dove n è un intero da 0 a 9, e [] indica opzionalità. */
DNUM ([0-9]*[\.][0-9]+)|([0-9]+[\.][0-9]*)
/** Definizione della regex che matcha una floating negativa nella forma (-[n].n) oppure (-n.[n]) 
    dove n è un intero da 0 a 9 e [] indicano opzionalità. */
NDNUM ("(-")([0-9]*[\.][0-9]+(")"))|("(-"[0-9]+[\.][0-9]*(")"))
/** Definizione della regex per le label costanti nella forma _ALFA[ALFA|0-9], cioè che iniziano con un underscore 
    seguito da almeno un letterale char maiuscolo, seguito opzionalmente da letterali maiuscoli o numeri da 0 a 9*/
CONST_LABEL "_"[A-Z][A-Z0-9_]*
/** Definizione della regex per le label delle variabili, costituite da almeno un letterale a carattere maiuscolo o minuscolo da
    A a Z, un underscore, oppure un carattere nel range UNICODE \x7f-\xff */
LABEL [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
WHITESPACE [ \n\r\t]+
TABS_AND_SPACES [ \t]*
TOKENS [;:,.\[\]()|^&+-/*=%!~$<>?@]
ENCAPSED_TOKENS [\[\]{}$]
ESCAPED_AND_WHITESPACE [\n\t\r #'.:;,()|^&+-/*=%!~<>?@]+
ANY_CHAR (.|[\n])
NEWLINE ("\r"|"\n"|"\r\n")

/** Imposta lo scanner per mantenere il numero della linea di input corrente nella variabile globale yylineno */
%option yylineno

/** Imposta lo scanner per NON chiamare la procedura yywrap() al termine del file di input */
%option noyywrap


%%


<ST_IN_SCRIPTING>"if" {
return T_IF;
}

<ST_IN_SCRIPTING>"elseif" {
return T_ELSEIF;
}

<ST_IN_SCRIPTING>"else" {
return T_ELSE;
}

<ST_IN_SCRIPTING>"while" {
return T_WHILE;
}

<ST_IN_SCRIPTING>"do" {
return T_DO;
}

<ST_IN_SCRIPTING>"for" {
return T_FOR;
}

<ST_IN_SCRIPTING>"switch" {
return T_SWITCH;
}

<ST_IN_SCRIPTING>"case" {
return T_CASE;
}

<ST_IN_SCRIPTING>"default" {
return T_DEFAULT;
}

<ST_IN_SCRIPTING>"break" {
return T_BREAK;
}

<ST_IN_SCRIPTING>"continue" {
return T_CONTINUE;
}

<ST_IN_SCRIPTING>"echo" {
return T_ECHO;
}

<ST_IN_SCRIPTING>"define" {
return T_DEFINE;
}

<ST_IN_SCRIPTING>"array" {
return T_ARRAY;
}

<ST_IN_SCRIPTING>"++" {
return T_INC;
}

<ST_IN_SCRIPTING>"--" {
return T_DEC;
}

<ST_IN_SCRIPTING>"==" {
return T_IS_EQUAL;
}

<ST_IN_SCRIPTING>"!="|"<>" {
return T_IS_NOT_EQUAL;
}

<ST_IN_SCRIPTING>"<=" { 
return T_IS_SMALLER_OR_EQUAL;
}

<ST_IN_SCRIPTING>">=" {
return T_IS_GREATER_OR_EQUAL;
}

<ST_IN_SCRIPTING>"+=" {
return T_PLUS_EQUAL;
}

<ST_IN_SCRIPTING>"-=" {
return T_MINUS_EQUAL;
}

<ST_IN_SCRIPTING>"*=" {
return T_MUL_EQUAL;
}

<ST_IN_SCRIPTING>"/=" {
return T_DIV_EQUAL;
}

<ST_IN_SCRIPTING>"%=" {
return T_MOD_EQUAL;
}

<ST_IN_SCRIPTING>"||" {
return T_BOOLEAN_OR;
}

<ST_IN_SCRIPTING>"&&" {
return T_BOOLEAN_AND;
}

<ST_IN_SCRIPTING>"0R" {
return T_LOGICAL_OR;
}

<ST_IN_SCRIPTING>"AND" {
return T_LOGICAL_AND;
}

<ST_IN_SCRIPTING>{TOKENS} {
return yytext[0];
}

<ST_IN_SCRIPTING>"{" {
yy_push_state(ST_IN_SCRIPTING);
return '{';
}

<ST_IN_SCRIPTING>"}" {
/* This is a temporary fix which is dependant on flex and it's implementation */
if ( yy_start_stack_ptr ) {
yy_pop_state( );
}
return '}';
}

<ST_IN_SCRIPTING>{LNUM} {
yylval.id = ( char * )strdup( yytext );
return T_LNUMBER;
}

<ST_IN_SCRIPTING>{NLNUM} {
/*elimina le parentesi tonde, fra le quali è compreso il numero intero negativo.*/
char *s = ( char * )malloc( ( strlen( yytext ) - 1 )*sizeof( char ) );
int j = 0;
int i;
for(i = 1; i < strlen( yytext ) - 1; i++) {
s[j]=yytext[i];
j++;
}
s[strlen( s )] = '\0';
yylval.id = s;
return T_LNUMBER;
}

<ST_DOUBLE_QUOTES>{LNUM}|{NLNUM} { 
/* treat numbers (almost) as strings inside encapsulated strings */
/*se è un numero negativo elimino le parentesi tonde.*/
char *s = ( char * )malloc( ( strlen( yytext ) - 1 )*sizeof( char ) );
s = ( char * )strndup( yytext, 1 );
if( strcmp( s, "(" ) == 0 ) {
int j = 0;
int i;
for( i = 1; i < strlen( yytext ) - 1; i++) {
s[j] = yytext[i];
j++;
}
s[strlen( s )] = '\0';
yylval.id = s;
} else
yylval.id = ( char * )strdup( yytext );

return T_NUM_STRING;
}

<ST_IN_SCRIPTING>{DNUM} {
yylval.id = ( char * )strdup( yytext );
return T_DNUMBER;
}

<ST_IN_SCRIPTING>{NDNUM} {
/*elimina le parentesi tonde, fra le quali è compreso il numero reale negativo.*/
char *s=( char * )malloc( ( strlen( yytext ) - 1 )*sizeof( char ) );
int j = 0;
int i;
for( i = 1; i < strlen( yytext ) - 1; i++) {
s[j] = yytext[i];
j++;
}
s[strlen( s )] = '\0';

yylval.id = s;
return T_DNUMBER;
}

<INITIAL>"<?php"([ \t]|{NEWLINE}) {
if (yytext[yyleng-1] == '\n') {
lineno++;
}
BEGIN(ST_IN_SCRIPTING);
return T_INIT;
}

<ST_IN_SCRIPTING>("?>"){NEWLINE}? {
BEGIN(INITIAL);
return T_FINAL;
}

<ST_IN_SCRIPTING,ST_DOUBLE_QUOTES>"$"{LABEL} {
yylval.id = ( char * )strdup( yytext + 1 );
return T_VARIABLE;
}

<ST_IN_SCRIPTING,ST_DOUBLE_QUOTES>"$"[0-9]{LABEL} {
/*Nel caso in cui si digiti erroneamente un numero dopo il $, parte una correzione automatica.*/
printf("\033[01;34mRiga: %i. CORREZIONE LESSICALE: e' stato corretto il nome della variabile \"%s\".\033[00m\n", yylineno, ( char * )strdup( yytext + 1 ) );
yylval.id = ( char * )strdup( yytext + 2 );
return T_VARIABLE;
}

<ST_IN_SCRIPTING,ST_DOUBLE_QUOTES>"$"[0-9][0-9]+{LABEL} {
/*Nel caso in cui si digiti erroneamente più di un numero dopo il $, parte una correzione automatica.*/
printf("\033[01;34mRiga: %i. CORREZIONE LESSICALE: e' stato corretto il nome della variabile \"%s\".\033[00m\n", yylineno, ( char * )strdup( yytext + 1 ) );
char *str = ( char * )strdup( yytext );
int num = 0;

while( *str )
{
/*conta le cifre.*/
if( isdigit( *str ) )
num++;
str++;
}
/*Salta il simbolo "$" e le cifre*/
yylval.id = ( char * )strdup( yytext + 1 + num );

return T_VARIABLE;
}

<ST_IN_SCRIPTING,ST_DOUBLE_QUOTES>{CONST_LABEL} {
yylval.id = ( char * )strdup( yytext );
return T_CONSTANT;
}


<ST_IN_SCRIPTING>{LABEL} {
yylval.id = ( char * )strdup( yytext );
return T_STRING;
}

<ST_DOUBLE_QUOTES>{LABEL} {
yylval.id = ( char * )strdup( yytext );
return T_STRING;
}

<ST_IN_SCRIPTING>{WHITESPACE} {
int i;
for (i = 0; i < yyleng; i++) {
if (yytext[i] == '\n') {
lineno++;
}
}
/* elimina gli spazi, le tabulazioni e i newline nel codice contando le righe */
}

<ST_IN_SCRIPTING>"#"|"//" {
BEGIN(ST_ONE_LINE_COMMENT);
yymore();
}

<ST_ONE_LINE_COMMENT>"?"|">" {
yymore();
}

<ST_ONE_LINE_COMMENT>[^\n\r?%>]+{ANY_CHAR} {
switch ( yytext[yyleng - 1] ) {
case '?': case '%': case '>':
yyless(yyleng-1);
yymore();
break;
case '\n':
lineno++;
/* intentional fall through */
default:
BEGIN(ST_IN_SCRIPTING);
}
}

<ST_ONE_LINE_COMMENT>{NEWLINE} {
BEGIN(ST_IN_SCRIPTING);
lineno++;
}

<ST_ONE_LINE_COMMENT>"?>" {
yymore();
}

<ST_IN_SCRIPTING>"/**"{WHITESPACE} {
comment_start_line = lineno;
BEGIN(ST_DOC_COMMENT);
yymore();
}

<ST_IN_SCRIPTING>"/*" {
comment_start_line = lineno;
BEGIN(ST_COMMENT);
yymore();
}

<ST_COMMENT,ST_DOC_COMMENT>[^*]+ {
yymore();
}

<ST_DOC_COMMENT>"*/" {
BEGIN(ST_IN_SCRIPTING);
}

<ST_COMMENT>"*/" {
BEGIN(ST_IN_SCRIPTING);
}

<ST_COMMENT,ST_DOC_COMMENT>"*" {
yymore();
}

<ST_IN_SCRIPTING>([']({CONST_LABEL})[']) {
/*elimina i singoli apici, restituendo il nome della costante.*/
char *s = ( char * )malloc( ( strlen( yytext ) - 1 )*sizeof( char ) );
int j = 0;
int i;
for( i = 1; i < strlen( yytext ) - 1; i++) {
s[j] = yytext[i];
j++;
}
s[strlen( s )] = '\0';
yylval.id = s;
return T_CONSTANT;
}

<ST_IN_SCRIPTING>(["]({CONST_LABEL})["]) {
/*elimina i doppi apici, restituendo il nome della costante.*/
char *s = ( char * )malloc( ( strlen( yytext ) - 1 )*sizeof( char ) );
int j = 0;
int i;
for( i = 1; i < strlen( yytext ) - 1; i++) {
s[j] = yytext[i];
j++;
}
s[strlen( s )] = '\0';
yylval.id = s;
return T_CONSTANT;
}

<ST_IN_SCRIPTING>(["]([^$_"\\]|("\\".))*["]) {
yylval.id = ( char * )strdup( yytext );
return T_CONSTANT_ENCAPSED_STRING;
}

<ST_IN_SCRIPTING>([']([^'\\]|("\\".))*[']) {
/*sostituisce ai singli apici i doppi apici, questo per evitare problemi con la definizione ( parola chiave define ) delle costanti.*/
char *s=( char * )malloc( ( strlen( yytext ) - 1 )*sizeof( char ) );
int j = 1;
s[0] = '"';
int i;
for( i = 1; i < strlen( yytext ) - 1; i++) {
s[j] = yytext[ i ];
j++;
}
s[j] = '"';
s[strlen( s )] = '\0';

yylval.id = s;
return T_CONSTANT_ENCAPSED_STRING;
}

<ST_IN_SCRIPTING>["] {
BEGIN(ST_DOUBLE_QUOTES);
return '\"';
}

<ST_IN_SCRIPTING>['] {
BEGIN(ST_SINGLE_QUOTE);
return '\'';
}

<ST_DOUBLE_QUOTES>{ESCAPED_AND_WHITESPACE} {
/*elimina gli spazi bianchi, questo per evitare che in traduzione compaiano spazi bianchi indesiderati.*/
char *s= ( char * )strdup( yytext );
if( strcmp( s, " " ) != 0 )
yylval.id = s;
else
yylval.id = "";

return T_ENCAPSED_AND_WHITESPACE;
}

<ST_SINGLE_QUOTE>([^'\\]|\\[^'\\])+ {
return T_ENCAPSED_AND_WHITESPACE;
}

<ST_DOUBLE_QUOTES>[`]+ {
return T_ENCAPSED_AND_WHITESPACE;
}

<ST_DOUBLE_QUOTES>"$"[^a-zA-Z_\x7f-\xff{] {
if (yyleng == 2) {
yyless(1);
}
return T_CHARACTER;
}

<ST_DOUBLE_QUOTES>{ENCAPSED_TOKENS} {
return yytext[0];
}

<ST_DOUBLE_QUOTES>"\\{" {
return T_STRING;
}

<ST_SINGLE_QUOTE>"\\'" {
return T_CHARACTER;
}

<ST_SINGLE_QUOTE>"\\\\" {
return T_CHARACTER;
}

<ST_DOUBLE_QUOTES>"\\\"" {
return T_CHARACTER;
}

<ST_DOUBLE_QUOTES>"\\"[0-7]{1,3} {
return T_CHARACTER;
}

<ST_DOUBLE_QUOTES>"\\x"[0-9A-Fa-f]{1,2} {
return T_CHARACTER;
}

<ST_DOUBLE_QUOTES>["] {
BEGIN(ST_IN_SCRIPTING);
return '\"';
}

<ST_SINGLE_QUOTE>['] {
BEGIN(ST_IN_SCRIPTING);
return '\'';
}

<ST_COMMENT,ST_DOC_COMMENT><<EOF>> {
printf( "\033[01;33mATTENZIONE: commento iniziato alla riga %d e non terminato.\033[00m\n", comment_start_line );
return 0;
}

<ST_IN_SCRIPTING,INITIAL,ST_DOUBLE_QUOTES,ST_SINGLE_QUOTE>{ANY_CHAR} {
printf( "\033[01;33mATTENZIONE: carattere inatteso in input '%c' (ASCII=%d) state=%d",yytext[0], yytext[0], YYSTATE );
}

%%
