/*
+----------------------------------------------------------------------+
| PHP2C -- gen_code.h |
+----------------------------------------------------------------------+
| Autori: BAVARO Gianvito |
| CAPURSO Domenico |
| DONVITO Marcello |
+----------------------------------------------------------------------+
*/

/* Questo file contiene un insieme di funzioni utili alla traduzione in C di alcuni costrutti e
istruzione in PHP. Tuttavia la maggior parte della traduzione viene svolta mediante funzioni
direttamente inserite nella definizione della grammatica, contenuta nel file php_parser.y. */

#include <stdio.h> /* Serve per la gestione dei messaggi di errore semantico. */
#include <string.h> // Serve per la gestione delle stringhe

#define PATH "/home/gianluca/projects/ph2c/f_out.c" /* Rappresenta il percorso dove verrà salvato il file "f_out.c" contenente la traduzione in C. */

char *op_name[] = { "=", "+=", "-=", "*=", "/=", "%=" };/* Un array contenente tutti i possibili operatori di assegnazione. */

/** La funzione apri_file apre il file f_out.c che conterrà il codice intermedio C, frutto della
compilazione e traduzione.
Restituisce il puntatore al file. */

FILE *apri_file( ) {
    FILE *f_ptr;

    if ( ( f_ptr = fopen( PATH, "w" ) ) == NULL ) {
        printf( "\033[01;31mERRORE: apertura del file f_out.c fallita. Directory %s non esistente.\033[00m\n", PATH );
        exit( 0 );
    }

    return f_ptr;
}

/** La funzione chiudi_file chiude il file f_out.c precedentemente aperto. */
void chiudi_file( FILE *f_ptr ) {
//forza l'effettiva scrittura di eventuali dati presenti nel buffer, prima della chiusura.
    fflush( f_ptr );
//chiude il file.
    fclose( f_ptr );
}

/** La funzione chiudi_cancella_file, in caso di errore semantico o grave, chiude ed elimina il
file f_out.c .
L'argomento è:
- f_ptr, il puntatore al file da chiudere. */
void chiudi_cancella_file( FILE *f_ptr ) {
    if ( f_ptr != NULL )
        chiudi_file( f_ptr );

    if ( remove( PATH ) == -1 )
        printf( "\033[01;31mERRORE: impossibile cancellare il file.\033[00m\n" );
}

/** La funzione cancella_file elimina il file f_out.c . Essa è richiamata ad ogni inizio
compilazione, in modo tale da poter ricreare un nuovo file. */
void cancella_file( ) {
    FILE *f_ptr;
//solo se il file esiste sarà cancellato.
    if ( ( f_ptr = fopen( PATH, "r" ) ) ) {
        fclose( f_ptr );
        if ( remove( PATH ) == -1 )
            printf( "\033[01;31mERRORE: impossibile cancellare il file.\033[00m\n" );
    }
}

/** La funzione gen_header genera l'header del file C, scrivendo due istruzioni di default stdio
e string, il metodo main e la definizione del tipo boolean.
L'argomento è:
- f_ptr, il puntatore al file nel quale stampare la traduzione. */
void gen_header( FILE* f_ptr ) {
    fprintf( f_ptr, "#include <stdio.h>\n" );
    fprintf( f_ptr, "#include <string.h>\n\n" );
    fprintf( f_ptr, "void main( void ) {\n" );
    fprintf( f_ptr, "\ttypedef enum { false, true } bool;\n" );
}

/** La funzione gen_constant genera le costanti dichiarate.
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare la traduzione;
- nameConstant, il nome della costante;
- type, il tipo della costante;
- value, il valore della costante. */
void gen_constant( FILE* f_ptr, char *nameConstant, char *type, char *value ) {
//se è una costante di tipo stringa allora devo stampare un array di caratteri.
    if ( strcmp( type, "char *" ) == 0 )
        fprintf( f_ptr, "const char %s[] = %s;\n", nameConstant, value );
    else
        fprintf( f_ptr, "const %s %s = %s;\n", type, nameConstant, value );
}

/** La funzione gen_expression genera le espressioni, create da composizioni di variabili,
elementi di array e costanti legate fra loro dagli operatori previsti dalla grammatica. Tutti
questi elementi sono stati precedentemente inseriti nella lista Exp.
L'argomento è:
- Exp, la lista contenenti gli elementi delle espressioni.
Restituisce l'espressione. */
char *gen_expression( testo *Exp ) {
    char *expression = NULL;

//get_testo( Exp, "ESPR" );
//assegna a expression il primo valore della lista Exp.
    if ( Exp != NULL ) {
        expression = Exp->tes;
        Exp = Exp->next;
    }
//concatena tutti i successivi elementi della lista.
    while ( Exp != NULL ) {
        strcat( expression, Exp->tes );
        Exp = Exp->next;
    }
    if ( Exp != NULL ) {
        strcat( expression, "\0" );
    }

    return expression;
}

