#include <assert.h>
#include <stdio.h>
#include "ui.h"
#include "queue.h"
#include "status.h"

int
tickit_init(struct seamus_frontend *s)
{

	Tickit *t = tickit_new_stdtty();
	TickitWindow *root = tickit_get_rootwin(t);

	if (!root) {
		log_error("Cannot create TickitTerm - %d\n", strerror(errno));
		return 1;
	}

	s->t = t;

	TickitWindow *main_window = tickit_window_new(root, (TickitRect){
		.top = 2, .left = 2, .lines = tickit_window_lines(root) - 5,
		.cols = tickit_window_cols(root) - 2
	}, 0);

	TickitWindow *status_window = tickit_window_new(root, (TickitRect){
		.top = tickit_window_lines(root) - 5, .left = 2, .lines = 5,
		.cols = tickit_window_cols(root) - 2
	}, 0);

	s->main_window = main_window;
	s->status_window = status_window;

	return 0;
}

int
tickit_start(struct seamus_frontend *s)
{
	TickitWindow *root = tickit_get_rootwin(s->t);

	tickit_window_bind_event(s->main_window, TICKIT_WINDOW_ON_EXPOSE, 0, &render_main_window, s);
	tickit_window_bind_event(s->status_window, TICKIT_WINDOW_ON_EXPOSE, 0, &render_status_window, s);
	tickit_window_bind_event(root, TICKIT_WINDOW_ON_EXPOSE, 0, &render_root, s);

	// Kick initial update event
	tickit_watch_timer_after_msec(s->t, 1000, 0, &update_status, s);

	tickit_run(s->t);
}

static int
update_status(Tickit *t, TickitEventFlags flags, void *_info, void *user)
{
	struct seamus_frontend *seamus = (struct seamus_frontend*) user;

	tickit_window_expose(seamus->status_window, NULL);
	tickit_watch_timer_after_msec(t, 1000, 0, &update_status, user);

	return 0;
}

static int
render_root(TickitWindow *win, TickitEventFlags flags, void *_info, void *data)
{
	TickitExposeEventInfo *info = _info;
	TickitRenderBuffer *render_buffer = info->rb;
	struct seamus_frontend *seamus = (struct seamus_frontend*) data;

	int right = tickit_window_cols(win) - 1;
	int bottom = tickit_window_lines(win) - 1;

	tickit_renderbuffer_eraserect(render_buffer, &(TickitRect){
		.top = 0, .left = 0, .lines = bottom + 1, .cols = right + 1,
	});

	return 1;
}

static int
render_status_window(TickitWindow *win, TickitEventFlags flags, void *_info, void *data)
{
	log_info("Bootstraping status window");
	TickitExposeEventInfo *info = _info;
	TickitRenderBuffer *render_buffer = info->rb;
	struct seamus_frontend *seamus = (struct seamus_frontend*) data;

	tickit_renderbuffer_eraserect(render_buffer, &info->rect);

	int r = fetch_current_status(seamus);

	if (r != 0) {
		log_fatal("We had some trouble");

		tickit_renderbuffer_goto(render_buffer, 0, 0);
		{
			tickit_renderbuffer_savepen(render_buffer);

			TickitPen *pen = tickit_pen_new_attrs(
					TICKIT_PEN_FG, 1,
					TICKIT_PEN_BOLD, 1,
					0);

			tickit_renderbuffer_setpen(render_buffer, pen);
			tickit_renderbuffer_text(render_buffer, "We had some error fetching the status");
			tickit_renderbuffer_restore(render_buffer);
		}

	} else {
		log_debug("Should have returned %d", r);
		log_debug("Status: %s", seamus->current_status);

		tickit_renderbuffer_goto(render_buffer, 0, 0);
		{
			tickit_renderbuffer_savepen(render_buffer);

			TickitPen *pen = tickit_pen_new_attrs(
					TICKIT_PEN_FG, 4,
					TICKIT_PEN_BOLD, 1,
					0);

			tickit_renderbuffer_setpen(render_buffer, pen);
			tickit_renderbuffer_text(render_buffer, seamus->current_status);
			tickit_renderbuffer_restore(render_buffer);
		}
	}

	return 1;
}

static int
render_main_window(TickitWindow *win, TickitEventFlags flags, void *_info, void *data)
{
	log_info("Starting the rendering of main window");

	TickitExposeEventInfo *info = _info;
	TickitRenderBuffer *render_buffer = info->rb;
	struct seamus_frontend *seamus = (struct seamus_frontend*) data;

	tickit_renderbuffer_eraserect(render_buffer, &info->rect);

	tickit_renderbuffer_goto(render_buffer, 0, 0);
	{
		tickit_renderbuffer_savepen(render_buffer);

		TickitPen *pen = tickit_pen_new_attrs(
			TICKIT_PEN_FG, 0,
			TICKIT_PEN_BG, 6,
			TICKIT_PEN_BOLD, 1,
		0);

		tickit_renderbuffer_setpen(render_buffer, pen);
		tickit_renderbuffer_text(render_buffer, "Hello, welcome to seamus");
		tickit_renderbuffer_restore(render_buffer);
	}

	int max_songs = tickit_window_lines(win) - 5;
	log_debug("Max songs allowed: %d of total %d lines", max_songs, tickit_window_lines(win));

	fetch_current_queue(seamus, max_songs);

	if (seamus->queue_size > 0) {
		for (size_t i = 0; i < seamus->queue_size; ++i) {
			struct seamus_song *s = &seamus->queue[i];

			if (s == NULL) {
				log_info("Nothing here...");
			} else {
				char *song_str = malloc(
						sizeof(char) *
						(strlen(s->artist) + strlen(s->title) + 4));

				sprintf(song_str, "%s - %s", s->artist, s->title);

				tickit_renderbuffer_goto(render_buffer, 4 + i, 0);
				tickit_renderbuffer_text(render_buffer, song_str);

				log_debug("Song queued: %d: %s\n", i, song_str);
				free(song_str);
			}
		}
	} else {
		tickit_renderbuffer_goto(render_buffer, 4, 0);
		tickit_renderbuffer_text(render_buffer, "No songs queued.");
	}

	return 1;
}

int
tickit_finish(struct seamus_frontend *s)
{
	TickitWindow *root = tickit_get_rootwin(s->t);

	tickit_window_close(root);
	tickit_unref(s->t);

	return 0;
}
