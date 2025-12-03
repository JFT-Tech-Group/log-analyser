#ifndef LECTURE_H
#define LECTURE_H

#include <ncurses.h>
#include <systemd/sd-journal.h>

#define MAX_FIELD 256
#define MAX_MSG 2048

// Structure pour stocker une entrée de journal
typedef struct Entry Entry;
struct Entry
{
    char timestamp[MAX_FIELD];
    char boot_id[MAX_FIELD];

    char service[MAX_FIELD];
    char message[MAX_MSG];
    char host[MAX_FIELD];
    
    char priority[MAX_FIELD];
    char pid[MAX_FIELD];
    char uid[MAX_FIELD];
    char gid[MAX_FIELD];

    char field_matched[MAX_FIELD];
};

int get_entry( sd_journal *journal, Entry *entre, char match_keyword[] );
int open_log( WINDOW *left, WINDOW *right, int flags );

#endif