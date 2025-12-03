/**
 * @file filter.c
 * @brief fonction de filtrage ( date, mot-clé, service,... )
*/

#include <time.h>
#include "filter.h"
#include "interface.h"
#include "lecture.h"
#include "utils.h"

// Ajout d'un filtre pour un programme spécifique dans le journal systemd ( return 0 si succès, -1 sinon )
int add_match_programme( sd_journal *journal, char programme[] )
{
    char fields[][ MAX_CHAINE ] = { "_SYSTEMD_UNIT=", "SYSLOG_IDENTIFIER=", "_COMM=", "_EXE=" };
    int ret;
    
    for ( int i = 0; i < 4; i++ )
    {
        strcat( fields[i], programme );
        ret = sd_journal_add_match( journal, fields[i], 0 );
        if ( ret < 0 )
        {
            return( -1 );
        }

        if ( i < 3 )
        {
            ret = sd_journal_add_disjunction( journal );
            if ( ret < 0 )
            {
                return( -1 );
            }
        }
    }

    return( 0 );
}

// Ajout d'un filtre pour un niveau de gravité spécifique dans le journal systemd ( return 0 si succès, -1 sinon )
int add_match_gravity( sd_journal *journal, int level )
{
    if ( level < 0 || level > 7 )
    {
        return -1;
    }

    char match[32];
    snprintf( match, sizeof( match ), "PRIORITY=%d", level );

    return( sd_journal_add_match( journal, match, 0 ) );
}

// Ajout d'un filtre par mots clés
// Cette fonction recherche un mot clé dans les champs de l'entrée courante
// Si le mot clé est trouvé, il copie la chaîne correspondante dans field_matched et retourne la position du mot clé dans cette chaîne
// Si le mot clé n'est pas trouvé, elle retourne 0
int add_match_keyword( sd_journal *journal, const char keyword[], char field_matched[] )
{
    void *data;
    size_t length;
    int pos = 0;

    if ( keyword == NULL )
    {
        return( -1 );
    }

    SD_JOURNAL_FOREACH_DATA( journal, data, length )
    {
        void *found = memmem( data, length, keyword, strlen( keyword ) );

        if ( found != NULL )
        {
            if ( field_matched != NULL )
            {
                size_t copy_len = length < MAX_FIELD - 1 ? length : MAX_FIELD - 1;
                memcpy(field_matched, data, copy_len);
                field_matched[copy_len] = '\0';
            }

            pos = ( int )( found - data );
            sd_journal_restart_data( journal ); // Repositionne l'indexation des champs de l'entrée courante

            return( pos );
        }
    }

    sd_journal_restart_data( journal ); // Repositionne l'indexation des champs de l'entrée courante
    return( 0 );
}

// Positionne le journal systemd à une date/heure spécifique
// Le format de la chaîne de caractères doit être "YYYY-MM-DD HH:MM:SS"
// Retourne 0 si succès, -1 sinon
int settime( sd_journal *journal, const char *buffer )
{
    struct tm tm;
    time_t t;
    int ret;

    // Conversion de la chaîne de caractères en struct tm
    if ( strptime( buffer, "%Y-%m-%d %H:%M:%S", &tm ) == NULL )
    {
        return( -1 );
    }

    // Conversion de struct tm en time_t (timestamp)
    t = mktime( &tm );
    if ( t == -1 )
    {
        return( -1 );
    }

    ret = sd_journal_seek_realtime_usec( journal, ( uint64_t )t * 1000000 );
    if ( ret < 0 )
    {
        return( -1 );
    }
    sd_journal_next( journal );

    return( 0 );
}

// Suppression de tous les filtres appliqués au journal systemd
int remove_all_filters( sd_journal *journal )
{
    sd_journal_flush_matches( journal );

    sd_journal_seek_head( journal ); // Toujours revenir au premier entrée
    sd_journal_restart_data(journal); // Repositionne l'indexation des champs de l'entrée courante

    return( 0 );
}

// Application des filtres via API systemd
// Retourne 0 si succès, -1 sinon
int applique_filtres_systemd( sd_journal *journal, FiltresActifs *filtres_actifs )
{
    int ret;

    // Suppression des filtres existants
    remove_all_filters( journal );

    if ( filtres_actifs->gravite >= 0 )
    {
        ret = add_match_gravity( journal, filtres_actifs->gravite );
        if ( ret < 0 )
        {
            return( -1 );
        }
    }

    if ( strlen( filtres_actifs->service ) > 0 )
    {
        ret = add_match_programme( journal, filtres_actifs->service );
        if ( ret < 0 )
        {
            return( -1 );
        }
    }

    sd_journal_seek_head( journal ); // Toujours revenir au premier entrée
    ret = sd_journal_next( journal ); // Positionner sur la première entrée après application des filtres
    if ( ret < 0 )
    {
        return( -1 );
    }

    return( 0 );
}

// Gestion des filtres appliqués via recherche de mots clés et date
// Retourne 0 si succès, -1 sinon
int applique_filtres_personnalises( sd_journal *journal, FiltresActifs *filtres_actifs, int next )
{
    int ret = 0;
    if ( strlen( filtres_actifs->mot_cle ) > 0 )
    {
        do
        {
            ret = add_match_keyword( journal, filtres_actifs->mot_cle, NULL );
            if ( ret == 0 )
            {
                if ( next )
                {
                    ret = sd_journal_next( journal );
                }
                else
                {
                    ret = sd_journal_previous( journal );
                }
            }
            else
            {
                return( 0 ); // Mot-clé trouvé
            }
            
        } while ( ret > 0 );
    }
    return( -1 ); // Mot-clé non trouvé
}