/**
+-----------------------------------------------------------------------+
| P2C -- inclusioni.h 							|
+-----------------------------------------------------------------------+
|									|
|  Autori: 	Vito Manghisi 						|
| 		Gianluca Grasso						|
+-----------------------------------------------------------------------+
*	
* Sorgente di gestione della symbol table per il traduttore da PHP a C
*
*/

#include <stdio.h> 	/** Per la gestione dei messaggi di errore semantico */
#include <string.h> 	/** Per la manipolazione delle stringhe */
#include <ctype.h>	/** Per la funzione isdigit */

/** Nome del file in uscita */
char * fout;

/** Puntatore al file che conterrà la traduzione in linguaggio C. */ 
 FILE *f_ptr;

/** Puntatore al file di log */
 FILE *log_file;
 
/** Struttura per il tipo di dato booleano  */
typedef enum { false, true } bool; 

 bool inFunction = false;
 
 char * lastFunction;


/** Flag per abilitare il logging su file */
bool logging;

/** Puntatori allo standard input ed error utili per il ripristono in console dopo il logging su file */
FILE *tmpStderr, *tmpStdout;

/** Procedura che inserirsce nel file passato un newline o un backslash */
void insertNewLine(FILE* f_ptr);

/** Nuovo tipo di dato "listaStringhe" come lista concatenata di stringhe. */
typedef struct listaStringhe 
{
    char * stringa;
    struct listaStringhe * next;
} listaStringhe;

/** 	Lista concatenata contenente i tipi delle variabili, elementi di array o costanti. 
 * 	Utile per il type_checking.
 */
listaStringhe *listaTipi;

/** 	Lista concatenata contenente gli elementi di una espressione 
 * 	(variabili, elementi di array, costanti, operatori).
 * 	Utile per  stampare intere espressioni nel file traduzione.c 
 */
listaStringhe *espressioni;

/**	Lista concatenata contenente frasi o parole ( non keywords ). 
 * 	Utile per stampare intere frasi, anche associate a espressioni, nel file tradotto in C . 
 */
listaStringhe *frasi;

/** Funzione che cancella il contenuto delle tre liste di supporto. Utile quando è
 *  necessario resettare le liste ogni volta che il parser valida una frase. 
 */
void liberaStrutture( ) {
    listaTipi = NULL;
    espressioni = NULL;
    frasi = NULL;
}

/** Dichiarazione della funzione che inserisce una stringa nel file di log del parser */
void logThis(const char *);

/** Funzione che stampa a console una stringa passata come parametro in rosso 
 *	Parametri: 	str = stringa da stampare
 * 			col = colore per stampa a console, tra red, green, blue, purple, yellow
 */
void stampaMsg(const char * str, const char * col){
  if(logging == false){
    if(strcmp(col,"blue") == 0)
      printf("\033[01;34m");
    else if(strcmp(col,"azure") == 0)
      printf("\033[01;36m");
    else if(strcmp(col,"yellow") == 0)
      printf("\033[01;33m");
    else if(strcmp(col,"green") == 0)
      printf("\033[01;32m");
    else if(strcmp(col,"purple") == 0)
      printf("\033[01;35m");
    else if(strcmp(col,"white") == 0)
      printf("\033[01;37m");
    else if(strcmp(col,"none") == 0)
      printf("\033[0m");
    else
      printf("\033[01;31m");  
  }
  printf("%s",str);
  if(logging == false)
    printf("\033[0m"); 
}


/** 	ins_in_lista inserisce una stringa in una lista concatenata
 *  	Argomenti
 * 	- pointer, doppio puntatore alla lista;
 * 	- str, nuova stringa da inserire.
 */

