/** 
 * @file stats.c
 * @brief Fonction de comptage, regroupement et statistiques
*/

#include "utils.h"

// Compte le nombre total d'entrées dans le journal
// Retourne le nombre d'entrées ou -1 en cas d'erreur
uint64_t count_entries_exact( sd_journal *journal )
{
    int r = sd_journal_seek_head( journal );
    if ( r < 0 )
    {
        fprintf( stderr, "Impossible de se placer au début : %s\n", strerror( -r ) );
        return( -1 );
    }

    uint64_t total = 0;

    // Lecture bloc par bloc pour accélérer
    const int BLOCK_SIZE = 10000;
    while ( 1 )
    {
        r = sd_journal_next_skip( journal, BLOCK_SIZE );
        if ( r > 0 )
        {
            total += r; // sd_journal_next_skip retourne toujours 1 si au moins une entrée sautée
        }
        else if ( r == 0 )
        {
            // Fin du journal
            break;
        }
        else
        {
            fprintf( stderr, "Erreur lors du comptage des entrées : %s\n", strerror( -r ) );
            return( -1 );
        }
    }

    // Ajustement final avec sd_journal_next pour toutes les entrées restantes
    while ( ( r = sd_journal_next( journal ) ) > 0 )
    {
        total++;
    }

    if ( r < 0 )
    {
        fprintf( stderr, "Erreur lecture journal : %s\n", strerror( -r ) );
        return ( -1 );
    }

    // Remet le journal au début
    sd_journal_seek_head( journal );

    return( total );
}