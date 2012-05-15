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

/** Firma di una funzione definita nel file php_parser.h: quando si
 verifica un errore semantico il file di traduzione verrà cancellato.
 */
void pulizia( );

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

/** Definizione della struttura della Symbol Table. */
typedef struct {
    char *nameToken; /**nome del token che si inserisce nella Symbol Table. Quesa stringa viene usata come se fosse una chiave.*/
    char *element; /**indica se è una "variable" o una "constant" o un "array".*/
    char *type; /**il tipo primitivo: int, float, char * o bool.*/
    char *value; /**il valore della variabile: pari a un valore, in caso di assegnazione semplice, zero per un'assegnazione complessa o NULL se è un array.*/
    int dim; /**dimensione dell'array.*/

    UT_hash_handle hh; // rende questa struttura indicizzabile mediante HASH.
} el_ST;

typedef el_ST *element_ptr; // Puntatore all'elemento.
element_ptr table = NULL; // Puntatore alla tabella.

/* Stampa a video gli elementi contenuti nella Symbol table. */
void stampaSymbolTable( ) {
    element_ptr s;
    printf("\n\033[01;35m#################### Start of the SYMBOL TABLE ####################\033[00m\n\n");
    for ( s = table; s != NULL; s = s->hh.next ) {
        printf( "%s element name %s type %s",  s->element, s->nameToken, s->type);
	if(strcmp(s->element,"array")==0){
	   printf( " dim %i\n", s->dim );
	}
	else if(strcmp(s->element,"variable")==0)
	{
	  printf(" value %s\n", s->value);
	}
    }
    printf("\n\033[01;35m####################  End of the Symbol Table  ####################\033[00m\n\n");
}

/** La funzione find_element trova e restituisce un elemento della Symbol table. L'argomento è:
- nameToken, il nome del simbolo da cercare.
Restituisce l'elemento, se trovato, o NULL. */
element_ptr find_element( char *nameToken ) {
    element_ptr s;
    HASH_FIND_STR( table, nameToken, s );

    return s;
}

/** La funzione delete_element rimuove un elemento dalla Symbol table. L'argomento è:
- s, il punatore all'elemento da rimuovere. */
void delete_element( element_ptr s ) {
    HASH_DEL( table, s );
}

