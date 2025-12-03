/** 
 * @file utils.c
 * @brief Fonctions utilitaires et menu d'aide
*/

#include "utils.h"

// Lecture d'une chaîne de caractères depuis l'entrée standard ( retourne -1 en cas d'erreur et 0 sinon )
int getchaine( char chaine[] , size_t max_size )
{
    char *ret = NULL;
    int i = 0;

    if ( chaine == NULL || max_size == 0 )
    {
        return( -1 );
    }
    
    if ( fgets( chaine, max_size, stdin ) == NULL )
    {
        return( -1 );
    }
    
    ret = strchr( chaine, '\n' );

    if ( ret != NULL )
    {
        *ret = '\0';
    }
    else
    {
        while( getchar() != '\n' );
    }
    
    return( 0 );
}

// Lecture d'un entier depuis l'entrée standard
int get_nbr( int *choix )
{
    if ( !scanf( "%d", choix ) )
    {
        while( getchar() != '\n' );
        return( 0 );
    }
    
    return( 1 );
}