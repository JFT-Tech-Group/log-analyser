/**
 * @file lecture.c
 * @brief fonction pour lire et découper les lignes de logs
*/

#include <time.h>
#include "filter.h"
#include "interface.h"
#include "lecture.h"
#include "utils.h"

// Récupération et affichage du timestamp d'une entrée du journal ( buffer 30 caractères ! )
int gettime( sd_journal *journal, char *buffer )
{
    uint64_t usec;
    time_t ts;
    struct tm time;

    // Récupération du timestamp
    if ( sd_journal_get_realtime_usec( journal, &usec ) < 0 )
    {
        return( -1 );
    }

    ts = (time_t)( usec / 1000000 );

    if ( !localtime_r( &ts, &time ) )
    {
        return( -1 );
    }

    if ( strftime( buffer, 30, "%Y-%m-%d %H:%M:%S", &time ) == 0 )
    {
        strcpy( buffer, "Unknown" );
    }

    return( 0 );
}

// Récupération d'un champs spécifique ( après "=" )
int get_field( sd_journal *journal, char field[], char *data, size_t data_size )
{
    const void *tmp;
    size_t length;

    int r = sd_journal_get_data( journal, field, &tmp, &length );
    if ( r  < 0 )
    {
        return( -1 );
    }

    char *eq = strchr( ( char * )tmp, '=' );
    if ( eq == NULL )
    {
        data[0] = '\0'; // champ vide si pas de "=" trouvé
        return( -1 );
    }

    eq++;
    length = ( ( char * )tmp + length ) - eq;
    
    if ( length >= data_size )
    {
        length = data_size - 1;
    }
    
    strncpy( data, eq, length );
    data[length] = '\0';

    return( 0 );
}

// Récupération du nom du service
int get_entry_service_name( sd_journal *journal, char *data )
{
    const void *tmp;
    size_t length;

    int r = sd_journal_get_data( journal, "_SYSTEMD_UNIT", &tmp, &length );
    if ( r < 0 )
    {
        r = sd_journal_get_data( journal, "SYSLOG_IDENTIFIER", &tmp, &length );
        if ( r < 0 )
        {
            r = sd_journal_get_data( journal, "_COMM", &tmp, &length );
            if ( r < 0 )
            {
                r = sd_journal_get_data( journal, "_EXE", &tmp, &length );
                if ( r < 0 )
                {
                    return( -1 );
                }
            }
        }
    }

    char *eq = strchr( ( char * )tmp, '=' );
    if ( eq == NULL )
    {
        data[0] = '\0'; // champ vide si pas de "=" trouvé
        return( -1 );
    }

    eq++;
    length = ( ( char * )tmp + length ) - eq;

    if ( length >= MAX_FIELD )
    {
        length = MAX_FIELD - 1;
    }

    strncpy( data, eq, length );
    data[length] = '\0';
    
    return( 0 );
}

// Récupération et parsage des champs de l'entrée courante
int get_entry( sd_journal *journal, Entry *entre, char match_keyword[] )
{
    // Initialisation de l'entré
    memset( entre, 0, sizeof( Entry ) );

    // Vérification si les champs de l'entrée courante contient le mots clés donné
    if ( strlen( match_keyword ) > 0 )
    {
        int pos = add_match_keyword( journal, match_keyword, entre->field_matched );

        if ( pos <= 0 )
        {
            return( -1 );
        }
    }

    // Récupération du timestamp en format lisible
    if ( gettime( journal, entre->timestamp ) < 0 ) strcpy( entre->timestamp, "Unknown" );

    // Récupération des différents champs
    if ( get_field( journal, "_BOOT_ID", entre->boot_id, MAX_FIELD ) < 0 ) strcpy( entre->boot_id, "Unknown" );
    if ( get_field( journal, "MESSAGE", entre->message, MAX_MSG ) < 0 ) strcpy( entre->message, "Unknown" );
    if ( get_field( journal, "PRIORITY", entre->priority, MAX_FIELD ) < 0 ) strcpy( entre->priority, "Unknown" );
    if ( get_field( journal, "_HOSTNAME", entre->host, MAX_FIELD ) < 0 ) strcpy( entre->host, "Unknown" );
    if ( get_field( journal, "_PID", entre->pid, MAX_FIELD ) < 0 ) strcpy( entre->pid, "Unknown" );
    if ( get_field( journal, "_UID", entre->uid, MAX_FIELD ) < 0 ) strcpy( entre->uid, "Unknown" );
    if ( get_field( journal, "_GID", entre->gid, MAX_FIELD ) < 0 ) strcpy( entre->gid, "Unknown" );
    
    // Récupération du nom du service
    if ( get_entry_service_name( journal, entre->service ) < 0 ) strcpy( entre->service, "Unknown" );

    return( 0 );
}

// Ouverture du journal en fonction de la catégorie sélectionnée
int open_log( WINDOW *left, WINDOW *right, int flags )
{
    int journaux[] = { SD_JOURNAL_LOCAL_ONLY, SD_JOURNAL_SYSTEM, SD_JOURNAL_CURRENT_USER, SD_JOURNAL_RUNTIME_ONLY, 0 };
    sd_journal *journal;

    int r = sd_journal_open( &journal, journaux[flags - 1] );

    if ( r < 0 )
    {
        return( -1 );
    }

    // Pour que la première entrée soit la première du journal
    sd_journal_seek_head( journal );
    r = sd_journal_next( journal );

    if ( r < 0 )
    {
        sd_journal_close( journal );
        return( -1 );
    }
    
    // Affichage du sous-menu
    sub_menu( left, right, journal );

    sd_journal_close( journal );

    return( 0 );
}