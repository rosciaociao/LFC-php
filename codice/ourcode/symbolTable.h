/**
+-----------------------------------------------------------------------+
| P2C -- symbolTable.h 							|
+-----------------------------------------------------------------------+
|									|
|  Autori: 	Vito Manghisi 						|
| 		Gianluca Grasso						|
+-----------------------------------------------------------------------+
*	
* Sorgente di gestione della symbol table per il traduttore da PHP a C
*
*/

/* Questo file contiene un insieme di funzioni utili alla gestione delle variabili, costanti ed
elementi di un array al fine di consentire una corretta applicazione delle regole semantiche. */

#include <stdio.h> 	/** Per la stampa degli errori semantici */
#include <stdlib.h> 	/** Per allocare memoria */
#include <string.h> 	/** Per manipolare le stringhe nella tabella dei simboli */
#include "inclusioni.h" /** Funzioni e tipi di supporto al processo di traduzione. */
#include "uthash.h" 	/** gestione degli indici HASH per la gestione dei record della symbol table. */

#define NUM_CONSTANTS 3 /** Indica la dimensione della tabella delle costanti predefinite nel linguaggio PHP. */
#define NUM_WARNINGS 6 /** Indica la dimensione dell'array warn contenente tutti i messaggi di warning previsti. */

/** Quando viene sollevato un warning, questo indice viene settato ad un
numero appropriato. */
int notice = -1;

/** Tale array contiene alcuni dei possibili messaggi di warning che il compilatore potrebbe
sollevare. L'accesso a uno specifico elemento è effettuato nel parser mediante l'indice notice.
*/
char* warn[ NUM_WARNINGS ] = { "ATTENZIONE: l'uso di un operando di tipo stringa non è corretto nel linguaggio target C.\n",
                               "ATTENZIONE: l'uso di un operando di tipo boolean non è corretto nel linguaggio target C.\n",
                               "ATTENZIONE: l'uso di un operando di tipo intero non è corretto nel linguaggio target C.\n",
                               "ATTENZIONE: l'uso di un operando di tipo float non è corretto nel linguaggio target C.\n",
                               "ATTENZIONE: l'uso di un elemento con offset negativo o superiore alla dimensione dell'array potrebbe causare problemi nel linguaggio target C.\n",  
			       "ATTENZIONE: la stampa di un elemento con offset negativo o superiore alla dimensione dell'array potrebbe causare problemi nel linguaggio target C.\n"
                             };

/** Definizione della struttura della tabella delle costanti predefinite del linguaggio PHP. */
typedef struct CONSTANTS_TABLE
{
    char *ctM; //parola maiuscola.
    char *ctm; //parola minuscola.
} CONSTANTS_TABLE;

/** La tabella delle costanti */
CONSTANTS_TABLE const_tab[ NUM_CONSTANTS ] = {
    { "TRUE","true" },
    { "FALSE","false" },
    { "NULL","null" }
};

/** Definizione della struttura della Symbol Table. Può memorizzare variabili, costanti o array. */
typedef struct {
    char *nomeToken; /**nome del token e chiave per UtHash (garantita univocità) */
    char *tipoToken; /**indica se è una "variable" o una "constant" o un "array" */
    char *type; /** il tipo primitivo tra int, float, char * o bool.*/
    char *value; /** valore variabile o zero per assegnazioni complesse o NULL per gli array.*/
    int dim; /** dimensione per un array */
    UT_hash_handle hh; // Maniglia per Uthash
} symbolTableEntry;

typedef symbolTableEntry *symbolTablePointer; // Puntatore all'elemento.
symbolTablePointer table = NULL; // Puntatore alla tabella.

/** Definizione della struttura della Symbol Table per le funioni */
typedef struct {
    char *nomeFunzione; /**nome del token funzione, univoco per usare Uthash */
    int numeroParam; /** il numero dei parametri */
    /**char **nomiParam;  L'elenco nomui dei parametri */
    /**char **tipiParam;  L'elenco dei tipi dei parametri */
    char *tipoRitorno; /** Il tipo di ritorno della funione */
    char *nomeRitorno; /** Nome per la variabile di ritorno */
    char *scope; /** Lo scope di definizione della funzione */
    symbolTablePointer sf; /** Symbol table locale alla funzione */
    UT_hash_handle hh; // Maniglia per Uthash
} functionSymbolTableEntry;

typedef functionSymbolTableEntry *functionSymbolTablePointer; // Puntatore all'elemento.
functionSymbolTablePointer functionTable = NULL; // Puntatore alla tabella.

