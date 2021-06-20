#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "seamus.h"
#include "ui.h"

int
setup_connection(struct seamus_frontend *s)
{
	struct mpd_connection *connection = mpd_connection_new(NULL, 0, 0);

	// XXX: Check which conditions could lead to this being NULL and notify
	// accordingly
	if (connection == NULL) {
		return 1;
	}

	if (mpd_connection_get_error(connection) != MPD_ERROR_SUCCESS) {
		const char *message = mpd_connection_get_error_message(connection);
		mpd_connection_free(connection);

		log_error("MPD connection error: %s", message);
		return 1;
	}

	s->conn = connection;

	return 0;
}

void
seamus_finish(struct seamus_frontend *seamus)
{
	int i;
	for (i = 0; i < seamus->queue_size; i++) {
		struct seamus_song *s = &seamus->queue[i];
		if (s->title != NULL && s->artist != NULL) {
			free(s->title);
			free(s->artist);
		}

		seamus->queue_size--;
	}

	free(seamus->queue);
	free(seamus->current_status);
}

int
seamus_init(struct seamus_frontend *s)
{
	// Setup MPD connection
	int mpd_con = setup_connection(s);
	if (mpd_con != 0) {
		log_error("Weren't able to connect with MPD");

		return 1;
	}
}

int
main(int argc, char *argv[])
{
	struct seamus_frontend seamus = {0};

	int r = seamus_init(&seamus);

	if (r != 0) {
		log_fatal("Couldn't initialize seamus");

		return 1;
	}

	r = tickit_init(&seamus);

	if (r != 0) {
		log_fatal("Something went wrong when initializing tickit");

		goto exit_tickit;
	}

	tickit_start(&seamus);

exit_tickit:
	tickit_finish(&seamus);

exit_mpd:
	seamus_finish(&seamus);

exit:
	return 0;
}
