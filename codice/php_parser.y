 %{
 /*
 +----------------------------------------------------------------------+
 | PHP2C -- php_parser.y |
 +----------------------------------------------------------------------+
 | Autori: BAVARO Gianvito |
 | CAPURSO Domenico |
 | DONVITO Marcello |
 +----------------------------------------------------------------------+
 */

 /*
 * LALR conflitti shift/reduce e come essi sono stati risolti:
 *
 * - 4 conflitti shift/reduce a causa dell'ambiguità pendente fra le regole ( corretta e con
errori ) del costrutto if. Risolti mediante shift.
 * - 2 conflitti shift/reduce a causa dell'ambiguità pendente sui costrutti elseif/else.
Risolti mediante shift.
 * - 6 conflitti shift/reduce a causa delle assegnazioni, semplici o in forma compatta, di
valori a elementi di un array. Risolti mediante shift.
 * - 1 conflitto shift/reduce a causa dell'ambiguità pendente fra le due regole ( corretta e
con errori ) del costrutto for. Risolto mediante shift.
 * - 39 conflitti shift/reduce a causa dell'ambiguità ( introdotta con le azioni semantiche
fprintf ) pendente fra tutte le espressioni avente i
 simboli '(' e ')'. Risolti mediante shift.
 *
 */

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include "symbol_table.h"

 #define YYDEBUG 1

 element_ptr element;
 void yyerror( const char *s ); /* Il prototipo della funzione yyerror per la  visualizzazione degli errori sintattici. */
 extern int yylineno; /* Il numero riga segnalato dalla variabile yylineno di Flex. */
 char *current_value; /* Il valore corrente di una variabile o costante. */
 char *current_type; /* Il tipo corrente di una variabile o costante. */
 char *index_element = NULL; /* L'offset di un elemento di un array. */
 int _error = 0, _warning = 0, dim = 0; /* Contatore di errori, di warnings e di elementi di un array (che definiscono la sua dimensione.) */
 int ntab = 0; /* Contatore dei simboli di tabulazione. */
 FILE *f_ptr; /* Puntatore al file che conterrà la traduzione in C. */

 bool array, read; /* Flag usati per discriminare la dichiarazione di un  array, una variabile in lettura o scrittura. */

 /* La funzione check avvia il controllo di una variabile, costante o elemento di un array.
 Gli argomento sono:
 - nameToken, il nome del simbolo da aggiungere;
 - offset, l'indice dell'elemento;
 - nr, il numero riga segnalato dalla variabile yylineno di Flex;
 - read, specifica se l'elemento è analizzato in lettura o scrittura
 ( solo per elementi di un array ). */
 void check( char *nameToken, char *offset, int nr, bool read )
 {
 element = check_element( nameToken, offset, yylineno, read );
 current_value = element->value;
 }

 %}

 %expect 52

 //Associazione degli operatori e dei Token
 %left ','
 %left T_LOGICAL_OR
 %left T_LOGICAL_AND
 %right T_PRINT
 %left '=' T_PLUS_EQUAL T_MINUS_EQUAL T_MUL_EQUAL T_DIV_EQUAL T_CONCAT_EQUAL T_MOD_EQUAL
 %left '?' ':'
 %left T_BOOLEAN_OR
 %left T_BOOLEAN_AND
 %nonassoc <id> T_IS_EQUAL T_IS_NOT_EQUAL
 %nonassoc '<' T_IS_SMALLER_OR_EQUAL '>' T_IS_GREATER_OR_EQUAL
 %left '+' '-' '.'
 %left '*' '/' '%'
 %right '!'
 %right '~' T_INC T_DEC
 %token T_IF
 %left T_ELSEIF
 %left T_ELSE
 %left T_ENDIF
 %left ')'
 %token <id> T_LNUMBER T_DNUMBER T_STRING T_VARIABLE T_CONSTANT T_NUM_STRING T_ENCAPSED_AND_WHITESPACE T_CONSTANT_ENCAPSED_STRING
 %token T_CHARACTER
 %token T_ECHO
 %token T_DO
 %token T_WHILE
 %token T_FOR
 %token T_SWITCH
 %token T_CASE
 %token T_DEFAULT
 %token T_BREAK
 %token T_CONTINUE
 %token T_ARRAY
 %token T_DEFINE
 %token T_WHITESPACE
 %token T_INIT
 %token T_FINAL

 /* L’opzione %union consente di specificare una varietà di tipi di dato che sono usati dalla
variabile yylval in Flex. L'unico tipo specificato è la
 stringa di caratteri. */
 %union{
 char *id;
 }

 %type <id> variable r_variable w_variable element_array encaps_var common_scalar;

 %start start;

 %error-verbose
 %locations


 %% /* Regole */

 /* Ogni volta che viene letto il simbolo di inizio script PHP "<?php" viene cancellato il file
f_out.c, frutto di precedenti compilazioni, e viene creato e aperto in modalità scrittura il
nuovo file. Tutte le le funzioni che iniziano con gen_*, dichiarate nel file gen_code.h, hanno
il compito di stampare nel file costrutti o espressioni. */
 start:
 T_INIT { cancella_file( ); f_ptr = apri_file( ); gen_header( f_ptr ); }
 top_statement_list
 ;

 /* La funzione fprintf sono utilizzate per stampare nel file f_out.c, contenente la traduzione
dei sorgenti in PHP in C, parte dei costrutti, parentesi ecc. La funzione gen_tab e il contatore
ntab servono per stampare nel file i simboli tab "\t" */
 end:
 T_FINAL { fprintf( f_ptr, "\n}" ); chiudi_file( f_ptr ); }
 ;

 top_statement_list:
 top_statement_list top_statement
 | /* empty */
 ;

 top_statement:
 { fprintf( f_ptr, "\t" ); } statement
 ;

 inner_statement_list:
 inner_statement_list { fprintf( f_ptr, "\t" ); } inner_statement
 | /* empty */
 ;

 inner_statement:
 { gen_tab( f_ptr, ntab ); } statement
 ;

 statement:
 unticked_statement
 ;

 /* Tale regola definisce i costrutti primari e le istruzioni principali del PHP. Solo per le
costanti è utilizzata la funzione add_element ( presente nel file symbol_table.h ) che consente
di aggiungere nuovi elementi nella tabella dei simboli, quando esse sono dichiarate mediante il
token T_DEFINE. Per tutti gli altri costrutti si utilizzano le funzioni fprintf. Nel caso di
espressioni, expr, e dell'istruzione echo, T_ECHO, viene gestita la visualizzazione dei messaggi
di warning. Molto spesso è richiamata la funzione clear per svuotare le liste concatenate T, Exp
e Phrase ( tutte contenute nel file utility.h ). */
 unticked_statement:
 '{' inner_statement_list { fprintf( f_ptr, "\t" ); } '}'
 | T_DEFINE '(' T_CONSTANT ',' common_scalar ')' { clear( ); add_element( $3, "constant", current_type, $5, 0, yylineno ); gen_constant( f_ptr, $3, current_type, $5 ); } ';'
 | T_IF '(' expr ')' { gen_if( f_ptr, Exp ); clear( ); ntab++; } statement elseif_list else_single 
 | T_IF '(' error ')' statement elseif_list else_single { yyerror( "ERRORE SINTATTICO: espressione nel costrutto IF non accettata" ); }
 | T_IF expr ')' statement elseif_list else_single { yyerror( "ERRORE SINTATTICO: '(' mancante nel costrutto IF" ); }
 | T_WHILE '(' expr ')' { gen_while( f_ptr, Exp ); clear( ); } while_statement { fprintf( f_ptr, "}\n" ); }
 | T_WHILE '(' error ')' while_statement { yyerror( "ERRORE SINTATTICO: espressione nel costrutto WHILE non accettata" ); }
 | T_WHILE expr ')' while_statement { yyerror( "ERRORE SINTATTICO: '(' mancante nel costrutto WHILE" ); }
 | T_DO { fprintf( f_ptr, "do {\n" ); ntab++; } statement T_WHILE { ntab--; gen_tab( f_ptr, ntab ); fprintf( f_ptr, "} while( " ); } 
    '(' expr ')' { print_expression( f_ptr, Exp );
      fprintf( f_ptr, " );\n" ); clear( ); } ';'
 | T_FOR
 '(' { fprintf( f_ptr, "for( " ); }
 for_expr
 ';' { fprintf( f_ptr, "; " ); }
 for_expr { print_expression( f_ptr, Exp ); clear( ); }
 ';' { fprintf( f_ptr, "; " ); }
 for_expr { clear( ); }
 ')' { fprintf( f_ptr, " ) {\n" ); }
 for_statement { gen_tab( f_ptr, ntab); fprintf( f_ptr, "}\n" ); clear( ); }
 | T_FOR '(' error ';' error ';' error ')' for_statement { yyerror( "ERRORE SINTATTICO: un argomento del costrutto FOR non è corretto" ); }
 | T_SWITCH '(' expr ')' { gen_switch( f_ptr, Exp ); clear( ); } switch_case_list { gen_tab( f_ptr, ntab ); ntab--; fprintf( f_ptr, "}\n" ); }
 | T_SWITCH expr ')' switch_case_list { yyerror( "ERRORE SINTATTICO: '(' mancante nel costrutto SWITCH" ); }
 | T_BREAK ';' { fprintf( f_ptr, "break;\n" ); }
 | T_CONTINUE ';' { fprintf( f_ptr, "continue;\n" ); }
 | T_ECHO echo_expr_list ';' 
  { gen_echo( f_ptr, Exp, Phrase ); clear( );
 //Stampa gli avvisi se notice è uguale a 5 ( avviso riservato proprio alla funzione echo ).
 if( notice == 5 ) {
  printf( "\033[01;33mRiga %i. %s\033[00m", yylineno, warn[ notice ] );
  _warning++;
  notice = -1;
  }
 }
 | T_ECHO error ';' { yyerror ( "ERRORE SINTATTICO: argomento della funzione ECHO errato" ); }
 | expr ';' {
 if( countelements( Exp ) == 1 )
 print_expression( f_ptr, Exp );
 clear( );
 fprintf( f_ptr,";\n" );
 //Stampa gli avvisi se notice è diverso da -1 e da 5 ( 5 è un avviso riservato alla funzione echo ).
 if( notice != -1 && notice != 5 ) {
  printf( "\033[01;33mRiga %i. %s\033[00m", yylineno, warn[ notice ] );
  _warning++;
  notice = -1;
  }
  }
 | ';' /* empty statement */
 | end
 ;

 for_statement:
 { ntab++; } statement { ntab--; }
 ;

 switch_case_list:
 '{' { ntab++; } case_list '}'
 ;

 /* La funzione print_expression ( presente nel file symbol_table.h ) stampa un'espressione
direttamente nel file f_out.c. */
 case_list:
 /* empty */
 | case_list T_CASE { ntab++; gen_tab( f_ptr, ntab ); fprintf( f_ptr, "case " ); }
