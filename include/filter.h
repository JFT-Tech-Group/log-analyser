#ifndef FILTER_H
#define FILTER_H

#include <systemd/sd-journal.h>
#include "interface.h"

int add_match_programme( sd_journal *journal, char programme[] );
int add_match_gravity( sd_journal *journal, int gravity_level );
int add_match_keyword( sd_journal *journal, const char keyword[], char field_matched[] );
int settime( sd_journal *journal, const char *buffer );
int remove_all_filters( sd_journal *journal );
int applique_filtres_systemd( sd_journal *journal, FiltresActifs *filtres_actifs );
int applique_filtres_personnalises( sd_journal *journal, FiltresActifs *filtres_actifs, int next );

#endif