void ins_in_lista( listaStringhe **pointer ,char *stringa )
{
    listaStringhe *punt, *copia;
    // allocazione memoria per il nuovo elemento
    copia = ( listaStringhe * )malloc( sizeof( listaStringhe ) );
    copia->stringa = ( char * )strdup( stringa );
    copia->next = NULL;
    
    if ( *pointer == NULL )
        *pointer = copia; 	// lista vuota, inserimento in testa
    else			// lista non vuota, inserimento in coda
    {
        punt = *pointer;
        while ( punt->next != NULL )
            punt = punt->next;
        punt->next = copia;
    }
}

/** 	stampa_lista stampa a video tutti i valori di una lista.
 * 	Gli argomenti sono:
 * 	- un puntatore lista;
 * 	- label, una stringa d'utilità per etichettare la stampa
 */
void stampa_lista( listaStringhe *pointer, char *label )
{
    listaStringhe *punt = pointer;

    printf( "#################### Start %s ####################\n\n", label );
    while ( punt != NULL ) {
        printf( "Elemento: %s\n", punt->stringa );
        punt = punt->next;
    }
    printf( "\n################ End %s #################\n\n", label );
}

/**	La funzione countelements conta il numero di elementi presenti in una lista.
 * 	L'argomento è:
 * 	- T, un puntatore alla lista;
 * 
 * 	Restituisce il numero degli elementi. 
 */
int countelements( listaStringhe *T ) {
    listaStringhe *punt = T;
    int value = 0;

    while ( punt != NULL ) {
        value++;
        punt = punt->next;
    }
    return value;
}

/** 	La funzione isnumeric stabilisce se una stringa è costituita da soli numeri,
 * 	ossia se si tratta di un numero o no.
 * 	L'argomento è:
 * 	- str, la stringa da analizzare;
 * 	
 * 	Restituisce 1 se la stringa è un numero, 0 altrimenti. 
 */
int isnumeric( char *str )
{
    char *c = (char *)strndup(str, 1);  //copia del primo carattere
    if ( strcmp( c, "-" ) == 0) {  //se è un "-" si scarta il segno
        c = ( char * )strdup( str + 1 );
        while ( *c )
        {
           if ( !isdigit( *c ) )
                return 0;
            c++;
        }
    } 
    else 
    {
        while ( *str )
        {
            if ( !isdigit( *str ) )
                return 0;
            str++;
        }
    }
    return 1;
}

/** Un array contenente tutti i possibili operatori di assegnazione. */
char *op_name[] = { "=", "+=", "-=", "*=", "/=", "%=" };

/** 	La funzione apri_file apre il file tradotto in C che conterrà il 
 * 	codice intermedio C tradotto.
 * 	
 * 	Restituisce il puntatore al file. 
 */
FILE *apri_file() {
    FILE *f_ptr;
    if ( ( f_ptr = fopen( fout, "w" ) ) == NULL ) {
	stampaMsg("ERRORE: apertura del file tradotto in C fallita.", "red");
        exit( 0 );
    }
    return f_ptr;
}

/** La funzione chiudiOutputFile chiude il file traduzione.c precedentemente aperto. */
void chiudiOutputFile( FILE *f_ptr ) {
//forza l'effettiva scrittura di eventuali dati presenti nel buffer, prima della chiusura.
    fflush( f_ptr );
//chiude il file.
    fclose( f_ptr );
}

/** 	La funzione eliminaOutputFile, in caso di errore semantico o grave,
 *	chiude ed elimina il file di traduzione
 * 	L'argomento è:
 * 	- f_ptr, il puntatore al file da chiudere. 
 */
void eliminaOutputFile( FILE *f_ptr ) {
    if ( f_ptr != NULL )
        chiudiOutputFile( f_ptr );
    if ( remove( fout ) == -1 )
        stampaMsg("[ERRORE]: impossibile eliminare il file di traduzione", "red");
}

/** 	La funzione eliminaFile elimina il file di traduzione. Essa è richiamata 
 * 	ad ogni inizio compilazione, in modo tale da poter ricreare un nuovo file. 
 */
