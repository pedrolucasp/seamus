#ifndef SEAMUS_SEAMUS
#define SEAMUS_SEAMUS
#include <string.h>
#include <stdbool.h>
#include <mpd/client.h>
#include <tickit.h>
#include "log.h"

struct seamus_song {
	int song_id;
	char *title;
	char *artist;
};

struct seamus_status {
	bool repeat;
	bool random;
	bool single;

	int current_song_position;
	int current_song_id;

	int version;
	int length;

	unsigned elapsed_time;
	unsigned total_time;

	enum mpd_state state;
	char *description;
};

struct seamus_frontend {
	struct mpd_connection *conn;
	struct seamus_song *queue;
	size_t queue_size;
	struct seamus_status *status;
	int scroll_position;
	TickitWindow *main_window;
	TickitWindow *status_window;
	Tickit *t;
};

int seamus_init(struct seamus_frontend *s);
int setup_connection(struct seamus_frontend *s);
int fetch_mpd_from_current_queue(struct seamus_frontend *seamus, int max_count);

#endif