/** La funzione add_element aggiunge un nuovo elemento nella Symbol table. Gli argomento sono:
- nameToken, il nome del simbolo da aggiungere;
- element, il tipo di elemento ossia "constant", "variable" o "array";
- current_type, il tipo "int", "float", "char *" o "bool";
- current_value, il valore assegnato alla variabile: in particolare esso è zero se
l'assegnazione è complessa o NULL se è un array;
- dim, la dimensione dell'array, altrimenti varrà zero;
- nr, il numero riga segnalato dalla variabile yylineno di Flex. */
void add_element( char *nameToken, char *element, char *current_type, char *current_value, int
                  dim, int nr ) {
    element_ptr s;
    element_ptr exist = find_element( nameToken );

//se non esiste inserisce l'elemento nella Symbol table.
    if ( !exist ) {
        s = malloc( sizeof( el_ST ) );
        s->nameToken = nameToken;
        s->element = element;
        s->type = current_type;
        s->value = current_value;
        s->dim = dim;
        HASH_ADD_KEYPTR( hh, table, s->nameToken, strlen( s->nameToken ), s );
        /*se si tenta di inserire una costante viene sollevato un errore fatale, che blocca la
        compilazione. Si sta cercando di ridefinire una costante.*/
    } else if ( strcmp( exist->element, "constant" ) == 0 ) {
        printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: ridefinizione di una costante.\033[00m\n", nr);
        printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
        pulizia( );
        //exit( 0 );
//altrimenti è una riassegnazione di valori a una variabile: occorre verificare che la riassegnazione non violi il tipo precedentemente definito.
    } else {
//in caso di non violazione verrà aggiornato il valore della variabile.
        if ( strcmp( exist->type, current_type ) == 0 ) {
            delete_element( exist );
            exist->value = current_value;
	    // manca HASH_DEL
            HASH_ADD_KEYPTR( hh, table, exist->nameToken, strlen( exist->nameToken ), exist );
//altrimenti sarà lanciato un errore semantico fatale.
        } else {
            printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'assegnazione viola il tipo primitivo della variabile.\033[00m\n", nr);
            printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
            pulizia( );
            //exit( 0 );
        }
    }
}

/** La funzione type_checking controlla se le varie operazioni matematiche di +, -, *, /, %, ++,
--, <, <=, >, >= abbiano come operandi variabili intere ( int ) o reali ( float ). In caso di
utilizzo di variabili booleane o di stringhe vengono generati dei warning mediante
l'assegnazione di un opportuno valore alla variabile notice.
Gli argomento sono:
- TT, una lista definita in utility.h che contiene tutti i tipi associati agli operandi;
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
void type_array_checking( listaStringhe *TT, char context, element_ptr exist, int nr ) {
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
                pulizia( );
                //exit( 0 );
            }
            TT = TT->next;
        }
//stampa_lista( TT );
        break;
//assegnazione singola
    case 's':
        if ( strcmp( exist->type, TT->stringa ) != 0 ) {
            printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'assegnazione viola l'omogeneità degli elementi dell'array \"%s\".\033[00m\n", nr, exist->nameToken );
            printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
            pulizia( );
            //exit( 0 );
        }
        break;
//assegnazione multipla ( espressioni )
    case 'm':
        tipo = exist->type;
        while ( TT != NULL ) {
            if ( strcmp( TT->stringa, tipo ) != 0 )
            {
                printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'assegnazione viola l'omogeneità degli elementi dell'array \"%s\".\033[00m\n", nr, exist->nameToken );
                printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
                pulizia( );
                //exit( 0 );
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
- nameToken, il nome del simbolo da cercare.
Restituisce 1 se l'elemento è stato trovato altrimenti 0. */
int check_element_gen_code( char *nameToken ) {
    if ( find_element( nameToken ) != NULL )
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
- nameToken, il nome dell'elemento dell'array;
- offset, l'indice dell'elemento;
- nr, il numero riga segnalato dalla variabile yylineno di Flex. */
void check_index( char *nameToken, char *offset, int nr ) {
    int index;
    element_ptr exist = find_element( nameToken );
    element_ptr exist_index;
//se l'elemento non esiste lancia un errore semantico fatale.
    if ( !exist ) {
        printf( "\033[01;31m\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, nameToken );
        printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
        pulizia( );
        //exit( 0 );
    } else {
//mediante la funzione isnumeric contenuta nel file utility.h ci si accerta che l'offset sia un numero
        if ( isnumeric( offset ) ) {
//in caso affermativo la funzione atoi converte il contenuto della stringa ( in tal caso un numero ) nel corrispondente valore intero.
            index = atoi
                    ( offset );
        } else {
//in caso contrario l'offset è una variabile. Occorre controllare che esista mediante la funzione find_element.
            exist_index = find_element( offset );

//se esiste
            if ( exist_index ) {
                /*controlla che un indice non sia intero poichè è ammissibile,
                quindi viene effettuato un controllo sulla variabile offset. Se il tipo è diverso da "int" viene
                visualizzato un errore semantico fatale.*/
                if ( strcmp( exist_index->type, "int" ) != 0 ) {
                    printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'uso di un elemento con offset non intero non è ammissibile.\033[00m\n", nr);
                    printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
                    pulizia( );
                    //exit( 0 );
                }
//la funzione atoi converte il valore della variabile nel corrispondente valore intero.
                index = atoi( exist_index->value );
            } else {
//se non esiste viene visualizzato un errore semantico fatale.
                printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, offset );
                printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
                pulizia( );
                //exit( 0 );
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
- nameToken, il nome della variabile o dell'elemento di un array;
- offset, l'indice dell'elemento dell'array;
- nr, il numero riga segnalato dalla variabile yylineno di Flex;
- read, specifica se l'elemento è analizzato in lettura o scrittura ( solo per elementi
di
un array ).
Restituisce l'elemento identificato dal nameToken, trovato nella Symbol table. */
element_ptr check_element( char *nameToken, char *offset, int nr, bool read )
{
    int index;
    char *tipo;
    char *el_array;

    element_ptr exist = find_element( nameToken );
    element_ptr exist_index;
//se l'elemento non esiste lancia un errore semantico fatale.
    if ( !exist ) {
        printf( "\033[01;31m\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, nameToken );
        printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
        pulizia( );
       //exit( 0 );

    } else {
//se è una variabile di sola lettura o una costante lo aggiungo alle liste T ed Exp ( Espressione ).
        if ( ( ( strcmp( exist->element, "variable" ) == 0 ) || ( strcmp( exist->element, "constant" ) == 0 ) ) && read ) {
//aggiungo il tipo dell'elemento alla lista T.
            ins_in_lista( &listaTipi, exist->type );
//aggiungo il nome dell'elemento alla lista Exp.
            ins_in_lista( &espressioni, exist->nameToken );
        } else {
//se è un elemento di un array di sola lettura ( non è un assegnazione a se sstringaso ) lo aggiungo alle liste T ed Exp ( Espressione ).
            if ( read ) {
//aggiungo il tipo dell'elemento alla lista T.
                ins_in_lista( &listaTipi, exist->type );
//viene legato al nome dell'array ( identificato dal nameToken ) l'offset: nome[offset]. Successivamente aggiungo il nome composto alla lista Exp.
                el_array = nameToken;
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
//in caso contrario l'offset è una variabile. Occorre controllare che esista mediante la funzione find_element.
                exist_index = find_element( offset );

//se esiste.
                if ( exist_index ) {
                    /*controlla che un indice non sia intero poichè è
                    ammissibile, quindi viene effettuato un controllo sulla variabile offset. Se il tipo è diverso
                    da "int" viene visualizzato un errore semantico fatale.*/
                    if ( strcmp( exist_index->type, "int" ) != 0 ) {
                        printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'uso di un elemento con offset non intero non è ammissibile.\033[00m\n", nr);
                        printf( "\n\n\033[01;31mParsing fallito.\033[00m \n" );
                        pulizia( );
                        //exit( 0 );
                    }
//la funzione atoi converte il valore della variabile nel corrispondente valore intero.
                    index = atoi( exist_index->value );
                } else {
//se non esiste viene visualizzato un errore semantico fatale.
                    printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, offset );
                    printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
                    pulizia( );
                    //exit( 0 );
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

/** La funzione echo_check verifica se le variabili, gli elementi di un array o le costanti
specificate nell'istruzione echo esistono o meno nella Symbol table. Nel caso degli elementi
di un array viene controllato l'offset secondo le sstringase modalità descritte in precedenza.
Sulla base del tipo di variabile analizzata vengono assegnate alle liste Exp e Phrase
(definite nel file utility.h) rispettivamente gli elementi e la frase che saranno
successivamente tradotti nella funzione C printf mediante la funzione gen_echo ( contenuta
nel file gen_code.h ).
Gli argomento sono:
- nameToken, il nome della variabile, dell'elemento di un array o di una costante
contenuta nell'espressione associata all'istruzione echo;
- offset, l'indice dell'elemento dell'array;
- nr, il numero riga segnalato dalla variabile yylineno di Flex. */
void echo_check( char *nameToken, char *offset, int nr )
{
    int index;
    char *tmp = " ) ? \"true\" : \"false\""; //variabile utilizzata per gestire la traduzione della stampa di valori booleani.
    char *el_array;
    element_ptr exist = find_element( nameToken );
    element_ptr exist_index;
//se l'elemento non esiste lancia un errore semantico fatale.
    if ( !exist ) {
        printf( "\033[01;31m\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, nameToken );
        printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
        pulizia( );
       //exit( 0 );
    } else {
//se l'elemento esiste e se è un array vengono effettuati gli sstringasi controlli sull'offset.
        if ( strcmp( exist->element, "array" ) == 0 ) {
            if ( offset == NULL ) {
                printf( "\033[01;31m\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: offset non definito.\033[00m\n", nr );
                printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
                pulizia( );
                //exit( 0 );
            }
            if ( isnumeric( offset ) ) {
                index = atoi( offset );
            } else {
                exist_index = find_element( offset );

//se esiste.
                if ( exist_index ) {
                    /*controlla che un indice non sia intero poichè è
                    inammissibile. Quindi viene effettuato un controllo sulla variabile offset. Se il tipo è
                    diverso da "int" viene visualizzato un errore semantico fatale.*/
                    if ( strcmp( exist_index->type, "int" ) != 0 ) {
                        printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: l'uso di un elemento con offset non intero non è ammissibile.\033[00m\n", nr);
                        printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
                        pulizia( );
                       //exit( 0 );
                    }
//la funzione atoi converte il valore della variabile nel corrispondente valore intero.
                    index = atoi( exist_index->value );
                } else {
                    printf( "\033[01;31m\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: variabile \"%s\" non definita.\033[00m\n", nr, nameToken );
                    printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
                    pulizia( );
                    //exit( 0 );
                }
            }
//l'offset viene legato al nome dell'array ( identificato dal nameToken ): nome[offset].
            el_array = nameToken;
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
                char *c = ( char * )malloc( ( strlen( nameToken ) + strlen
                                              ( tmp ) + 1 ) * sizeof( char ) );
                strcpy( c, "( " );
                strcat( c, nameToken );
                strcat( c, tmp );
                ins_in_lista( &espressioni, c );
                free( c );
            } else
                ins_in_lista( &espressioni, nameToken );
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
        element_ptr exist = find_element( string );

        if ( exist && ( strcmp( exist->element, "constant" ) == 0) ) {
            current_type = exist->type;
            trov = 1;
        }
    }
//se non è una costante viene lanciato un errore semantico fatale.
    if ( trov == 0 ) {
        printf( "\033[01;31mRiga %i. [ FATALE ] ERRORE SEMANTICO: stringa \"%s\" non riconosciuta.\033[00m\r\n", nr, string );
        printf( "\n\n\033[01;31mParsing fallito.\033[00m\n" );
        pulizia( );
       // exit( 0 );
    }

    return current_type;
}

