#ifndef SEAMUS_QUEUE
#define SEAMUS_QUEUE
#include "seamus.h"

int fetch_mpd_from_current_queue(struct seamus_frontend *seamus, int max_count);
void print_songs_from_queue(struct seamus_frontend *seamus);

#endif
