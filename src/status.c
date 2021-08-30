#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "seamus.h"

int
fetch_current_status(struct seamus_frontend *s)
{
	assert(s->conn != NULL);

	struct mpd_status *status = mpd_run_status(s->conn);

	if (status == NULL) {
		const char *message = mpd_connection_get_error_message(s->conn);
		log_error("MPD Error - No Status: %s", message);

		return 1;
	}

	// TODO: create or populate this mf
	if (s->status == NULL) {
		struct seamus_status *s_status = calloc(1, sizeof(struct seamus_status));
		memset(&s_status->description, 0, sizeof(char));
		s->status = s_status;
	}

	s->status->repeat = mpd_status_get_repeat(status);
	s->status->version = mpd_status_get_queue_version(status);
	s->status->length = mpd_status_get_queue_length(status);
	s->status->current_song_position = mpd_status_get_song_pos(status);
	s->status->current_song_id = mpd_status_get_song_id(status);
	s->status->state = mpd_status_get_state(status);
	s->status->elapsed_time = mpd_status_get_elapsed_time(status);
	s->status->total_time = mpd_status_get_total_time(status);

	mpd_response_finish(s->conn);

	struct mpd_song *song = mpd_run_current_song(s->conn);
	if (song != NULL) {
		int desc = generate_description(s->status, song);

		if (desc != 0) {
			log_debug("Something went off, when generate description");
		}
	}

	mpd_status_free(status);
	mpd_response_finish(s->conn);

	return 0;
}

int
generate_description(struct seamus_status *status, struct mpd_song *song)
{

	const char *title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	const char *artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);

	char *elapsed_time = (char*) malloc(13 * sizeof(char));

	sprintf(elapsed_time, "%3i:%02i",
			status->elapsed_time / 60,
			status->elapsed_time % 60);

	char *total_time = (char*) malloc(13 * sizeof(char));

	sprintf(total_time, "%i:%02i",
			status->total_time / 60,
			status->total_time % 60);

	// TODO: This looks kind of bad. Probably have some
	// less stupid way to do it
	size_t strsz = (
			sizeof(char) * (
				strlen(artist) +
				strlen(title) +
				strlen(elapsed_time) +
				strlen(total_time) +
				20
				)
		       );

	char *str = malloc(strsz);

	if (status->description != NULL) {
		free(status->description);
	}

	status->description = malloc(strsz);

	sprintf(str, "%s - %s: %s/%s",
			artist,
			title,
			elapsed_time,
			total_time
	       );

	strcpy(status->description, str);

	free(total_time);
	free(elapsed_time);
	free(str);
	mpd_song_free(song);

	return 0;
//} else {
//	const char *stat = "Stopped";
//
//	// This works like a charm, albeit it's ugly:
//	status->description = malloc(sizeof(char) * strlen(stat));
//	strcpy(status->description, stat);
//
//	return 0;
//}

//return 1;
}