/* Stampa a video gli elementi della Symbol table */
void stampaSymbolTable(){
    symbolTablePointer s = table;    
    unsigned int num_entry;
    num_entry = HASH_COUNT(s);
    if(num_entry){
      stampaMsg("\n#################### Inizio della SYMBOL TABLE ####################\n\n","purple");
      for ( ; s != NULL; s = s->hh.next ) {
	  printf( "%s element name %s type %s",  s->tipoToken, s->nomeToken, s->type);
	  if(strcmp(s->tipoToken,"array")==0){
	    printf( " dim %i\n", s->dim );
	  }
	  else if(strcmp(s->tipoToken,"variable")==0)
	  {
	    printf(" value %s\n", s->value);
	  }
      }
    stampaMsg("\n####################  Fine della Symbol Table  ####################\n","purple");
    }else
       stampaMsg("\n[INFO] La Symbol Table è vuota\n","purple");
}

void stampaFunctionsSymbolTable(){
    functionSymbolTablePointer s = functionTable;    
    unsigned int num_entry, symbols;
    num_entry = HASH_COUNT(s);
    if(num_entry){
      stampaMsg("\n#################### FUNCTION SYMBOL TABLE ####################\n\n","blue");
      for ( ; s != NULL; s = s->hh.next ) {
	  printf("Nome funzione: %s, numero parametri %d, tipo ritorno %s, scope: %s\n",  s->nomeFunzione, s->numeroParam, s->tipoRitorno, s->scope);
	  symbolTablePointer st = s->sf;
	  symbols = HASH_COUNT(st);
	  stampaMsg("\nNum Parametri trovati: ","yellow");
	  stampaMsg(itoa(symbols),"yellow");
	  if(symbols){
	    stampaMsg("\n\t#################### SYMBOL TABLE per la Funzione ####################\n\n","green");
	    for ( ; st != NULL; st = st->hh.next ) {
	      if(strcmp(st->tipoToken,"parametro")==0)
		printf( "\tTipo token: %s, nome %s\n",  st->tipoToken, st->nomeToken);
	      else
		printf( "\tTipo token %s, nome %s, tipo %s\n",  st->tipoToken, st->nomeToken, st->type);
	      /*if(strcmp(st->tipoToken,"array")==0){
		printf( " dim %i\n", st->dim );
	      }
	      else if(strcmp(st->tipoToken,"variable")==0)
	      {
		printf(" value %s\n", st->value);
	      }*/
	    }
	     stampaMsg("\n\t#################### FINE ST per la Funzione ####################\n\n","green");
	  }else
	    stampaMsg("\n[INFO] La Symbol Table dei parametri della funzione è vuota\n","blue");
      }
    stampaMsg("\n################## END FUNCTION SYMBOL TABLE ##################\n","blue");
    }else
       stampaMsg("\n[INFO] La Symbol Table per le funzioni è vuota\n","blue");
}

/** La funzione findElement trova e restituisce un elemento della Symbol table. L'argomento è:
- nomeToken, il nome del simbolo da cercare.
Restituisce l'elemento, se trovato, o NULL. */
symbolTablePointer findElement( char *nomeToken ) {
    symbolTablePointer s;
    if(lastFunction!= NULL){
      HASH_FIND_STR( functionTable->sf, nomeToken, s );
    }else{
      HASH_FIND_STR( table, nomeToken, s );
    }
    
    return s;
}

/** La funzione findElement trova e restituisce un elemento della Symbol table per le funzioni. L'argomento è:
- nomeToken, il nome del simbolo da cercare.
Restituisce l'elemento, se trovato, o NULL. */
functionSymbolTablePointer findFunctionElement( char *nomeFunzione ) {
    functionSymbolTablePointer s;
    HASH_FIND_STR( functionTable, nomeFunzione, s );
    return s;
}

/** La funzione deleteElement rimuove un elemento dalla Symbol table. L'argomento è:
- s, il punatore all'elemento da rimuovere. */
void deleteElement( symbolTablePointer s ) {
    HASH_DEL( table, s );
    free(s);
}

/** La funzione deleteElement rimuove un elemento dalla Symbol table. L'argomento è:
- s, il punatore all'elemento da rimuovere. */
void deleteFuncionElement( functionSymbolTablePointer s ) {
    HASH_DEL( functionTable, s );
    free(s);
}


