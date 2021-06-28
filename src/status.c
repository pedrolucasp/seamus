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

	if (mpd_status_get_state(status) == MPD_STATE_PLAY ||
		mpd_status_get_state(status) == MPD_STATE_PAUSE) {

		struct mpd_song *song = mpd_run_current_song(s->conn);

		const char *state;

		if (mpd_status_get_state(status) == MPD_STATE_PLAY) {
			state = "[playing]";
		} else {
			state = "[paused]";
		}

		if (song != NULL) {
			const char *title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
			const char *artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);

			char *elapsed_time = (char*) malloc(13 * sizeof(char));

			sprintf(elapsed_time, "%3i:%02i",
					mpd_status_get_elapsed_time(status) / 60,
					mpd_status_get_elapsed_time(status) % 60);

			char *total_time = (char*) malloc(13 * sizeof(char));

			sprintf(total_time, "%i:%02i",
					mpd_status_get_total_time(status) / 60,
					mpd_status_get_total_time(status) % 60);

			// TODO: This looks kind of bad. Probably have some
			// less stupid way to do it
			size_t strsz = (
				sizeof(char) * (
					strlen(state) +
					strlen(artist) +
					strlen(title) +
					strlen(elapsed_time) +
					strlen(total_time) +
					20
				)
			);

			char *str = malloc(strsz);

			if (s->current_status != NULL) {
				free(s->current_status);
			}

			s->current_status = malloc(strsz);

			sprintf(str, "%s %s - %s: %s/%s",
				state,
				artist,
				title,
				elapsed_time,
				total_time
			);

			strcpy(s->current_status, str);

			free(total_time);
			free(elapsed_time);
			free(str);
			mpd_song_free(song);
		}
	} else {
		const char *stat = "Stopped";

		// This works like a charm, albeit it's ugly:
		s->current_status = malloc(sizeof(char) * strlen(stat));
		strcpy(s->current_status, stat);

		return 0;
	}

	mpd_status_free(status);
	mpd_response_finish(s->conn);

	return 0;
}
