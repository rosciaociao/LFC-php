%{

  /**
  +---------------------------------------------------------------------+
  | P2C -- parser.y	 						|
  +---------------------------------------------------------------------+
  |									|
  |  Autori: 	Vito Manghisi 						|
  | 		Gianluca Grasso						|
  +---------------------------------------------------------------------+
  *	
  * Input file per Bison, generatore del parser per il traduttore da PHP a C
  *
  */


 /**
 * 	LALR conflitti shift/reduce e come essi sono stati risolti:
 *	
 * 	- 4 conflitti shift/reduce a causa dell'ambiguità pendente fra le regole ( corretta e con
 *	  errori ) del costrutto if. Risolti mediante shift.
 * 	- 2 conflitti shift/reduce a causa dell'ambiguità pendente sui costrutti elseif/else.
 *	Risolti mediante shift.
 * 	- 6 conflitti shift/reduce a causa delle assegnazioni, semplici o in forma compatta, di
 *	  valori a elementi di un array. Risolti mediante shift.
 * 	- 1 conflitto shift/reduce a causa dell'ambiguità pendente fra le due regole ( corretta e
 *	  con errori ) del costrutto for. Risolto mediante shift.
 * 	- 39 conflitti shift/reduce a causa dell'ambiguità ( introdotta con le azioni semantiche
 *	fprintf ) pendente fra tutte le espressioni avente i simboli '(' e ')'. Risolti mediante shift.
 *
 */

 #include "symbolTable.h"
 #define YYDEBUG 1

 char * tempReturnExpr;
 
 int actualNumParams = 0;

 symbolTablePointer element;
 /* Prototipo della funzione yyerror per la  visualizzazione degli errori sintattici. */
 void yyerror( const char *s );
 /* Il numero riga segnalato dalla variabile yylineno di Flex. */
 extern int yylineno;
 /** Abilita il debug di Flex */
 extern int yy_flex_debug;
 /** Nome del file in input a bison */
 extern FILE *yyin;

 /* Il valore corrente di una variabile o costante. */
 char *current_value;
 /* Il tipo corrente di una variabile o costante. */ 
 char *current_type; 
 /* L'offset di un elemento di un array. */
 char *index_element = NULL; 
 /* Contatore di errori, di warnings e di elementi di un array (che definiscono la sua dimensione.) */
 int _error = 0, _warning = 0, dim = 0; 
 /* Contatore dei simboli di tabulazione. */
 int ntab = 0;
 /** Puntatore al file di output del parser definito nel file di inclusioni */
 extern char * fout;

 /* Flag usati per discriminare la dichiarazione di un  array, una variabile in lettura o scrittura. */
 bool array, read; 

 /** Funzione per il controllo di una variabile, costante o elemento di un array.
  *  Gli argomento sono:
  *    - nomeToken, il nome del simbolo da aggiungere;
  *    - offset, l'indice dell'elemento;
  *    - nr, il numero riga segnalato dalla variabile yylineno di Flex;
  *    - read, specifica se l'elemento è analizzato in lettura o scrittura
  *      ( solo per elementi di un array ). 
  */
 void check( char *nomeToken, char *offset, int nr, bool read )
 {
 element = check_element( nomeToken, offset, yylineno, read );
 current_value = element->value;
 }

%}

 //%expect 53

//Associatività degli operatori e dei Token
 %left ','
 %left T_LOGICAL_OR
 %left T_LOGICAL_AND
 %left '=' T_PLUS_EQUAL T_MINUS_EQUAL T_MUL_EQUAL T_DIV_EQUAL T_MOD_EQUAL
 %left '?' ':'
 %left T_BOOLEAN_OR
 %left T_BOOLEAN_AND
 %nonassoc <id> T_IS_EQUAL T_IS_NOT_EQUAL
 %nonassoc '<' T_IS_SMALLER_OR_EQUAL '>' T_IS_GREATER_OR_EQUAL
 %left '+' '-'
 %left '*' '/' '%'
 %right T_INC T_DEC
 %left T_ELSEIF
 %left T_ELSE
 %left ')'
