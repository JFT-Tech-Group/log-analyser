#ifndef STATS_H
#define STATS_H

#include <systemd/sd-journal.h>

long int count_entries( sd_journal *journal );

#endif