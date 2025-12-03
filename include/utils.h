#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-journal.h>

#define MAX_CHAINE 1024

int getchaine( char chaine[] , size_t max_size );
int get_nbr( int *choix );

#endif