// Definizioni dei TOKEN
 %token <id> T_LNUMBER 				//token per numeri interi
 %token <id> T_SNUMBER				//token per numeri interi con segno
 %token <id> T_DNUMBER 				//token per numeri decimali
 %token <id> T_STRING 				//token per le stringhe
 %token <id> T_VARIABLE 			//token per le variabili
 %token <id> T_CONSTANT 			//token per le costanti
 %token <id> T_NUM_STRING 			//token per numeri in stringhe
 %token <id> T_ENCAPSED_AND_WHITESPACE 		//token per whitespace e metacaratteri nelle stringhe
 %token <id> T_CONSTANT_ENCAPSED_STRING		//token per stringhe costanti
 %token T_IF
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
 %token T_FUNCTION
 %token T_RETURN

 /* L’opzione %union consente di specificare una varietà di tipi di dato che sono usati dalla
variabile yylval in Flex. L'unico tipo specificato è la stringa di caratteri. */
%union{
  char *id;
}

%type <id> variable r_variable w_variable element_array encaps_var common_scalar;

 /*%start start;*/

 /** Abilita il tracciamento delle componenti di una regola per Bison */
%locations
 /** Abilita il reporting verboso degli errori */
%error-verbose
/** Abilita la LookAhead Correction: Bison preso un token dallo scanner sospende le azionie e 
  * inizia una esplorazione delle regole usando una copia dello stack, se incontra uno shift
  * riavvia il parsing, se trova un errore (in verbose) mostra la lista dei token attesi
  */
/*%define parse.lac none*/

%% /* Inizio della sezione Bison dedicata alle Regole semantiche */

/** Produzione relativa all'inizio dello script PHP "<?php". L'azione associata elimina
 * il file di traduzione se questo è stato precedentemente creato da un'altra compilazione,
 * lo apre e crea una intestazione per l'inclusione di due librerie standard C
 */
start:
 T_INIT { eliminaFile(); f_ptr = apri_file(); gen_header( f_ptr ); }
 top_statement_list
 ;

/** Produzione finale. L'azione conclude il main nel file tradotto C. */
end:
 T_FINAL { fprintf( f_ptr, "\n}" ); }
 ;


 top_statement_list:
 top_statement_list top_statement
 | /* empty */
 ;

 top_statement:
 { fprintf( f_ptr, "\t" ); } statement
 | function_declaration_statement
 ;

 inner_statement_list:
 inner_statement_list { fprintf( f_ptr, "\t" ); } inner_statement
 | /* empty */
 ;

 inner_statement:
 { gen_tab( f_ptr, ntab ); } statement
// | function_declaration_statement
 ;

 statement:
 unticked_statement
 ;

 /* Tale regola definisce i costrutti primari e le istruzioni principali del PHP. Solo per le
costanti è utilizzata la funzione add_element ( presente nel file symbolTable.h ) che consente
di aggiungere nuovi elementi nella tabella dei simboli, quando esse sono dichiarate mediante il
token T_DEFINE. Per tutti gli altri costrutti si utilizzano le funzioni fprintf. Nel caso di
espressioni, expr, e dell'istruzione echo, T_ECHO, viene gestita la visualizzazione dei messaggi
di warning. Molto spesso è richiamata la funzione liberaStrutture per svuotare le liste concatenate listaTipi, espressioni
e frasi ( tutte contenute nel file inclusioni.h ). */
 unticked_statement:
 '{' inner_statement_list { fprintf( f_ptr, "\t" ); } '}'
 | T_DEFINE '(' T_CONSTANT ',' common_scalar ')' { liberaStrutture( ); add_element( $3, "constant", current_type, $5, 0, yylineno ); gen_constant( f_ptr, $3, current_type, $5 ); } ';'
 | T_IF '(' expr ')' { gen_if( f_ptr, espressioni ); liberaStrutture( ); ntab++; } statement elseif_list else_single 
 | T_IF '(' error ')' statement elseif_list else_single { yyerror( "ERRORE SINTATTICO: espressione nel costrutto IF non accettata" ); }
 | T_IF expr ')' statement elseif_list else_single { yyerror( "ERRORE SINTATTICO: '(' mancante nel costrutto IF" ); }
 | T_WHILE '(' expr ')' { gen_while( f_ptr, espressioni ); liberaStrutture( ); } while_statement { fprintf( f_ptr, "}\n" ); }
 | T_WHILE '(' error ')' while_statement { yyerror( "ERRORE SINTATTICO: espressione nel costrutto WHILE non accettata" ); }
 | T_WHILE expr ')' while_statement { yyerror( "ERRORE SINTATTICO: '(' mancante nel costrutto WHILE" ); }
 | T_DO { fprintf( f_ptr, "do {\n" ); ntab++; } statement T_WHILE { ntab--; gen_tab( f_ptr, ntab ); fprintf( f_ptr, "} while( " ); } 
    '(' expr ')' { print_expression( f_ptr, espressioni );
      fprintf( f_ptr, " );\n" ); liberaStrutture( ); } ';'
 | T_FOR
    '(' { fprintf( f_ptr, "for( " ); }
    for_expr
    ';' { fprintf( f_ptr, "; " ); }
    for_expr { print_expression( f_ptr, espressioni ); liberaStrutture( ); }
    ';' { fprintf( f_ptr, "; " ); }
    for_expr { liberaStrutture( ); }
    ')' { fprintf( f_ptr, " ) {\n" ); }
    for_statement { gen_tab( f_ptr, ntab); fprintf( f_ptr, "}\n" ); liberaStrutture( ); }
 | T_FOR '(' error ';' error ';' error ')' for_statement { yyerror( "ERRORE SINTATTICO: un argomento del costrutto FOR non è corretto" ); }
 | T_SWITCH '(' expr ')' { gen_switch( f_ptr, espressioni ); liberaStrutture( ); } switch_case_list { gen_tab( f_ptr, ntab ); ntab--; fprintf( f_ptr, "}\n" ); }
 | T_SWITCH expr ')' switch_case_list { yyerror( "ERRORE SINTATTICO: '(' mancante nel costrutto SWITCH" ); }
 | T_BREAK ';' { fprintf( f_ptr, "break;\n" ); }
 | T_CONTINUE ';' { fprintf( f_ptr, "continue;\n" ); }
