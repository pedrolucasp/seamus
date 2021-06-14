#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <tickit.h>
#include <poll.h>
#include <mpd/client.h>

TickitWindow *main_window;
Tickit *t;

static struct mpd_connection *setup_connection(void)
{
	struct mpd_connection *connection = mpd_connection_new(NULL, 0, 0);

	if (connection == NULL) {
		fprintf(stderr, "Couldn't establish a connection!");
		tickit_stop(t);
		tickit_unref(t);
		exit(1);
	}

	if (mpd_connection_get_error(connection) != MPD_ERROR_SUCCESS) {
		const char *message = mpd_connection_get_error_message(connection);
		mpd_connection_free(connection);

		fprintf(stderr, "seamus error: %s\n", message);
		tickit_stop(t);
		tickit_unref(t);
		exit(1);
	}

	return connection;
}

void fetch_mpd_status(struct mpd_connection *connection, char *str)
{
	struct mpd_status *status = mpd_run_status(connection);

	if (status == NULL) {
		const char *message = mpd_connection_get_error_message(connection);
		fprintf(stderr, "MPD Error - No Status: %s\n", message);
		tickit_stop(t);
		tickit_unref(t);
		exit(1);
	}

	if (mpd_status_get_state(status) == MPD_STATE_PLAY ||
		mpd_status_get_state(status) == MPD_STATE_PAUSE) {

		struct mpd_song *song = mpd_run_current_song(connection);

		const char *state;

		if (mpd_status_get_state(status) == MPD_STATE_PLAY) {
			state = "[playing]";
		} else {
			state = "[paused]";
		}

		if (song != NULL) {
			enum mpd_tag_type tag_artist = mpd_tag_name_iparse("Artist");
			enum mpd_tag_type tag_title = mpd_tag_name_iparse("Title");

			const char *title = mpd_song_get_tag(song, tag_title, 0);
			const char *artist = mpd_song_get_tag(song, tag_artist, 0);

			char *elapsed_time = (char*) malloc(13 * sizeof(char));

			sprintf(elapsed_time, "%3i:%02i",
					mpd_status_get_elapsed_time(status) / 60,
					mpd_status_get_elapsed_time(status) % 60);

			char *total_time = (char*) malloc(13 * sizeof(char));

			sprintf(total_time, "%i:%02i",
					mpd_status_get_total_time(status) / 60,
					mpd_status_get_total_time(status) % 60);

			// Move this into a proper struct
			asprintf(str, "%s %s - %s: %s/%s",
				state,
				artist,
				title,
				elapsed_time,
				total_time
			);

			free(total_time);
			free(elapsed_time);
			mpd_song_free(song);
		}
	}

	mpd_status_free(status);
}

static int update(Tickit *t, TickitEventFlags flags, void *_info, void *user);
static int update(Tickit *t, TickitEventFlags flags, void *_info, void *user) {
	tickit_window_expose(main_window, NULL);
	tickit_watch_timer_after_msec(t, 1000, 0, &update, NULL);

	return 0;
}

static int render(TickitWindow *win, TickitEventFlags flags, void *_info, void *data)
{
	TickitExposeEventInfo *info = _info;
	TickitRenderBuffer *render_buffer = info->rb;
	struct mpd_connection *connection = setup_connection();

	tickit_renderbuffer_eraserect(render_buffer, &info->rect);

	tickit_renderbuffer_goto(render_buffer, 0, 0);
	{
		tickit_renderbuffer_savepen(render_buffer);

		TickitPen *pen = tickit_pen_new_attrs(
			TICKIT_PEN_FG, 1,
			TICKIT_PEN_BOLD, 1,
		0);

		tickit_renderbuffer_setpen(render_buffer, pen);
		tickit_renderbuffer_text(render_buffer, "Hello!");
		tickit_renderbuffer_restore(render_buffer);
	}

	char *current_status;
	fetch_mpd_status(connection, &current_status);

	tickit_renderbuffer_goto(render_buffer, 2, 0);
	tickit_renderbuffer_text(render_buffer, current_status);

	return 1;
}

static int render_root(TickitWindow *win, TickitEventFlags flags, void *_info, void *data)
{
	TickitExposeEventInfo *info = _info;
	TickitRenderBuffer *render_buffer = info->rb;

	int right = tickit_window_cols(win) - 1;
	int bottom = tickit_window_lines(win) - 1;

	tickit_renderbuffer_eraserect(render_buffer, &(TickitRect){
		.top = 0, .left = 0, .lines = bottom+1, .cols = right+1,
	});

	return 1;
}

void debug(char *message)
{
	fprintf(stderr, "DEBUG: %s \n", message);
}

int main(int argc, char *argv[])
{
	t = tickit_new_stdtty();

	TickitWindow *root = tickit_get_rootwin(t);

	if (!root) {
		fprintf(stderr, "Cannot create TickitTerm - %d\n", strerror(errno));
		return 1;
	}

	main_window = tickit_window_new(root, (TickitRect){
		.top = 8, .left = 2, .lines = 3,
		.cols = tickit_window_cols(root) - 7
	}, 0);

	tickit_window_bind_event(main_window, TICKIT_WINDOW_ON_EXPOSE, 0, &render, NULL);
	tickit_window_bind_event(root, TICKIT_WINDOW_ON_EXPOSE, 0, &render_root, NULL);

	// Initial update
	tickit_watch_timer_after_msec(t, 1000, 0, &update, NULL);

	tickit_run(t);
	tickit_window_close(root);
	tickit_unref(t);
	return 0;
}
