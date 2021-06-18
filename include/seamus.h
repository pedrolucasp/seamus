#ifndef SEAMUS_SEAMUS
#define SEAMUS_SEAMUS
#include <string.h>
#include <stdbool.h>
#include <mpd/client.h>
#include <tickit.h>
#include "log.h"

struct seamus_song {
	char *title;
	char *artist;
};

struct seamus_frontend {
	struct mpd_connection *conn;
	struct seamus_song *queue;
	size_t queue_size;
	TickitWindow *main_window;
	Tickit *t;
};

int seamus_init(struct seamus_frontend *s);
int setup_connection(struct seamus_frontend *s);
int fetch_mpd_from_current_queue(struct seamus_frontend *seamus, int max_count);

#endif