/** La funzione add_element aggiunge un nuovo elemento nella Symbol table. Gli argomento sono:
- nomeToken, il nome del simbolo da aggiungere;
- tipoToken, il tipo di elemento ossia "constant", "variable" o "array";
- current_type, il tipo "int", "float", "char *" o "bool";
- current_value, il valore assegnato alla variabile: in particolare esso è zero se
l'assegnazione è complessa o NULL se è un array;
- dim, la dimensione dell'array, altrimenti varrà zero;
- nr, il numero riga segnalato dalla variabile yylineno di Flex. */
void add_element( char *nomeToken, char *tipoToken, char *current_type, char *current_value, int
                  dim, int nr ) {
    symbolTablePointer s;
    symbolTablePointer exist = findElement( nomeToken );
    
//se non esiste inserisce l'elemento nella Symbol table.
    if ( !exist ) {
        s = malloc( sizeof( symbolTableEntry ) );
        s->nomeToken = nomeToken;
        s->tipoToken = tipoToken;
        s->type = current_type;
        s->value = current_value;
        s->dim = dim;
	if(lastFunction!= NULL)	  
	  HASH_ADD_KEYPTR( hh, functionTable->sf, nomeToken, strlen(nomeToken), s );
	else
	  HASH_ADD_KEYPTR( hh, table, s->nomeToken, strlen( s->nomeToken ), s );
	
        /*se si tenta di inserire una costante viene sollevato un errore fatale, che blocca la
        compilazione. Si sta cercando di ridefinire una costante.*/
    } else if ( strcmp( exist->tipoToken, "constant" ) == 0 ) {
        printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: ridefinizione di una costante.\033[00m\n", nr);
        printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
//altrimenti è una riassegnazione di valori a una variabile: occorre verificare che la riassegnazione non violi il tipo precedentemente definito.
    } else {
//in caso di non violazione verrà aggiornato il valore della variabile.
        if ( strcmp( exist->type, current_type ) == 0 ) {
            //deleteElement( exist );
            exist->value = current_value;
	    // manca HASH_DEL
            //HASH_ADD_KEYPTR( hh, table, exist->nomeToken, strlen( exist->nomeToken ), exist );
//altrimenti sarà lanciato un errore semantico fatale.
        } else {
            printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'assegnazione viola il tipo primitivo della variabile.\033[00m\n", nr);
            printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
        }
    }
}

//add functionTable element

void addFunctionElement( char *nomeFunzione, char *scope, int nr ) {
    functionSymbolTablePointer s;
    functionSymbolTablePointer exist = findFunctionElement( nomeFunzione );
    symbolTableEntry sf;

//se non esiste inserisce l'elemento nella Symbol table funzioni.
    if ( !exist ) {
        s = malloc( sizeof( functionSymbolTableEntry ) );
        s->nomeFunzione = nomeFunzione;
	s->scope = strdup(scope);
	s->numeroParam=0;
	//sf = malloc( sizeof( symbolTableEntry ) );
        HASH_ADD_KEYPTR( hh, functionTable, s->nomeFunzione, strlen( s->nomeFunzione ), s );
    }else{      
      stampaMsg("\n[ERRORE FATALE]: Non è possibile ridichiarare una funzione\n","red");
      stampaMsg("Riga: ","red");
      stampaMsg(itoa(nr), "red");
      stampaMsg("\n", "red");
    }
}

void addElementInFunctionSymbolTable(char * nomeFunzione, char * nomeToken, char * tipoToken, int nr){
  
    functionSymbolTablePointer s;
    functionSymbolTablePointer exist = findFunctionElement( nomeFunzione );
    
    if ( exist ) {
//stampaMsg("\nTrovata la funzione nella ST funzioni\n","azure");
      symbolTablePointer sf;
      HASH_FIND_STR( exist->sf, nomeToken, sf );
     if ( !sf ) { 
       sf = malloc( sizeof( symbolTableEntry ) );
       sf->nomeToken = nomeToken;
       sf->tipoToken = tipoToken;
       sf->type= "int";
       sf->value= "0";
       sf->dim = 0;
       exist->numeroParam+=1;
       HASH_ADD_KEYPTR( hh, exist->sf, nomeToken, strlen( nomeToken ), sf );
     }
     else{      
      stampaMsg("\n[ERRORE FATALE]: Si sta ridefinendo un parametro già esistente.\n","red");
      stampaMsg("Riga: ","red");
      stampaMsg(itoa(nr), "red");
      stampaMsg("\n", "red");
    }
    }else{      
      stampaMsg("\n[ERRORE FATALE]: Si vuole inserire un parametro in una funzione inesistente.\n","red");
      stampaMsg("Riga: ","red");
      stampaMsg(itoa(nr), "red");
      stampaMsg("\n", "red");
    }
    
  
}

/** La funzione addFunctionElement aggiunge un nuovo elemento nella Symbol table per le funzioni.
- nomeFunzione, il nome del simbolo da aggiungere;
...
- nr, il numero riga segnalato dalla variabile yylineno di Flex.
void addFunctionElement( char *nomeFunzione, int numParam, char **nomiParam, char **tipoParam,  char *tipoRitorno, char *scope, int nr ) {
    functionSymbolTablePointer s;
    functionSymbolTablePointer exist = findFunctionElement( nomeFunzione );

//se non esiste inserisce l'elemento nella Symbol table funzioni.
    if ( !exist ) {
        s = malloc( sizeof( functionSymbolTableEntry ) );
        s->nomeFunzione = nomeFunzione;
        s->numeroParam = numParam;
	s->nomiParam = (char**) malloc(sizeof(char*)*numParam);
	int i = -1;
	while( ++i < numParam){
	  //s->nomiParam[i] = (char *)malloc(sizeof(char) * strlen(*nomiParam[i]));
	  s->nomiParam[i] = (char*) strdup(nomiParam[i]);
	}
	s->tipiParam = (char**) malloc(sizeof(char*)*numParam);
	i = -1;
	while( ++i < numParam){
	  //s->nomiParam[i] = (char *)malloc(sizeof(char) * strlen(*nomiParam[i]));
	  s->tipiParam[i] = (char*) malloc(sizeof(char)*5);
	}
        s->tipoRitorno = strdup(tipoRitorno);
	s->scope = strdup(scope);
        HASH_ADD_KEYPTR( hh, functionTable, s->nomeFunzione, strlen( s->nomeFunzione ), s );
    }else{      
      stampaMsg("\n[ERRORE FATALE]: Non è possibile ridichiarare una funzione\n","red");
      stampaMsg("Riga: ","red");
      stampaMsg(itoa(nr), "red");
      stampaMsg("\n", "red");
    }
}
 */

/** La funzione type_checking controlla se le varie operazioni matematiche di +, -, *, /, %, ++,
--, <, <=, >, >= abbiano come operandi variabili intere ( int ) o reali ( float ). In caso di
utilizzo di variabili booleane o di stringhe vengono generati dei warning mediante
l'assegnazione di un opportuno valore alla variabile notice.
Gli argomento sono:
- TT, una lista definita che contiene tutti i tipi associati agli operandi;
- nr, il numero riga segnalato dalla variabile yylineno di Flex.
Restituisce il tipo da assegnare alla variabile da aggiungere o aggiornare ( solo il valore )
nella Symbol table. */
char *type_checking( listaStringhe *TT, int nr )
{
    listaStringhe *punt = TT;
    char *tipo;
    char *current_type = "int";

//stampa_lista( TT, "TIPE" );

    while ( punt != NULL ) {

        if ( strcmp( punt->stringa, "float" ) == 0 ) {
            current_type = "float";
        }

        if ( strcmp( punt->stringa, "char *" ) == 0 ) {
            notice = 0;
//break;
        }
        if ( strcmp(punt->stringa, "bool" ) == 0 ) {
            notice = 1;
//break;
        }
        punt = punt->next;
    }

    return current_type;
}

/** La funzione type_array_checking controlla se:
- le varie operazioni di creazione di un array siano di tipo omogeneo;
- l'assegnazione a un elemento dell'array abbia come operandi variabili omogenee e di
tipo identico a quello dell'array dichiarato nella Symbol table.
Gli argomento sono:
- TT, una lista definita in utility.h che contiene tutti i tipi associati agli operandi;
- context, indica il constringato di utilizzo di tale funzione:
-> 'c' indica "create" ovvero la creazione di un array. La funzione con tale
constringato è richiamata da un'azione semantica di una regola del parser;
-> 's' indica "single" ossia quando è in atto un'assegnazione di un solo valore
a un elemento di array;
-> 'm' indica "multiple" ossia quando è in atto un'assegnazione multipla a un
elemento di un array.
Le funzioni con constringato 's' e 'm' sono richiamate dalla funzione check_element
spiegata più avanti;
- exist, l'elemento ricercato precedentemente nella funzione check_element ( è l'array di
appartenenza dell'elemento );
- nr, il numero riga segnalato dalla variabile yylineno di Flex. */
void type_array_checking( listaStringhe *TT, char context, symbolTablePointer exist, int nr ) {
    listaStringhe *punt = TT;
    char *tipo;

//stampa_lista( TT, "TIPI" );

    switch ( context ) {
//Controlla la creazione degli array: omogeneità dei tipi
    case 'c':
        if ( TT != NULL )
            tipo = punt->stringa;
        while ( TT != NULL ) {
            if ( strcmp( TT->stringa, tipo ) != 0 )
            {
                printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'uso di un operando di tipo non omogeneo in un array tipizzato non è corretto nel linguaggio target C.\033[00m\n", nr );
                printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
            }
            TT = TT->next;
        }
//stampa_lista( TT );
        break;
//assegnazione singola
    case 's':
        if ( strcmp( exist->type, TT->stringa ) != 0 ) {
            printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'assegnazione viola l'omogeneità degli elementi dell'array \"%s\".\033[00m\n", nr, exist->nomeToken );
            printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
        }
        break;
//assegnazione multipla ( espressioni )
    case 'm':
        tipo = exist->type;
        while ( TT != NULL ) {
            if ( strcmp( TT->stringa, tipo ) != 0 )
            {
                printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'assegnazione viola l'omogeneità degli elementi dell'array \"%s\".\033[00m\n", nr, exist->nomeToken );
                printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
            }
            TT = TT->next;
        }
        break;
    }
}

/** La funzione check_element_gen_code controlla se l'elemento indicato esiste o meno nella
Symbol table. Tale funzione è utilizzata nella traduzione delle istruzione di assegnazione
( funzioni gen_create_array e gen_assignment dichiarate nel file gen_code.h ) :
- se si assegna un valore a una variabile nuova e, quindi non presente in tabella, si
riporta anche il tipo associato;
- altrimenti se si riassegna un valore a una variabile già esistente in tabella, non
si riporta il tipo associato.
L'unico argomento è:
- nomeToken, il nome del simbolo da cercare.
Restituisce 1 se l'elemento è stato trovato altrimenti 0. */
int check_element_gen_code( char *nomeToken ) {
    if ( findElement( nomeToken ) != NULL )
        return 1;
    return 0;
}

/** La funzione check_index è richiamata in caso di assegnazione di un valore a un elemento di un
array ( in fase di scrittura, quindi ) al fine di controllare se l'elemento, e il suo indice ( o
offset ), siano validi. Essa controlla se l'elemento indicato esista nella Symbol table e, in
caso affermativo, effettua dei controlli sull'offset:
-> se è un numero, converte il contenuto della stringa nel corrispondente valore intero;
-> se è una variabile si accerta della sua esistenza e, nel caso esista, converte il
contenuto della stringa nel corrispondente valore intero;
-> un ultimo controllo è dedicato al valore intero dell'offset. Se è minore di zero o
maggiore della dimensione massima dichiarata nella Symbol table, è lanciato un
warning.
Gli argomento sono:
- nomeToken, il nome dell'elemento dell'array;
- offset, l'indice dell'elemento;
- nr, il numero riga segnalato dalla variabile yylineno di Flex. */
void check_index( char *nomeToken, char *offset, int nr ) {
    int index;
    symbolTablePointer exist = findElement( nomeToken );
    symbolTablePointer exist_index;
//se l'elemento non esiste lancia un errore semantico fatale.
    if ( !exist ) {
        printf( "\033[01;31m\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, nomeToken );
        printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
    } else {
//mediante la funzione isnumeric contenuta nel file utility.h ci si accerta che l'offset sia un numero
        if ( isnumeric( offset ) ) {
//in caso affermativo la funzione atoi converte il contenuto della stringa ( in tal caso un numero ) nel corrispondente valore intero.
            index = atoi
                    ( offset );
        } else {
//in caso contrario l'offset è una variabile. Occorre controllare che esista mediante la funzione findElement.
            exist_index = findElement( offset );

//se esiste
            if ( exist_index ) {
                /*controlla che un indice non sia intero poichè è ammissibile,
                quindi viene effettuato un controllo sulla variabile offset. Se il tipo è diverso da "int" viene
                visualizzato un errore semantico fatale.*/
                if ( strcmp( exist_index->type, "int" ) != 0 ) {
                    printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'uso di un elemento con offset non intero non è ammissibile.\033[00m\n", nr);
                    printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
                }
//la funzione atoi converte il valore della variabile nel corrispondente valore intero.
                index = atoi( exist_index->value );
            } else {
//se non esiste viene visualizzato un errore semantico fatale.
                printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, offset );
                printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
            }
        }
        /*se l'indice è minore di 0 o maggiore della dimensione specificata nella Symbol
        table viene visualizzato un messaggio di warning, assegnano alla variabile notice un valore
        appropriato.*/
        if ( index < 0 || index >= exist->dim ) {
            notice = 4;
        }
    }
}

/** La funzione check_element è richiamata per controllare l'esistenza delle variabili o degli
elementi di un array, in sola lettura, o delle costanti, ossia tutte quelle variabili
utilizzate nelle operazioni:
- composizione di un espressione, anche complessa, di assegnazione;
- costrutti tipo, if, for ( condizione ), switch, while e do-while;
- operatori ++ o -- ( solo per le variabili ).
Tale funzione ha molte cose in comune con la precedente, ma viene richiamata in molti punti
differenti della grammatica:
-> nel caso di lettura di una variabile, di un elemento di un array o di una costante,
operando di un'espressione o di un costrutto, la funzione è richiamata con valore
read = true. Dopo il controllo positivo nella Symbol table, il tipo associato verrà
inserito nella lista T ( Tipi ).
-> nel caso delle assegnazioni di espressioni a un elemento di un array, essa è
richiamata dopo la creazione dell'espressione di assegnazione con valore read = false.
Poichè l'espressione è stata generata avremo a disposizione la lista T, e in base al
numero di elementi presenti verrà chiamata la funzione type_array_checking nel
constringato
's' o 'm' per un controllo sull'omogeneità dei tipi.
Gli argomento sono:
- nomeToken, il nome della variabile o dell'elemento di un array;
- offset, l'indice dell'elemento dell'array;
- nr, il numero riga segnalato dalla variabile yylineno di Flex;
- read, specifica se l'elemento è analizzato in lettura o scrittura ( solo per elementi
di
un array ).
Restituisce l'elemento identificato dal nomeToken, trovato nella Symbol table. */
symbolTablePointer check_element( char *nomeToken, char *offset, int nr, bool read )
{
    int index;
    char *tipo;
    char *el_array;

    symbolTablePointer exist = findElement( nomeToken );
    symbolTablePointer exist_index;
//se l'elemento non esiste lancia un errore semantico fatale.
    if ( !exist ) {
        printf( "\033[01;31m\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, nomeToken );
        printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
    } else {
//se è una variabile di sola lettura o una costante lo aggiungo alle liste T ed Exp ( Espressione ).
        if ( ( ( strcmp( exist->tipoToken, "variable" ) == 0 ) || ( strcmp( exist->tipoToken, "constant" ) == 0 ) ) && read ) {
//aggiungo il tipo dell'elemento alla lista T.
            ins_in_lista( &listaTipi, exist->type );
//aggiungo il nome dell'elemento alla lista Exp.
            ins_in_lista( &espressioni, exist->nomeToken );
        } else {
//se è un elemento di un array di sola lettura ( non è un assegnazione a se sstringaso ) lo aggiungo alle liste T ed Exp ( Espressione ).
            if ( read ) {
//aggiungo il tipo dell'elemento alla lista T.
                ins_in_lista( &listaTipi, exist->type );
//viene legato al nome dell'array ( identificato dal nomeToken ) l'offset: nome[offset]. Successivamente aggiungo il nome composto alla lista Exp.
                el_array = nomeToken;
                strcat(el_array, "[");
                strcat(el_array, offset);
                strcat(el_array, "]");
                strcat(el_array, "\0");
                ins_in_lista( &espressioni, el_array );
            }
//mediante la funzione isnumeric contenuta nel file utility.h ci si accerta che l'offset sia un numero
            if ( isnumeric( offset ) ) {
//in caso affermativo la funzione atoi converte il contenuto della stringa ( in tal caso un numero ) nel corrispondente valore intero.
                index = atoi( offset );
            } else {
//in caso contrario l'offset è una variabile. Occorre controllare che esista mediante la funzione findElement.
                exist_index = findElement( offset );

//se esiste.
                if ( exist_index ) {
                    /*controlla che un indice non sia intero poichè è
                    ammissibile, quindi viene effettuato un controllo sulla variabile offset. Se il tipo è diverso
                    da "int" viene visualizzato un errore semantico fatale.*/
                    if ( strcmp( exist_index->type, "int" ) != 0 ) {
                        printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'uso di un elemento con offset non intero non è ammissibile.\033[00m\n", nr);
                        printf( "\n\n\033[01;31mParsing fallito.\033[00m \n" );
                    }
//la funzione atoi converte il valore della variabile nel corrispondente valore intero.
                    index = atoi( exist_index->value );
                } else {
//se non esiste viene visualizzato un errore semantico fatale.
                    printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, offset );
                    printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
                }
            }

            /*se l'indice è minore di 0 o maggiore della dimensione specificata
            nella Symbol table viene visualizzato un messaggio di warning, assegnando alla variabile notice
            un valore appropriato.*/
            if ( index < 0 || index >= exist->dim ) {
                notice = 4;
            }

//stampa_lista( T, "TIPI" );

            /*se la lista T non è vuota e se sto assegnando un valore all'elemento
            di un array ( scrittura, read = false ), sulla base del numero di elementi nella lista T
            effettuo un controllo sull'omogeneità dei tipi, secondo il constringato 's' o 'm'.*/
            if ( countelements( listaTipi ) != 0 && !read ) {

                switch ( countelements( listaTipi ) ) {
                case 1:
                    type_array_checking( listaTipi, 's', exist, nr );
                    break;

                default:
                    type_array_checking( listaTipi, 'm', exist, nr );
                    break;
                }
            }
        }
    }

    return exist;
}

/** La funzione echo_check ha il compito di verificare l'esistenza nella ST del token passato come argomento.
 * Se il token è un array allora viene controllato l'offset corrispondente all'elemento da controllare.
 * In base al tipo di variabile vengono valorizzare le liste relative alle espressioni o alle frasi. 
 * Argomenti: 
 * - nomeToken: il nome della variabile, dell' array o di una costante;
 * - offset: l'indice dell'elemento nell'array, nel caso degli array;
 * - nr: il numero riga prelevato da Flex. */
void echo_check( char *nomeToken, char *offset, int nr )
{
    int index;
    char *tmp = " ) ? \"true\" : \"false\""; //variabile utilizzata per gestire la traduzione della stampa di valori booleani.
    char *el_array;
    symbolTablePointer exist = NULL;
    if(inFunction == true){
      ins_in_lista( &espressioni, nomeToken ); 
    }
    else{
      exist = findElement( nomeToken );
      symbolTablePointer exist_index;
  //se l'elemento non esiste lancia un errore semantico fatale.
      if ( !exist ) {
	  printf( "\033[01;31m\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, nomeToken );
	  printf( "\033[01;31mParsing fallito.\033[00m\n" );
      } else {
  //se l'elemento esiste e se è un array vengono effettuati gli sstringasi controlli sull'offset.
	  if ( strcmp( exist->tipoToken, "array" ) == 0 ) {
	      if ( offset == NULL ) {
		  printf( "\033[01;31m\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: offset non definito.\033[00m\n", nr );
		  printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
	      }
	      if ( isnumeric( offset ) ) {
		  index = atoi( offset );
	      } else {
		  exist_index = findElement( offset );
  //se esiste.
		  if ( exist_index ) {
		      /*controlla che un indice non sia intero poichè è
		      inammissibile. Quindi viene effettuato un controllo sulla variabile offset. Se il tipo è
		      diverso da "int" viene visualizzato un errore semantico fatale.*/
		      if ( strcmp( exist_index->type, "int" ) != 0 ) {
			  printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'uso di un elemento con offset non intero non è ammissibile.\033[00m\n", nr);
			  printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
		      }
  //la funzione atoi converte il valore della variabile nel corrispondente valore intero.
		      index = atoi( exist_index->value );
		  } else {
		      printf( "\033[01;31m\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, nomeToken );
		      printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
		  }
	      }
  //l'offset viene legato al nome dell'array ( identificato dal nomeToken ): nome[offset].
	      el_array = nomeToken;
	      strcat( el_array, "[" );
	      strcat( el_array, offset );
	      strcat( el_array, "]" );
	      strcat( el_array, "\0" );
	      /*e il tipo dell'elemento è booleano viene aggiunta alla lista Exp la
	      variabile tmp. In entrambi i casi è aggiunta alla lista Exp l'elemento dell'array.*/
	      if ( strcmp( exist->type, "bool" ) == 0 ) {
		  char *c = ( char * )malloc( ( strlen( el_array ) + strlen( tmp ) + 1 ) * sizeof( char ) );
		  strcpy( c, "( " );
		  strcat( c, el_array );
		  strcat( c, tmp );
		  ins_in_lista( &espressioni, c );
		  free( c );
	      } else
		  ins_in_lista( &espressioni, el_array );

	      if ( index < 0 || index >= exist->dim ) {
		  notice = 5;
	      }
	  } else {
	      /*nel caso in cui l'elemento sia una variabile o una costante: se il
	      tipo dell'elemento è booleano viene aggiunta alla lista Exp la variabile tmp. In entrambi i casi
	      è aggiunta alla lista Exp la variabile o la costante.*/
	      if ( strcmp( exist->type, "bool" ) == 0 ) {
		  char *c = ( char * )malloc( ( strlen( nomeToken ) + strlen
						( tmp ) + 1 ) * sizeof( char ) );
		  strcpy( c, "( " );
		  strcat( c, nomeToken );
		  strcat( c, tmp );
		  ins_in_lista( &espressioni, c );
		  free( c );
	      } else
		  ins_in_lista( &espressioni, nomeToken);
	  }

	  /*in base al tipo dell'elemento viene aggiunto alla lista stringato il giusto
	  identificatore di variabili utilizzato dal C nella funzione printf.*/
	  if ( strcmp( exist->type, "int" ) == 0 )
	      ins_in_lista( &frasi, " %i " );
	  else if ( strcmp( exist->type, "float" ) == 0 )
	      ins_in_lista( &frasi, " %f " );
	  else
	      ins_in_lista( &frasi, " %s " );

      }
    }
}

/** La funzione isconstant verifica se la stringa letta ( identificata dal token T_STRING ) sia
una costante predefinita del PHP o se sia una costante definita dall'utente mediante la
funzione define.
Gli argomento sono:
- string, la stringa da controllare;
- nr, il numero riga segnalato dalla variabile yylineno di Flex.
Restituisce il tipo della costante per effettuare i controlli di tipo ( type_checking ). */
char *isconstant( char *string, int nr )
{
    int i;
    int trov = 0;
    char *current_type = "bool";
// 1° controllo nella tabella delle costanti predefinite
    for ( i = 0; i < NUM_CONSTANTS; i++ )
    {
        if ( strcmp( string, const_tab[i].ctM ) == 0 )
            trov = 1;
        if ( strcmp( string, const_tab[i].ctm ) == 0 )
            trov = 1;
    }
//solo se non è una costante predefinita effettuo un ulteriore controllo.
    if ( trov == 0 ) {
// 2° controllo nella symbol table.
        symbolTablePointer exist = findElement( string );

        if ( exist && ( strcmp( exist->tipoToken, "constant" ) == 0) ) {
            current_type = exist->type;
            trov = 1;
        }
    }
//se non è una costante viene lanciato un errore semantico fatale.
    if ( trov == 0 ) {
        printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: stringa \"%s\" non riconosciuta.\033[00m\r\n", nr, string );
        printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
    }

    return current_type;
}


void printDeclarationFunctionHeader(char * nomeFunzione){
  functionSymbolTablePointer f;
  f = findFunctionElement(nomeFunzione);
  fprintf(f_ptr, "#define %s(",f->nomeFunzione);
  int i=0;
  if(f->numeroParam!=0){
    symbolTablePointer s;
    s = f->sf;
    for ( ; i < f->numeroParam; s = s->hh.next ,i++ ) {
      fprintf(f_ptr,"%s",s->nomeToken);
      if(i != (f->numeroParam - 1) )
	fprintf(f_ptr,",");
    }    
  }
  fprintf(f_ptr,") {\t\t\\\n");
  //iniziano gli innner statement  
}


void printFunctionCallHeader(char * nomeFunzione, int nr){
  functionSymbolTablePointer f;
  f = findFunctionElement(nomeFunzione);
  
  if(f==NULL){
    stampaMsg("\n[ERRORE SEMANTICO]: Riga ","red");
    stampaMsg(itoa(nr),"yellow");
    stampaMsg(", La funzione ", "red");    
    stampaMsg(nomeFunzione,"yellow");
    stampaMsg(" richiamata non esiste!\n","red");    
  }
  else
  {
    fprintf(f_ptr,"%s(",f->nomeFunzione);
    
    
//     if(numPassedParam != f->numeroParam )
//     {
//       stampaMsg("\n[ERRORE SEMANTICO]: Riga ","red");
//     stampaMsg(itoa(nr),"yellow");
//     stampaMsg(", il numero dei parametri nella chiamata alla funzione ", "red");    
//     stampaMsg(nomeFunzione,"yellow");
//     stampaMsg(" è errato!\n","red");   
//     }
    
    
    
  }
  
}

void printReturnStatement(char * nomeFunzione){
 functionSymbolTablePointer f = findFunctionElement(nomeFunzione);
 
 
 
}