void eliminaFile( ) {
    FILE *f_ptr;
//solo se il file esiste sarà cancellato.
    if ( ( f_ptr = fopen( fout, "r" ) ) ) {
        fclose( f_ptr );
        if ( remove( fout ) == -1 )
            stampaMsg("[ERRORE]: impossibile eliminare il file di traduzione", "red");
    }
}

/** 	La funzione gen_header genera l'header del file C, scrivendo due istruzioni 
 * 	di default stdio e string, il metodo main e la definizione del tipo boolean.
 * 	L'argomento è:
 * 	- f_ptr, il puntatore al file nel quale stampare la traduzione. 
 */
void gen_header( FILE* f_ptr ) {
    fprintf( f_ptr, "#include <stdio.h>\n" );
    fprintf( f_ptr, "#include <string.h>\n\n" );
    fprintf( f_ptr, "void main( void ) {\n" );
    fprintf( f_ptr, "\ttypedef enum { false, true } bool;\n" );
}

/** 	La funzione gen_constant genera le costanti dichiarate.
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file nel quale stampare la traduzione;
 * 	- nameConstant, il nome della costante;
 * 	- type, il tipo della costante;
 * 	- value, il valore della costante. 
 */
void gen_constant( FILE* f_ptr, char *nameConstant, char *type, char *value ) {
//se è una costante di tipo stringa allora devo stampare un array di caratteri.
    if ( strcmp( type, "char *" ) == 0 )
        fprintf( f_ptr, "const char %s[] = %s;\n", nameConstant, value );
    else
        fprintf( f_ptr, "const %s %s = %s;\n", type, nameConstant, value );
}

/** 	La funzione gen_expression genera le espressioni, create da composizioni di variabili,
 * 	elementi di array e costanti legate fra loro dagli operatori previsti dalla grammatica. Tutti
 * 	questi elementi sono stati precedentemente inseriti nella lista espressioni.
 * 	L'argomento è:
 * 	- Exp, la lista contenenti gli elementi delle espressioni.
 * 	
 * 	Restituisce l'espressione. 
 */
char *gen_expression( listaStringhe *Exp ) {
    char *expression = NULL;

//get_listaStringhe( Exp, "ESPR" );
//assegna a expression il primo valore della lista Exp.
    if ( Exp != NULL ) {
        expression = Exp->stringa;
        Exp = Exp->next;
    }
//concatena tutti i successivi elementi della lista.
    while ( Exp != NULL ) {
        strcat( expression, Exp->stringa );
        Exp = Exp->next;
    }
    if ( Exp == NULL ) {
        strcat( expression, "\0" );
    }

    return expression;
}

/**	La funzione genecho_expression è simile alla precedente ma finalizzata alla creazione di
 * 	espressioni per l'istruzione echo.
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file nel quale stampare la traduzione;
 * 	- Exp, la lista contenenti gli elementi delle espressioni.
 * 	Restituisce l'espressione. 
 */
char * gen_echo_expression( FILE* f_ptr, listaStringhe *Exp ) {
    char *expression = NULL;
//se gli elementi sono in numero maggiore di uno è prevista la stampa della ",".
    int elements = countelements( Exp );

//get_listaStringhe( Exp, "GEN ECHO ESPR" );

//assegna a expression il primo valore della lista Exp.
    if ( Exp != NULL ) {
        expression = Exp->stringa;
        Exp = Exp->next;
    }
//concatena tutti i successivi elementi della lista.
    while ( Exp != NULL ) {
        if ( elements > 1 )
            strcat( expression, ", " );
        strcat( expression, Exp->stringa );
        Exp = Exp->next;
    }
    if ( Exp == NULL ) {
        strcat( expression, "\0" );
    }

    return expression;
}