/** La funzione gen_echo_expression è simile alla precedente ma finalizzata alla creazione di
espressioni per l'istruzione echo.
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare la traduzione;
- Exp, la lista contenenti gli elementi delle espressioni.
Restituisce l'espressione. */
char *gen_echo_expression( FILE* f_ptr, testo *Exp ) {
    char *expression = NULL;
//se gli elementi sono in numero maggioe di uno è prevista la stampa della ",".
    int elements = countelements( Exp );

//get_testo( Exp, "GEN ECHO ESPR" );

//assegna a expression il primo valore della lista Exp.
    if ( Exp != NULL ) {
        expression = Exp->tes;
        Exp = Exp->next;
    }
//concatena tutti i successivi elementi della lista.
    while ( Exp != NULL ) {
        if ( elements > 1 )
            strcat( expression, ", " );
        strcat( expression, Exp->tes );
        Exp = Exp->next;
    }
    if ( Exp != NULL ) {
        strcat( expression, "\0" );
    }

    return expression;
}

/** La funzione print_expression genera le espressioni che stamperà nel file f_out.c, create da
composizioni di variabili, elementi di array e costanti legate fra loro dagli operatori previsti
dalla grammatica. Tutti questi elementi sono stati precedentemente inseriti nella lista Exp.
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare la traduzione;
- Exp, la lista contenenti gli elementi delle espressioni. */
void print_expression( FILE* f_ptr, testo *Exp ) {
    char *expression;

//get_testo( Exp, "ESPR" );

    expression = Exp->tes;
    Exp = Exp->next;

    while ( Exp != NULL ) {
        strcat( expression, Exp->tes );
        Exp = Exp->next;
    }
    strcat(expression, "\0");
    fprintf( f_ptr, "%s", expression );
}

/** La funzione gen_create_array stampa la dichiarazione di un array.
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare la traduzione;
- name_array, il nome dell'array;
- type, il tipo dell'array;
- Exp, la lista contenenti gli elementi delle espressioni. */
void gen_create_array( FILE* f_ptr, char *name_array, char *type, testo *Exp ) {
    char *expression = gen_expression( Exp );
//Se l'array non è stato definito viene stampato anche il tipo, altrimenti no.
    if ( check_element_gen_code( name_array ) )
        fprintf( f_ptr, "%s[] = { %s }", name_array, expression );
    else
        fprintf( f_ptr, "%s %s[] = { %s }", type, name_array, expression );
}


/** La funzione gen_assignment stampa le dichiarazioni di una variabile o di un elemento di un
array.
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare la traduzione;
- index, indice per identificare il tipo di operatore di assegnazione contenuto
nell'array op_name;
- left_var, il nome della variabile o dell'elemento dell'array;
- type, il tipo dell'array;
- index_element, l'offset dell'elemento di un array;
- Exp, la lista contenenti gli elementi delle espressioni. */
void gen_assignment( FILE* f_ptr, int index, char *left_var, char *type, char *index_element, testo *Exp, bool array ) {
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

/** La funzione gen_if genera l'istruzione condizionale if.
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare la traduzione;
- Exp, la lista contenenti gli elementi delle espressioni. */
void gen_if( FILE* f_ptr, testo *Exp ) {
    char *expression = gen_expression( Exp );
    fprintf( f_ptr, "if( %s ) {\n", expression );
}

/** La funzione gen_elseif genera l'istruzione condizionale else if.
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare la traduzione;
- Exp, la lista contenenti gli elementi delle espressioni. */
void gen_elseif( FILE* f_ptr, testo *Exp ) {
    char *expression = gen_expression( Exp );
    fprintf( f_ptr, " else if( %s ) {\n", expression );
}

/** La funzione gen_while genera l'istruzione di ciclo while.
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare la traduzione;
- Exp, la lista contenenti gli elementi delle espressioni. */
void gen_while( FILE* f_ptr, testo *Exp ) {
    char *expression = gen_expression( Exp );
    fprintf( f_ptr, "while( %s ) {\n", expression );
}

/** La funzione gen_switch genera il costrutto switch.
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare la traduzione;
- Exp, la lista contenenti gli elementi delle espressioni. */
void gen_switch( FILE* f_ptr, testo *Exp ) {
    char *expression = gen_expression( Exp );
    fprintf( f_ptr, "switch( %s ) {\n", expression );
}

/** La funzione gen_echo genera l'istruzione di stampa a video printf.
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare la traduzione;
- Exp, la lista contenenti gli elementi delle espressioni;
- Phrase, la lista contenenti gli elementi della frase da associare all'espressione. */
void gen_echo( FILE* f_ptr, testo *Exp, testo *Phrase ) {
    char *phrase = gen_expression( Phrase );
    char *expression = gen_echo_expression( f_ptr, Exp );

    if ( phrase != NULL && expression != NULL )
        fprintf( f_ptr, "printf( \"%s\", %s );\n", phrase, expression );
    else if ( phrase != NULL )
        fprintf( f_ptr, "printf( \"%s\" );\n", phrase );
    else
        fprintf( f_ptr, "printf( %s );\n", expression );
}

/** La funzione gen_tab genera il simbolo tab "\t".
Gli argomenti sono:
- f_ptr, il puntatore al file nel quale stampare il simbolo;
- ntab, il numero di stampa del simbolo tab. */
void gen_tab( FILE *f_ptr, int ntab ) {
    int i;
    for (i = 0; i < ntab; i++)
        fprintf( f_ptr, "\t" );
}