// return token
 | T_RETURN ';' { fprintf( f_ptr, "return;\n" );}
 | T_RETURN expr ';' { 
   
    // valorizza il nome ed il tipo di ritorno nella entry della symbol table funzioni
    // per la funzione attuale
    
    stampaMsg(lastFunction, "azure");
   
    
    //printReturnStatement();
    
    gen_tab( f_ptr, ntab); 
    fprintf( f_ptr, ";\n",tempReturnExpr ); 
    gen_tab( f_ptr, 1); 
    fprintf( f_ptr, "return %s;\n",tempReturnExpr ); 
   
  }
 | T_ECHO echo_expr_list ';' 
  { genEcho( f_ptr, espressioni, frasi ); liberaStrutture( );
  //Stampa gli avvisi se notice è uguale a 5 ( avviso riservato proprio alla funzione echo ).
  if( notice == 5 ) {
    printf( "\033[01;33mRiga %i. %s\033[00m", yylineno, warn[ notice ] );
    _warning++;
    notice = -1;
    }
  }
 | T_ECHO error ';' { yyerror ( "ERRORE SINTATTICO: argomento della funzione ECHO errato" ); }
 | expr ';' {
 if( countelements( espressioni ) == 1 )
 print_expression( f_ptr, espressioni );
 liberaStrutture( );
 fprintf( f_ptr,";" );
 insertNewLine(f_ptr);
 //Stampa gli avvisi se notice è diverso da -1 e da 5 ( 5 è un avviso riservato alla funzione echo ).
 if( notice != -1 && notice != 5 ) {
  printf( "[WARNING] Riga %i. %s", yylineno, warn[ notice ] );
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

 /* La funzione print_expression ( presente nel file symbolTable.h ) stampa un'espressione
direttamente nel file di uscita della traduzione */
 case_list:
 /* empty */
 | case_list T_CASE { ntab++; gen_tab( f_ptr, ntab ); fprintf( f_ptr, "case " ); }
expr { print_expression( f_ptr, espressioni ); liberaStrutture( ); } case_separator { fprintf( f_ptr, "\n" ); }
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
 | elseif_list T_ELSEIF '(' expr ')' { gen_elseif( f_ptr, espressioni ); liberaStrutture( ); ntab++; }
statement { fprintf( f_ptr, "\n" ); gen_tab( f_ptr, ntab ); ntab--; fprintf( f_ptr, "}" ); }
 ;

 else_single:
 /* empty */ { fprintf( f_ptr, "\n" ); }
 | T_ELSE { fprintf( f_ptr, " else {\n" ); ntab++; } statement { fprintf( f_ptr,
"\n" ); gen_tab( f_ptr, ntab ); ntab--; fprintf( f_ptr, "}\n" ); }
 ;

/** Sezione per la semantica della funzione echo di PHP. 
 *  In caso array controlla l'offset associato. 
 */
 echo_expr_list:
 '"' encaps_list '"'
 | T_CONSTANT { echo_check( $1, 0, yylineno ); }
 | T_CONSTANT_ENCAPSED_STRING { ins_in_lista( &espressioni, $1 ); }
 | r_variable { echo_check( $1, index_element, yylineno ); }
 ;

 for_expr:
 /* empty */
 | expr
 ;

 function_declaration_statement:
		T_FUNCTION { 
		    inFunction = true;
		    //gen_tab(f_ptr, ntab);
		    //fprintf(f_ptr, "#define "); 
		  } 
		  T_STRING { 
		    addFunctionElement( $3, "main", yylineno );
		    lastFunction = strdup($3);
		    //fprintf(f_ptr,"%s(", $3);
		  }
		  '(' parameter_list ')'{  
		      gen_tab(f_ptr, ntab);
		      printDeclarationFunctionHeader(lastFunction);
		      //fprintf(f_ptr,") {"); 
		  } 
		  '{' inner_statement_list {  
		      inFunction = false;
		      //fprintf(f_ptr,"");
		  } 
		  '}' {
		      fprintf(f_ptr,"}\n"); 
		      inFunction = false;		      
		      free(lastFunction);
		      lastFunction = NULL;
		      stampaFunctionsSymbolTable();
		  }
 ;

 parameter_list:
  /* empty */
  | '&' T_VARIABLE { 
      addElementInFunctionSymbolTable(lastFunction, $2, "variable", yylineno);
    }
  | '&' T_VARIABLE { addElementInFunctionSymbolTable(lastFunction, $2, "variable", yylineno); /*fprintf(f_ptr,"--%s--,", $1);*/ }  	  ',' parameter_list
  | T_VARIABLE { 
      addElementInFunctionSymbolTable(lastFunction, $1, "variable", yylineno);
    }
  | T_VARIABLE { addElementInFunctionSymbolTable(lastFunction, $1, "variable", yylineno); /*fprintf(f_ptr,"--%s--,", $1);*/ } ',' parameter_list
 ;

 function_call:
  T_STRING { printFunctionCallHeader($1,yylineno);}
    '('
     function_call_parameter_list
    ')' { 
      fprintf(f_ptr,")");
    }
 ;

function_call_parameter_list:
  /* empty */ 
 | function_call_parameter { actualNumParams +=1; }
 | function_call_parameter { actualNumParams +=1; } ',' {  fprintf(f_ptr,","); } function_call_parameter_list
 ;


function_call_parameter:
  variable { fprintf(f_ptr,"%s",$1); }
 | common_scalar { fprintf(f_ptr,"%s",$1); }
;

/** Regola per le espressioni principali del PHP. */
expr_without_variable:
  T_CONSTANT '=' expr { 
    isconstant( $1, yylineno ); yyerror( "ERRORE SEMANTICO: non è consentito assegnare un valore a una costante" ); 
  }
 | w_variable '=' expr {
    if( array ) {
      type_array_checking( listaTipi, 'c', NULL, yylineno );
      gen_create_array( f_ptr, $1, current_type, espressioni );
      add_element( $1, "array", current_type, NULL, dim, yylineno );
      array = false;
    } else {
      countelements( espressioni ) > 0 ? current_value = espressioni->stringa : current_value;
      gen_assignment( f_ptr, 0, $1, current_type, index_element, espressioni, false );
      add_element( $1, "variable", current_type, current_value, 0, yylineno );
    }
    liberaStrutture();
  }
 | element_array '=' {
    /* Si controlla l'indice dell'elemento nel caso si voglia assegnare un valore a un elemento di un array */
    fprintf( f_ptr, "%s[%s]", $1, index_element ); check_index( $1,
    index_element, yylineno ); } expr {
    /* La chiamata alla funzione check_element effettua un controllo dei tipi.
     * Se il controllo ha esito positivo l'istruzione di assegnazione sarà stampata nel file tradotto */
    check_element( $1, index_element, yylineno, false );
    gen_assignment( f_ptr, 0, $1, current_type, index_element, espressioni, true );
    liberaStrutture( );
  }
 | error '=' expr { yyerror("ERRORE SINTATTICO: parte sinistra dell'espressione non riconosciuta"); }
 | w_variable T_PLUS_EQUAL expr {
    countelements( listaTipi ) > 1 ? current_value = "0" : current_value;
    gen_assignment( f_ptr, 1, $1, current_type, index_element, espressioni, false );
    add_element( $1, "variable", current_type, current_value, dim, yylineno );
    liberaStrutture( );
 }
 | element_array T_PLUS_EQUAL { fprintf( f_ptr, "%s[%s]", $1, index_element );
    check_index( $1, index_element, yylineno ); } expr {
    check_element( $1, index_element, yylineno, false );
    gen_assignment( f_ptr, 1, $1, current_type, index_element, espressioni, true );
    liberaStrutture( );
 }
 | w_variable T_MINUS_EQUAL expr {
    countelements( listaTipi ) > 1 ? current_value = "0" : current_value;
    gen_assignment( f_ptr, 2, $1, current_type, index_element, espressioni, false );
    add_element( $1, "variable", current_type, current_value, dim, yylineno );
    liberaStrutture( );
 }
 | element_array T_MINUS_EQUAL { fprintf( f_ptr, "%s[%s]", $1, index_element );
    check_index( $1, index_element, yylineno ); } expr {
    check_element( $1, index_element, yylineno, false );
    gen_assignment( f_ptr, 2, $1, current_type, index_element, espressioni, true );
    liberaStrutture( );
 }
 | w_variable T_MUL_EQUAL expr {
    countelements( listaTipi ) > 1 ? current_value = "0" : current_value;
    gen_assignment( f_ptr, 3, $1, current_type, index_element, espressioni, false );
    add_element( $1, "variable", current_type, current_value, dim, yylineno );
    liberaStrutture( );
 }
 | element_array T_MUL_EQUAL { fprintf( f_ptr, "%s[%s]", $1, index_element ); check_index( $1, index_element, yylineno ); } expr {
    check_element( $1, index_element, yylineno, false );
    gen_assignment( f_ptr, 3, $1, current_type, index_element, espressioni, true );
    liberaStrutture( );
 }
 | w_variable T_DIV_EQUAL expr {
    countelements( listaTipi ) > 1 ? current_value = "0" : current_value;
    gen_assignment( f_ptr, 4, $1, current_type, index_element, espressioni, false );
    add_element( $1, "variable", current_type, current_value, dim, yylineno );
    liberaStrutture( );
 }
 | element_array T_DIV_EQUAL { fprintf( f_ptr, "%s[%s]", $1, index_element ); check_index( $1, index_element, yylineno ); } expr {
    check_element( $1, index_element, yylineno, false );
    gen_assignment( f_ptr, 4, $1, current_type, index_element, espressioni, true );
    liberaStrutture( );
 }
 | w_variable T_MOD_EQUAL expr {
    countelements( listaTipi ) > 1 ? current_value = "0" : current_value;
    gen_assignment( f_ptr, 5, $1, current_type, index_element, espressioni, false );
    add_element( $1, "variable", current_type, current_value, dim, yylineno );
    liberaStrutture( );
 }
 | element_array T_MOD_EQUAL { fprintf( f_ptr, "%s[%s]", $1, index_element ); check_index( $1, index_element, yylineno ); } expr {
    check_element( $1, index_element, yylineno, false );
    gen_assignment( f_ptr, 5, $1, current_type, index_element, espressioni, true );
    liberaStrutture( );
 }
 | variable T_INC {  
    ins_in_lista(&listaTipi,"int"); 
    check( $1, index_element, yylineno, true ); 
    type_checking( listaTipi, yylineno ); 
    ins_in_lista( &espressioni, "++" );   
 } 
 | T_INC { ins_in_lista( &espressioni, "++" ); 
   } 
   variable { 
     check( $3, index_element, yylineno, true ); 
     type_checking( listaTipi, yylineno ); 
  }
 | variable T_DEC 
    { 
      ins_in_lista(&listaTipi,"int");  
      check( $1, index_element, yylineno, true ); 
      type_checking( listaTipi, yylineno );
      ins_in_lista( &espressioni, "--" ); 
    }
 | T_DEC { 
    ins_in_lista( &espressioni, "--" ); 
    } 
    variable { 
      check( $3, index_element, yylineno, true ); 
      type_checking( listaTipi, yylineno );
    }
 | expr T_BOOLEAN_OR { ins_in_lista( &espressioni, " || " ); } expr
  | expr T_BOOLEAN_OR error { yyerror( "ERRORE SINTATTICO: (||) secondo termine dell'espressione mancante" ); }
// | error T_BOOLEAN_OR expr { yyerror( "ERRORE SINTATTICO: (||) PRIMO termine dell'espressione mancante" ); }
 | expr T_BOOLEAN_AND { ins_in_lista( &espressioni, " && " ); } expr
  | expr T_BOOLEAN_AND error { yyerror( "ERRORE SINTATTICO: (&&) secondo termine dell'espressione mancante" ); } 
 | expr T_LOGICAL_OR { ins_in_lista( &espressioni, " OR " ); } expr
  | expr T_LOGICAL_OR error { yyerror( "ERRORE SINTATTICO: (OR) secondo termine dell'espressione mancante" ); } 
 | expr T_LOGICAL_AND { ins_in_lista( &espressioni, " AND " ); } expr
  | expr T_LOGICAL_AND error { yyerror( "ERRORE SINTATTICO: (AND) secondo termine dell'espressione mancante" ); } 
 | expr T_IS_EQUAL { ins_in_lista( &espressioni, " == " ); } expr
  | expr T_IS_EQUAL error { yyerror( "ERRORE SINTATTICO: (==) secondo termine dell'espressione mancante" ); } 
 | expr T_IS_NOT_EQUAL { ins_in_lista( &espressioni, " != " ); } expr
  | expr T_IS_NOT_EQUAL error { yyerror( "ERRORE SINTATTICO: (!=) secondo termine dell'espressione mancante" ); } 
 | expr '<' { ins_in_lista( &espressioni, " < " ); } expr { current_type = type_checking( listaTipi, yylineno ); }
  | expr '<' error { yyerror( "ERRORE SINTATTICO: (<) secondo termine dell'espressione mancante" ); } 
 | expr T_IS_SMALLER_OR_EQUAL { ins_in_lista( &espressioni, " <= " ); } expr { current_type = type_checking( listaTipi, yylineno ); }
  | expr T_IS_SMALLER_OR_EQUAL error { yyerror( "ERRORE SINTATTICO: (<=) secondo termine dell'espressione mancante" ); }
 | expr '>' { ins_in_lista( &espressioni, " > " ); } expr { current_type = type_checking( listaTipi,yylineno ); }
  | expr '>' error { yyerror( "ERRORE SINTATTICO: (>) secondo termine dell'espressione mancante" ); }
 | expr T_IS_GREATER_OR_EQUAL { ins_in_lista( &espressioni, " >= " ); } expr { current_type = type_checking( listaTipi, yylineno ); }
  | expr T_IS_GREATER_OR_EQUAL error { yyerror( "ERRORE SINTATTICO: (>=) secondo termine dell'espressione mancante" ); }
 | expr '+' { ins_in_lista( &espressioni, " + " ); } expr { current_type = type_checking( listaTipi, yylineno ); }
  | expr '+' error { yyerror( "ERRORE SINTATTICO: (+) secondo termine dell'espressione mancante" ); }
 | expr '-' { ins_in_lista( &espressioni, "-" ); } expr { current_type = type_checking( listaTipi, yylineno ); }
  | expr '-' error { yyerror( "ERRORE SINTATTICO: (-) secondo termine dell'espressione mancante" ); }
 | expr '*' { ins_in_lista( &espressioni, " * " ); } expr { current_type = type_checking( listaTipi, yylineno ); }
  | expr '*' error { yyerror( "ERRORE SINTATTICO: (*) secondo termine dell'espressione mancante" ); }
 | expr '/' { ins_in_lista( &espressioni, " / " ); } expr { current_type = type_checking( listaTipi, yylineno ); }
  | expr '/' error { yyerror( "ERRORE SINTATTICO: (/) secondo termine dell'espressione mancante" ); }
 | expr '%' { ins_in_lista( &espressioni, " % " ); } expr { current_type = type_checking( listaTipi, yylineno ); }
  | expr '%' error { yyerror( "ERRORE SINTATTICO: (%) secondo termine dell'espressione mancante" ); }
 | '(' { ins_in_lista( &espressioni, "(" ); } expr ')' { ins_in_lista( &espressioni, ")" ); }
 | '+' { ins_in_lista(&espressioni,"+"); } expr %prec T_INC 
 | '-' { ins_in_lista(&espressioni,"-"); } expr %prec T_INC 
 | expr '?' expr ':' expr
 | scalar
 | expr common_signed_scalar
 | T_ARRAY { array = true; dim = 0; } '(' array_pair_list ')'
 | function_call 
 ;

 common_signed_scalar:
  T_SNUMBER{
      tempReturnExpr = $1;
      if( !read ) {
	current_value = $1; current_type = "int";
	if( array || index_element != NULL ) {
	  dim++;
	}
      }
      ins_in_lista( &listaTipi, "int" );
      ins_in_lista( &espressioni, $1 );
  }
 ;

 common_scalar:
 T_LNUMBER {
    //tempReturnExpr = $1;
    if( !read ) {
      current_value = $1; current_type = "int";
      if( array || index_element != NULL ) {
	dim++;
      }
    }
    ins_in_lista( &listaTipi, "int" );
    ins_in_lista( &espressioni, $1 );
 }
 | T_DNUMBER {
    //tempReturnExpr = $1;
    if( !read ) {
      current_value = $1; current_type = "double";
      if( array || index_element != NULL ) {
	dim++;
      }
    }
    ins_in_lista( &listaTipi, "double" );
    ins_in_lista( &espressioni, $1 );
  }
 | T_CONSTANT_ENCAPSED_STRING {
    //tempReturnExpr = $1;
    if( !read ) {
      current_value = $1; current_type = "char *";
      if( array || index_element != NULL ) {
	dim++;
      }
    }
    ins_in_lista( &listaTipi, "char *" );
    ins_in_lista( &espressioni, $1 );
 }
 | T_STRING {
    //tempReturnExpr = $1;
    current_type = isconstant($1, yylineno);
    if( !read ) {
      current_value = $1;
      if( array || index_element != NULL ) {
	dim++;
      }
    }
    ins_in_lista( &listaTipi, current_type );
    ins_in_lista( &espressioni, $1 );
 }
 ;

 scalar:
 common_scalar
 | common_signed_scalar
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
dell'elemento e il suo tipo sono inseriti rispettivamente nella lista T ( Tipi ) e espressioni
( Espressione ). */
 expr:
 r_variable { if(inFunction == false) check( $1, index_element, yylineno, true ); }
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
 T_VARIABLE { $$ = $1; /*tempReturnExpr = $1;*/ }
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
 non_empty_array_pair_list ',' { ins_in_lista( &espressioni, ", " ); } scalar
 | scalar
 ;

/** Tale regola definisce li elementi validi per una espressione di echo PHP. 
 *  Nel caso di variabili, array o costanti viene richiamata la funzione echo_check (symbolTable.h )
 *  che controlla l'esistenza del simbolo.
 *  L'inserimento avviene in frasi o espressioni in base al tipo di token
 */
 encaps_list:
 encaps_list encaps_var { echo_check( $2, index_element, yylineno ); }
 | encaps_list T_STRING { strcat( $2, " " ); ins_in_lista( &frasi, $2 ); }
 | encaps_list T_NUM_STRING { ins_in_lista( &frasi, $2 ); }
 | encaps_list T_ENCAPSED_AND_WHITESPACE { ins_in_lista( &frasi, $2 ); }
 | /* empty */
 ;

 encaps_var:
 T_VARIABLE { $$=$1; index_element = 0; }
 | T_VARIABLE '[' T_NUM_STRING ']' { $$=$1; index_element = $3; }
 | T_VARIABLE '[' T_VARIABLE ']' { $$=$1; index_element = $3; }
 | T_CONSTANT { $$=$1; index_element = 0; }
 ;


 %%

 /** Estendione della funzione yyerror di Bison con argomento la stringa passata dal generatore.
  */
 void yyerror( const char *s ) {
    _error++;
    char * lineno = (char *) malloc(sizeof(char) * yylineno);
    sprintf(lineno,"%d",yylineno);
    stampaMsg("[ERRORE] Riga: ","red");
    stampaMsg(lineno,"yellow");
    stampaMsg(". Bison riporta: ","red");
    stampaMsg(s,"red");
    //printf( "\033[01;31mRiga %i. %s.\033[00m\n", yylineno, s );
 }

/** Procedura di avvio del parser per un file in input */
void startParsing(char * nomeFile){  
  // apertura in lettura del file php in input
  if( (yyin = fopen(nomeFile,"r")) != NULL ){
    stampaMsg("Parsing del file: ", "green");
    stampaMsg(nomeFile,"yellow");
    stampaMsg("\n","yellow");
    // impostazione del nome file C in uscita
    fout = (char*) malloc( strlen(nomeFile)-2);	
    strncpy(fout,nomeFile,strlen(nomeFile)-3);
    strncat(fout,"c",1);
    stampaMsg("Nome del file di output: ", "green");
    stampaMsg(fout, "yellow");
    stampaMsg("\n","none");
    // Avvio del parser
    yyparse( ); 
    // Report del processo di parsing
    if( _error == 0 && _warning == 0 ) {
      stampaSymbolTable();
      stampaMsg("\nParsing del file ","green"); 
      stampaMsg(nomeFile,"yellow");
      stampaMsg(" riuscito.\n","green"); 
    } 
    else if ( _error == 0 && _warning != 0 ) {
      stampaSymbolTable();
      stampaMsg("\nParsing del file ","yellow"); 
      stampaMsg(nomeFile,"yellow");
      stampaMsg(" riuscito con Warning n^ ","yellow");
      char * numwarn = (char *) malloc(sizeof(char) * _warning);
      sprintf(numwarn,"%d",_warning);
      stampaMsg(numwarn, "red");
      stampaMsg("\n", "red");
    }
    else {
      stampaMsg("\nE' fallito il parsing del file ","red");
      stampaMsg(nomeFile,"yellow");
      stampaMsg(", con errori num^ ","red");
      char * numerr = (char *) malloc(sizeof(char) * _error);
      sprintf(numerr,"%d",_error);
      stampaMsg(numerr, "red");
      stampaMsg("\n", "red");

     //eliminaFile();

    }
    // inizializzazione strutture interne ed eliminazione della Symbol Table
    liberaStrutture();
    HASH_CLEAR(hh,table);

  }
  else { // in caso non sia stato possibile leggere il file di input stampa un errore
    stampaMsg("\nERRORE, file ","red"); 
    stampaMsg(nomeFile,"yellow");
    stampaMsg(" non trovato.\n", "red");
  }

 

}

 /* La funzione main avvia il processo di compilazione e traduzione mediante la funzione interna
yyparse.
 Gli argomenti sono:
 - argc, il numero di argomenti passati da riga di comando;
 - argv, i nomi dei file passati da riga di comando, contenenti il codice sorgente in PHP
da compilare e tradurre in C. */

int main( int argc, char *argv[] ){ 
 ++argv; --argc; 	// esclusione nome eseguibile dai parametri
 int i;
 logging = false;
 if(argc > 0)
 {
    // forza ad off il debug di flex, per default abilitato
    yy_flex_debug = 0;
    // ciclo per controllare che vi sia almeno un file con ext PHP in input e per impostare le opzioni
    bool inputPHP = false;
    for( i = 0; i < argc; i++ ) 
    {
      if(strstr(argv[i],"php") != NULL)
      {
	  inputPHP = true;
	  continue;
      }
      if(strcmp(argv[i],"-dl") == 0)
      { //imposta il debug per flex se passato il parametro -dl
	yy_flex_debug = 1;
	continue;  
      }
      if(strcmp(argv[i],"-dp") == 0)
      { //imposta il debug per bison se passato il parametro -dp
	yydebug = 1;
	continue;	  
      }
      if(strcmp(argv[i],"-log") == 0)
      { //imposta il debug per bison se passato il parametro -log
	logging = true;
	continue;	  
      }
    }
    startLog();
    if(inputPHP == true)
    {
      for( i = 0; i < argc; i++ ) 
      {
	if(strstr(argv[i],"php") != NULL)
	{	  
	  startParsing(argv[i]); 
	  if( _error > 0)
	  {
	      stopLog();
	      logging = false;
	      stampaMsg("\n\nParsing fallito per uno o più file in input.\n", "yellow");   
	  }
	  chiudiOutputFile(f_ptr);
	}
       }
    }
    else
    {
      stopLog();
      logging = false;
      stampaMsg("\nAttenzione è necessario passare come parametro un file con estensione php.\n\n", "red");
      exit(1);
    }
  stopLog();
  }
  else
  {
      stampaMsg("\nE' necessario passare come parametro almeno un file con estensione php", "red");
      stampaMsg("\nFomato parametri:\n\t\t p2c [-dl|-dp|-log] nomefile.php [secondofile.php...]", "yellow" );
      stampaMsg("\n\t\t\t -dl = abilita il debug dello scanner", "yellow" );
      stampaMsg("\n\t\t\t -dp = abilita il debug del parser", "yellow" );
      stampaMsg("\n\t\t\t -log = abilita il logging nel file parserlog.log\n\n", "yellow" );
      exit(1);
  }
 return 0;
}