/** 	La funzione print_expression genera le espressioni che stamperà nel file traduzione.c, create da
 * 	composizioni di variabili, elementi di array e costanti legate fra loro dagli operatori previsti
 * 	dalla grammatica. Tutti questi elementi sono stati precedentemente inseriti nella lista Exp.
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file nel quale stampare la traduzione;
 * 	- Exp, la lista contenenti gli elementi delle espressioni. 
 */
void print_expression( FILE* f_ptr, listaStringhe *Exp ) {
    char *expression;

//get_listaStringhe( Exp, "ESPR" );

    expression = Exp->stringa;
    Exp = Exp->next;

    while ( Exp != NULL ) {
        strcat( expression, Exp->stringa );
        Exp = Exp->next;
    }
    strcat(expression, "\0");
    fprintf( f_ptr, "%s", expression );
}

/** 	La funzione gen_create_array stampa la dichiarazione di un array.
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file nel quale stampare la traduzione;
 * 	- name_array, il nome dell'array;
 * 	- type, il tipo dell'array;
 * 	- Exp, la lista contenenti gli elementi delle espressioni. 
 */
void gen_create_array( FILE* f_ptr, char *name_array, char *type, listaStringhe *Exp ) {
    char *expression = gen_expression( Exp );
//Se l'array non è stato definito viene stampato anche il tipo, altrimenti no.
    if ( check_element_gen_code( name_array ) )
        fprintf( f_ptr, "%s[] = { %s }", name_array, expression );
    else
        fprintf( f_ptr, "%s %s[] = { %s }", type, name_array, expression );
}


/** 	La funzione gen_assignment stampa le dichiarazioni di una variabile o di un elemento di un
 * 	array.
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file nel quale stampare la traduzione;
 * 	- index, indice per identificare il tipo di operatore di assegnazione contenuto
 * 	  nell'array op_name;
 * 	- left_var, il nome della variabile o dell'elemento dell'array;
 * 	- type, il tipo dell'array;
 * 	- index_element, l'offset dell'elemento di un array;
 * 	- Exp, la lista contenenti gli elementi delle espressioni. 
 */
void gen_assignment( FILE* f_ptr, int index, char *left_var, char *type, char *index_element, listaStringhe *Exp, bool array ) {
    char *expression = gen_expression( Exp );
//Se è una variabile
    if ( !array ) {
//Se la variabile è stata definita viene stampato anche il tipo, altrimenti no.
        if ( check_element_gen_code( left_var ) )
            fprintf( f_ptr, "%s %s %s", left_var, op_name[ index ], expression );
        else
            fprintf( f_ptr, "%s %s %s %s", type, left_var, op_name[ index ], expression );
    } else {
        fprintf( f_ptr, " %s %s", op_name[index], expression );
    }
}

/** 	La funzione gen_if genera l'istruzione condizionale if.
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file nel quale stampare la traduzione;
 * 	- Exp, la lista contenenti gli elementi delle espressioni. 
 */
void gen_if( FILE* f_ptr, listaStringhe *Exp ) {
    char *expression = gen_expression( Exp );
    fprintf( f_ptr, "if( %s ) {\n", expression );
}

/**	La funzione gen_elseif genera l'istruzione condizionale else if.
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file nel quale stampare la traduzione;
 * 	- Exp, la lista contenenti gli elementi delle espressioni. 
 */
void gen_elseif( FILE* f_ptr, listaStringhe *Exp ) {
    char *expression = gen_expression( Exp );
    fprintf( f_ptr, " else if( %s ) {\n", expression );
}

/** 	La funzione gen_while genera l'istruzione di ciclo while.
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file nel quale stampare la traduzione;
 * 	- Exp, la lista contenenti gli elementi delle espressioni. 
 */
void gen_while( FILE* f_ptr, listaStringhe *Exp ) {
    char *expression = gen_expression( Exp );
    fprintf( f_ptr, "while( %s ) {\n", expression );
}

/**	La funzione gen_switch genera il costrutto switch.
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file nel quale stampare la traduzione;
 * 	- Exp, la lista contenenti gli elementi delle espressioni. 
 */