expr { print_expression( f_ptr, Exp ); clear( ); } case_separator { fprintf( f_ptr, "\n" ); }
inner_statement_list { ntab--; }
 | case_list T_DEFAULT { ntab++; gen_tab( f_ptr, ntab ); fprintf( f_ptr, "default" ); } case_separator { fprintf( f_ptr, "\n" ); } inner_statement_list { ntab--; }
 ;

 case_separator:
 ':' { fprintf( f_ptr, ":" ); }
 | ';' { fprintf( f_ptr, ";" ); }
 ;

 while_statement:
 { gen_tab( f_ptr, ntab ); ntab++; } statement { ntab--; }
 ;

 elseif_list:
 /* empty */ { fprintf( f_ptr, "\n" ); gen_tab( f_ptr, ntab ); ntab--; fprintf( f_ptr, "}" ); }
 | elseif_list T_ELSEIF '(' expr ')' { gen_elseif( f_ptr, Exp ); clear( ); ntab++; }
statement { fprintf( f_ptr, "\n" ); gen_tab( f_ptr, ntab ); ntab--; fprintf( f_ptr, "}" ); }
 ;

 else_single:
 /* empty */ { fprintf( f_ptr, "\n" ); }
 | T_ELSE { fprintf( f_ptr, " else {\n" ); ntab++; } statement { fprintf( f_ptr,
"\n" ); gen_tab( f_ptr, ntab ); ntab--; fprintf( f_ptr, "}\n" ); }
 ;

 /* Tale sezione amministra l'espressione associata all'istruzione PHP di stampa echo.
 Per le variabili e gli elementi di un array, in sola lettura, e le costanti è richiamata la
funzione echo_check ( presente nel file symbol_table.h ) al fine di controllare l'esistenza
delle stesse nella symbol table.
 In caso di elementi di un array sarà anche controllato l'offset associato. */
 echo_expr_list:
 '"' encaps_list '"'
 | T_CONSTANT { echo_check( $1, 0, yylineno ); }
 | T_CONSTANT_ENCAPSED_STRING { put_testo( &Exp, $1 ); }
 | r_variable { echo_check( $1, index_element, yylineno ); }
 ;

 for_expr:
 /* empty */
 | expr
 ;

 /* Tale regola definisce la creazione delle espressioni associate ai vari costrutti del PHP.
 La prima parte definisce le regola di assegnazione a una variabile o ad un elemento di un array,
mentre la seconda parte definisce le varie operazioni matematiche o logiche. L'ultima parte
amministra l'operatore condizionale ternario <condizione> ? <istruzione1> : <istruzione2> e
l'istanza di array ( T_ARRAY ). Nelle operazioni di assegnazione si assume la variabile
sinistra, in sola scrittura ( read = false ), mentre, per determinati operatori come ++ o --, la
variabile associata si assume come in sola lettura ( read = true ). */
 expr_without_variable:
 T_CONSTANT '=' expr { isconstant( $1, yylineno ); yyerror( "ERRORE SEMANTICO: non è consentito assegnare un valore a una costante" ); }
 | w_variable '=' expr {
 if( array ) {
 /*nel caso si stia definendo un array ( un
particolare caso di assegnazione, possibile solo con l'operatore = ) è effettuato un
type_checking sull'omogeneità degli elementi dell'array. Se il controllo ha esito positivo
l'istruzione è stampata nel file f_out.c e l'array viene aggiunto nella Symbol table.*/
 type_array_checking( T, 'c', NULL, yylineno );
 gen_create_array( f_ptr, $1, current_type, Exp );
 add_element( $1, "array", current_type, NULL, dim, yylineno );
 array = false;
 } else {
 /*nel caso si stia definendo una variabile è
effettuato un controllo sulla lista concatenata T ( Tipi ). Se il numero di elementi è pari a
uno, allora è un'assegnazione semplice quindi è conservato il valore di current_value;
altrimenti il valore è impostato a zero. L'istruzione di assegnazione è stampata nel file
f_out.c e la variabile è aggiunta nella Symbol table.*/
 countelements( T ) > 1 ? current_value = "0" :
current_value;
 gen_assignment( f_ptr, 0, $1, current_type, index_element, Exp, false );
 add_element( $1, "variable", current_type, current_value, 0, yylineno );
 }

 clear( );
 }
 | element_array '=' {
 /*nel caso si voglia assegnare un valore a un elemento di un array, per
prima cosa viene controllato l'indice dell'elemento, mediante la funzione check_index
( contenuta nel file symbol_table.h ).*/
 fprintf( f_ptr, "%s[%s]", $1, index_element ); check_index( $1,
index_element, yylineno ); } expr {
 /*successivamente mediante la funzione check_element ( posta nel
medesimo file ) si effettua un type_checking. Se i controlli hanno esito positivo, l'istruzione
di assegnazione sarà stampata nel file f_out.c .*/
 check_element( $1, index_element, yylineno, false );
 gen_assignment( f_ptr, 0, $1, current_type, index_element, Exp, true );
 clear( );
 }
 | error '=' expr { yyerror("ERRORE SINTATTICO: parte sinistra dell'espressione non riconosciuta"); }
 | w_variable T_PLUS_EQUAL expr {
 countelements( T ) > 1 ? current_value = "0" : current_value;
 gen_assignment( f_ptr, 1, $1, current_type, index_element, Exp, false );
 add_element( $1, "variable", current_type, current_value, dim, yylineno );
 clear( );
 }
 | element_array T_PLUS_EQUAL { fprintf( f_ptr, "%s[%s]", $1, index_element );
check_index( $1, index_element, yylineno ); } expr {
 check_element( $1, index_element, yylineno, false );
 gen_assignment( f_ptr, 1, $1, current_type, index_element, Exp, true );
 clear( );
 }
 | w_variable T_MINUS_EQUAL expr {
 countelements( T ) > 1 ? current_value = "0" : current_value;
 gen_assignment( f_ptr, 2, $1, current_type, index_element, Exp, false );
 add_element( $1, "variable", current_type, current_value, dim, yylineno );
 clear( );
 }
 | element_array T_MINUS_EQUAL { fprintf( f_ptr, "%s[%s]", $1, index_element );
check_index( $1, index_element, yylineno ); } expr {
 check_element( $1, index_element, yylineno, false );
 gen_assignment( f_ptr, 2, $1, current_type, index_element, Exp, true );
 clear( );
 }
 | w_variable T_MUL_EQUAL expr {
 countelements( T ) > 1 ? current_value = "0" : current_value;
 gen_assignment( f_ptr, 3, $1, current_type, index_element, Exp, false );
 add_element( $1, "variable", current_type, current_value, dim, yylineno );
 clear( );
 }
 | element_array T_MUL_EQUAL { fprintf( f_ptr, "%s[%s]", $1, index_element ); check_index( $1, index_element, yylineno ); } expr {
 check_element( $1, index_element, yylineno, false );
 gen_assignment( f_ptr, 3, $1, current_type, index_element, Exp, true );
 clear( );
 }
 | w_variable T_DIV_EQUAL expr {
 countelements( T ) > 1 ? current_value = "0" : current_value;
 gen_assignment( f_ptr, 4, $1, current_type, index_element, Exp, false );
 add_element( $1, "variable", current_type, current_value, dim, yylineno );
 clear( );
 }
 | element_array T_DIV_EQUAL { fprintf( f_ptr, "%s[%s]", $1, index_element ); check_index( $1, index_element, yylineno ); } expr {
 check_element( $1, index_element, yylineno, false );
 gen_assignment( f_ptr, 4, $1, current_type, index_element, Exp, true );
 clear( );
 }
 | w_variable T_MOD_EQUAL expr {
 countelements( T ) > 1 ? current_value = "0" : current_value;
 gen_assignment( f_ptr, 5, $1, current_type, index_element, Exp, false );
 add_element( $1, "variable", current_type, current_value, dim, yylineno );
 clear( );
 }
 | element_array T_MOD_EQUAL { fprintf( f_ptr, "%s[%s]", $1, index_element ); check_index( $1, index_element, yylineno ); } expr {
 check_element( $1, index_element, yylineno, false );
 gen_assignment( f_ptr, 5, $1, current_type, index_element, Exp, true );
 clear( );
 }
 /*di seguito, solo per gli operatori matematici verrà eseguito un controllo dei tipi
con, eventualmente, visualizzazione di warnings.*/
 | variable { check( $1, index_element, yylineno, true ); } T_INC { put_testo( &Exp, "++" ); type_checking( T, yylineno ); print_expression( f_ptr, Exp ); }
 | T_INC { put_testo( &Exp, "++" ); } variable { check( $3, index_element, yylineno, true ); type_checking( T, yylineno ); print_expression( f_ptr, Exp ); }
 | variable { check( $1, index_element, yylineno, true ); } T_DEC { put_testo( &Exp, "--" ); type_checking( T, yylineno ); print_expression( f_ptr, Exp ); }
 | T_DEC { put_testo( &Exp, "--" ); } variable { check( $3, index_element, yylineno, true ); type_checking( T, yylineno ); print_expression( f_ptr, Exp ); }
 | expr T_BOOLEAN_OR { put_testo( &Exp, " || " ); } expr
 | expr T_BOOLEAN_OR { yyerror( "ERRORE SINTATTICO: (||) secondo termine dell'espressione mancante" ); }
 | expr T_BOOLEAN_AND { put_testo( &Exp, " && " ); } expr
 | expr T_BOOLEAN_AND { yyerror( "ERRORE SINTATTICO: (&&) secondo termine dell'espressione mancante" ); }
 | expr T_LOGICAL_OR { put_testo( &Exp, " OR " ); } expr
 | expr T_LOGICAL_OR { yyerror( "ERRORE SINTATTICO: (OR) secondo termine dell'espressione mancante" ); }
 | expr T_LOGICAL_AND { put_testo( &Exp, " AND " ); } expr
 | expr T_LOGICAL_AND { yyerror( "ERRORE SINTATTICO: (AND) secondo termine dell'espressione mancante" ); }
 | expr T_IS_EQUAL { put_testo( &Exp, " == " ); } expr
 | expr T_IS_EQUAL { yyerror( "ERRORE SINTATTICO: (==) secondo termine dell'espressione mancante" ); }
 | expr T_IS_NOT_EQUAL { put_testo( &Exp, " != " ); } expr
 | expr T_IS_NOT_EQUAL { yyerror( "ERRORE SINTATTICO: (!=) secondo termine dell'espressione mancante" ); }
 | expr '<' { put_testo( &Exp, " < " ); } expr { current_type = type_checking( T, yylineno ); }
 | expr '<' { yyerror( "ERRORE SINTATTICO: (<) secondo termine dell'espressione mancante" ); }
 | expr T_IS_SMALLER_OR_EQUAL { put_testo( &Exp, " <= " ); } expr { current_type = type_checking( T, yylineno ); }
 | expr T_IS_SMALLER_OR_EQUAL { yyerror( "ERRORE SINTATTICO: (<=) secondo termine dell'espressione mancante" ); }
 | expr '>' { put_testo( &Exp, " > " ); } expr { current_type = type_checking( T,yylineno ); }
 | expr '>' { yyerror( "ERRORE SINTATTICO: (>) secondo termine dell'espressione mancante" ); }
 | expr T_IS_GREATER_OR_EQUAL { put_testo( &Exp, " >= " ); } expr { current_type = type_checking( T, yylineno ); }
 | expr T_IS_GREATER_OR_EQUAL { yyerror( "ERRORE SINTATTICO: (>=) secondo termine dell'espressione mancante" ); }
 | expr '+' { put_testo( &Exp, " + " ); } expr { current_type = type_checking( T, yylineno ); }
 | expr '+' { yyerror( "ERRORE SINTATTICO: (+) secondo termine dell'espressione mancante" ); }
 | expr '-' { put_testo( &Exp, " - " ); } expr { current_type = type_checking( T, yylineno ); }
 | expr '-' { yyerror( "ERRORE SINTATTICO: (-) secondo termine dell'espressione mancante" ); }
 | expr '*' { put_testo( &Exp, " * " ); } expr { current_type = type_checking( T, yylineno ); }
 | expr '*' { yyerror( "ERRORE SINTATTICO: (*) secondo termine dell'espressione mancante" ); }
 | expr '/' { put_testo( &Exp, " / " ); } expr { current_type = type_checking( T, yylineno ); }
 | expr '/' { yyerror( "ERRORE SINTATTICO: (/) secondo termine dell'espressione mancante" ); }
 | expr '%' { put_testo( &Exp, " % " ); } expr { current_type = type_checking( T, yylineno ); }
 | expr '%' { yyerror( "ERRORE SINTATTICO: (%) secondo termine dell'espressione mancante" ); }
 | '(' { put_testo( &Exp, "(" ); } expr ')' { put_testo( &Exp, ")" ); }
 | expr '?' expr ':' expr
 | scalar
 //tale flag discrimina se l'assegnazione a una variabile sia in realtà la definizione diun array.
 | T_ARRAY { array = true; dim = 0; } '(' array_pair_list ')'
 ;

 /* Tale regola definisce i vari numeri, interi o reali, stringhe ( racchiuse fra singoli apici
'' o doppi apici "" ) e le stringhe ( senza apici ), potenziali costanti predefinite del PHP o
definite dall'utente. Si noti che se viene letto un valore in scrittura ( assegnazione ), esso e
il suo tipo sono memorizzati nelle variabili current_value e cuttent_type. Inoltre se è in
acquisizione un array viene incrementata la sua dimensione. Sia in caso di scrittura che di
lettura il valore e il tipo sono rispettivamente aggiunti nelle liste T ( Tipi, per il controllo
dei tipi mediante le funzioni type_checking e type_array_checking del file symbol_table.h ) e
Exp ( Espressione, per la generazione di espressioni mediante le funzioni gen_expression,
gen_echo_expression o print_expression del file gen_code.h ). */
 common_scalar:
 T_LNUMBER {
 if( !read ) {
 current_value = $1; current_type = "int";
 if( array || index_element != NULL ) {
 dim++;
 }
 }
 put_testo( &T, "int" );
 //Se è un numero negativo, inserisce nella lista testo il numero racchiuso fra parentesi tonde
 char *c = strndup($1, 1);
 if( strcmp( c, "-" ) == 0) {
 c = ( char * )malloc( ( strlen( $1 ) + 3 ) * sizeof( char ) );
 strcpy(c, "(");
 strcat(c, $1);
 strcat(c, ")");
 put_testo( &Exp, c );
 free(c);
 } else
 put_testo( &Exp, $1 );
 }
 | T_DNUMBER {
 if( !read ) {
 current_value = $1; current_type = "float";
 if( array || index_element != NULL ) {
 dim++;
 }
 }

 put_testo( &T, "float" );
 //Se è un numero negativo, inserisce nella lista testo il numero racchiuso fra parentesi tonde
 char *c = strndup($1, 1);
 if( strcmp( c, "-" ) == 0) {
 c = ( char * )malloc( ( strlen( $1 ) + 3 ) * sizeof( char ) );
 strcpy(c, "(");
 strcat(c, $1);
 strcat(c, ")");
 put_testo( &Exp, c );
 free(c);
 } else
 put_testo( &Exp, $1 );
 }
 | T_CONSTANT_ENCAPSED_STRING {
 if( !read ) {
 current_value = $1; current_type = "char *";
 if( array || index_element != NULL ) {
 dim++;
 }
 }

 put_testo( &T, "char *" );
 put_testo( &Exp, $1 );
 }
 | T_STRING {
 current_type = isconstant($1, yylineno);
 if( !read ) {
 current_value = $1;
 if( array || index_element != NULL ) {
 dim++;
 }
 }

 put_testo( &T, current_type );
 put_testo( &Exp, $1 );
 }
 ;

 scalar:
 common_scalar
 | '"' encaps_list '"'
 | '\'' encaps_list '\''
 ;

 possible_comma:
 /* empty */
 | ','
 ;

 /* Tale regola definisce le variabile, gli elementi di un array, in sola lettura, o le costanti
utilizzate in una espressione o in una parte destra di una qualsiasi operazione di assegnazione,
semplice o complessa o in un costrutto. Per ogni elemento in sola lettura è effettuato un
controllo di esistenza nella Symbol table: se il controllo ha esito positivo, il nome
dell'elemento e il suo tipo sono inseriti rispettivamente nella lista T ( Tipi ) e Exp
( Espressione ). */
 expr:
 r_variable { check( $1, index_element, yylineno, true ); }
 | T_CONSTANT { check( $1, 0, yylineno, true ); }
 | expr_without_variable
 ;

 r_variable:
 variable { $$ = $1; read = true; }
 ;

 w_variable:
 variable { $$ = $1; read = false; }
 ;

 variable:
 T_VARIABLE { $$ = $1; }
 | element_array
 ;

 element_array:
 T_VARIABLE '[' T_LNUMBER ']' { $$ = $1; index_element = $3; }
 | T_VARIABLE '[' T_VARIABLE ']' { $$ = $1; index_element = $3; }
 ;

 array_pair_list:
 /* empty */
 | non_empty_array_pair_list possible_comma
 ;

 non_empty_array_pair_list:
 non_empty_array_pair_list ',' { put_testo( &Exp, ", " ); } scalar
 | scalar
 ;

 /* Tale regola definisce quali debbano essere gli elementi presenti nell'espressione
dell'istruzione PHP di stampa echo. Solamente nel caso di variabili, elementi di un array o di
costanti ( si veda la regola encaps_var ) viene richiamata la funzione echo_check ( del file
symbol_table.h ) che controlla l'esistenza, nella Symbol table o nella Constant table, nel caso
delle costanti, dei suddetti elementi. Se il controllo ha esito positivo, il nome dell'elemento
è inserito nella lista Exp ( Espressione ), mentre l'eventuale frase o stringa, nella lista
Phrase ( Frase ). */
 encaps_list:
 encaps_list encaps_var { echo_check( $2, index_element, yylineno ); }
 | encaps_list T_STRING { strcat( $2, " " ); put_testo( &Phrase, $2 ); }
 | encaps_list T_NUM_STRING { put_testo( &Phrase, $2 ); }
 | encaps_list T_ENCAPSED_AND_WHITESPACE { put_testo( &Phrase, $2 ); }
 | /* empty */
 ;

 encaps_var:
 T_VARIABLE { $$=$1; index_element = 0; }
 | T_VARIABLE '[' T_NUM_STRING ']' { $$=$1; index_element = $3; }
 | T_VARIABLE '[' T_VARIABLE ']' { $$=$1; index_element = $3; }
 | T_CONSTANT { $$=$1; index_element = 0; }
 ;


 %%


 /* La funzione pulizia, in caso di errore semantico fatale o per altri errori che comportino
l'interruzione della compilazione, chiude ed elimina il file di output f_out.c . */
 void pulizia( ) {
 chiudi_cancella_file( f_ptr );
 }

 /* La funzione yyerror stampa un messaggio di errore.
 L'argomento è:
 - s, la stringa contenente il messaggio di errore. */
 void yyerror( const char *s ) {
 _error++;
 printf( "\033[01;31mRiga %i. %s.\033[00m\n", yylineno, s );
 }

 /* La funzione main avvia il processo di compilazione e traduzione mediante la funzione interna
yyparse.
 Gli argomenti sono:
 - argc, il numero di argomenti passati da riga di comando;
 - argv, i nomi dei file passati da riga di comando, contenenti il codice sorgente in PHP
da compilare e tradurre in C. */

 main( int argc, char *argv[] )
 {
 extern FILE *yyin;
 ++argv; --argc;
 int i;

 for( i = 0; i < argc; i++ ) {
 printf( "Analisi del file %d: %s\n\n", i + 1, argv[ i ] );
 if( fopen( argv[ i ], "r" ) != NULL )
 yyin = fopen( argv[ i ], "r" );
 else {
 printf( "\033[01;31mERRORE: file %s non trovato.\033[00m\n", argv[ i ] );
 exit( 0 );
 }
 //yydebug = 1;
 yyparse( );
 printf( "\n" );
 }

 if( _error == 0 && _warning == 0 ) {
 print_elements();
 printf( "\n\n\033[01;32mParsing riuscito.\033[00m\n" );
 } else if ( _error == 0 && _warning != 0 ) {
 print_elements();
 printf( "\n\n\033[01;33mParsing riuscito, ma sono presenti %i warnings.\033[00m\n", _warning );
 } else {
 printf( "\n\n\033[01;31mParsing fallito:\033[00m" );
 if( _error > 1 )
 printf( "\033[01;31m sono stati rilevati %i errori. \033[00m\n", _error );
 else
 printf( "\033[01;31m è stato rilevato %i errore. \033[00m\n", _error );

 cancella_file( );
 }
 }