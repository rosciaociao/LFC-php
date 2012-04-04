/*
+----------------------------------------------------------------------+
| PHP2C -- utility.h |
+----------------------------------------------------------------------+
| Autori: BAVARO Gianvito |
| CAPURSO Domenico |
| DONVITO Marcello |
+----------------------------------------------------------------------+
*/

/* Questo file contiene un insieme di strutture dati e funzioni di utilità e supporto per il
processo di compilazione e traduzione. */

typedef enum { false, true } bool; /* Struttura per un nuovo tipo di dato: bool.
 Rappresenta i valori booleani. */

typedef struct testo /* Struttura per un nuovo tipo di dato: testo. E' una lista concatenata di stringhe. */
{
    char *tes;
    struct testo *next;
} testo;

/** Lista concatenata contenente i tipi delle variabili, elementi di array o costanti. Utile per il type_checking.*/
testo *T;

/** Lista concatenata contenente gli elementi di una espressione (variabili, elementi di array, costanti, operatori).
 * Utile per  stampare intere espressioni nel file f_out.c contenente la  traduzione in C. */
testo *Exp;

/** Lista concatenata contenente frasi o parole ( non keywords ). Utile per stampare intere frasi,
 * anche associate a espressioni, nel file f_out.c . */
testo *Phrase;

/** La funzione clear cancella il contenuto di ogni lista. Una funzione molto importante poichè è
 necessario resettare le liste ogni volta che il parser valida una frase. */
void clear( ) {
    T = NULL;
    Exp = NULL;
    Phrase = NULL;
}

/** La funzione put_testo inserisce una stringa in una lista.
Gli argomenti sono:
- TT, un doppio puntatore alla lista per consentire l'aggiunta di nuovi elementi;
- str, la stringa da inserire nel campo tes del nuovo elemento che viene inserito
nella lista. */
void put_testo( testo **TT,char *str )
{
    testo *punt,*t_el;
    t_el = ( testo * )malloc( sizeof( testo ) );
    t_el->tes = ( char * )strdup( str );
    t_el->next = NULL;
    if ( *TT == NULL )
        *TT = t_el;
    else
    {
        punt = *TT;
        while ( punt->next!=NULL )
            punt = punt->next;
        punt->next = t_el;
    }
}

/** La funzione get_testo stampa a video tutti i valori di una lista.
Gli argomenti sono:
- TT, un puntatore alla lista;
- nome, una stringa d'utilità per etichettare la stampa, in modo tale da indentificare
la lista. */
void get_testo( testo *TT, char *nome )
{
    testo *punt = TT;

    printf( "/* * * * * * * * * * * %s * * * * * * * * * * */\n\n", nome );
    while ( punt != NULL ) {
        printf( "Elemento: %s\n", punt->tes );
        punt = punt->next;
    }
    printf( "\n/* * * * * * * * * * * * * * * * * * * * * * * * */\n\n" );
}

/** La funzione countelements conta il numero di elementi presenti in una lista.
L'argomento è:
- T, un puntatore alla lista;
Restituisce il numero degli elementi. */
int countelements( testo *T ) {
    testo *punt = T;
    int value = 0;

    while ( punt != NULL ) {
        value++;
        punt = punt->next;
    }
    return value;
}

/** La funzione isnumeric stabilisce se una stringa è costituita da soli numeri,
ossia se si tratta di un numero o no.
L'argomento è:
- str, la stringa da analizzare;
Restituisce 1 se la stringa è un numero, 0 altrimenti. */
int isnumeric( char *str )
{
//prelevo il primo carattere.
    char *c = (char *)strndup(str, 1);
//se è uguale a "-", è in analisi un numero negativo.
    if ( strcmp( c, "-" ) == 0) {
//scarta il segno "-".
        c = ( char * )strdup( str + 1 );
//per ogni carattere verifica che sia un numero.
        while ( *c )
        {
//se anche un solo carattere non è un numero, restituisce zero e interrompe il ciclo.
            if ( !isdigit( *c ) )
                return 0;
            c++;
        }
//se il primo carattere non è "-", è in analisi un numero maggiore o uguale a zero.
    } else {
//per ogni carattere verifica che sia un numero.
        while ( *str )
        {
//se anche un solo carattere non è un numero, restituisce zero e
            interrompe il ciclo.
            if ( !isdigit( *str ) )
                return 0;
            str++;
        }
    }
    return 1;
}