void gen_switch( FILE* f_ptr, listaStringhe *Exp ) {
    char *expression = gen_expression( Exp );
    fprintf( f_ptr, "switch( %s ) {\n", expression );
}

/** 	La funzione genEcho mappa la funzione di echo PHP in una printf C.
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file di traduzione;
 * 	- expr, la lista di supporto contenente i chunck delle espressioni;
 * 	- frasi, la lista contenenti gli elementi della frase da associare all'espressione. 
 */
void genEcho( FILE* f_ptr, listaStringhe *expr, listaStringhe *frasi ) {
    char * phrase = gen_expression( frasi );
    char * expression = gen_echo_expression( f_ptr, expr );

    if ( phrase != NULL && expression != NULL )
        fprintf( f_ptr, "printf(\"%s\",%s);", phrase, expression );
    else if ( phrase != NULL )
        fprintf( f_ptr, "printf(\"%s\");", phrase );
    else
        fprintf( f_ptr, "printf(%s);", expression );
    insertNewLine(f_ptr);
}

/** Procedura che appende un newline al file in uscita oppure un
 *  backslash se nel contesto di una funzione in uscita
 */ 
void insertNewLine(FILE* f_ptr){
  if(lastFunction != NULL)
    fprintf( f_ptr, "\t\t\\\n" );
  else
    fprintf( f_ptr, "\n" );
}

/** Funzione che inserisce ntab tabulazioni nel file di uscita
 * 	Gli argomenti sono:
 * 	- f_ptr, il puntatore al file nel quale stampare il simbolo;
 * 	- ntab, il numero di stampa del simbolo tab. 
 */
void gen_tab( FILE *f_ptr, int ntab ) {
    int i;
    for (i = 0; i < ntab; i++)
        fprintf( f_ptr, "\t" );
}

/** Procedura per la gestione del file di log. Avvia la scrittura sul file "parselog.log"
 * e redirezione lo standard output e lo standard error verso il file di log, salvando 
 * i riferimenti originali per il ripristino successivo.
 */
void startLog(){
  if( logging == true){
    if ( ( log_file = fopen( "parselog.log", "w" ) ) == NULL )
      stampaMsg("\nINFO_ERR: Fallito avvio scrittura del file di log\n", "red");    
    const char * timestamp = "Log ultima esecuzione di p2c";
    fprintf( log_file, "\n****************** %s *****************\n", timestamp );
    stampaMsg("\nINFO: log abilitato in scrittura nel file parselog.log\n", "green");    
    tmpStdout = stdout; //salvataggio riferimento allo stdout
    stdout = log_file;  //redirect dello standard output al file di log
    tmpStderr = stderr; //salvataggio riferimento allo stderr
    stderr = log_file;  //redirect dello standard errore al file di log    
  }
}

/** Procedura per la gestione del file di log. Termina la scrittura del file
 * e ripristina lo standard output e lo standard error
 */
void stopLog(){
  if(logging == true){
    const char * timestamp = "Termine log di p2c";
    fprintf( log_file, "\n****************** %s *****************\n", timestamp );
    fflush( log_file );
    fclose( log_file );
    stderr = tmpStderr; //ripristino riferimento allo stderr
    stdout = tmpStdout; //ripristino riferimento allo stdout
  }
}

/** Procedura che inserisce una stringa nel log */
void logThis(const char * s){
  if(logging == true){    
     fprintf(log_file, "%s", s );
  }
}

/** Funzione che converte un intero in una stringa con dimensione opportuna */ 
char * itoa(int val){
  int i = 1, count = val;
  while( count > 10 ){
    count /= 10;
    i++;
  }
  char * ret = (char *)malloc(sizeof(char)*i+1);
  sprintf(ret,"%d",val);
  ret[i+1] = '\0';
  return ret;
}
