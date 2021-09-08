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
		.top = 2, .left = 2, .lines = tickit_window_lines(root) - 6,
		.cols = tickit_window_cols(root) - 2
	}, 0);

	TickitWindow *status_window = tickit_window_new(root, (TickitRect){
		.top = tickit_window_lines(root) - 5 + 2, .left = 2, .lines = 5,
		.cols = tickit_window_cols(root) - 2
	}, 0);

	s->scrolling_pen = tickit_pen_new_attrs(
		TICKIT_PEN_BG, 3,
		TICKIT_PEN_FG, 0,
		0
	);

	s->playing_pen = tickit_pen_new_attrs(
		TICKIT_PEN_BOLD, 1,
		0
	);

	s->main_window = main_window;
	s->status_window = status_window;

	return 0;
}

int
tickit_start(struct seamus_frontend *s)
{
	TickitWindow *root = tickit_get_rootwin(s->t);
	TickitTerm *tt = tickit_get_term(s->t);

	tickit_window_bind_event(s->status_window, TICKIT_WINDOW_ON_EXPOSE, 0, &render_status_window, s);
	tickit_window_bind_event(s->main_window, TICKIT_WINDOW_ON_EXPOSE, 0, &render_main_window, s);
	tickit_window_bind_event(root, TICKIT_WINDOW_ON_EXPOSE, 0, &render_root, s);

	tickit_term_bind_event(tt, TICKIT_TERM_ON_KEY, 0, &on_key_event, s);

	// Kick initial update event
	tickit_watch_timer_after_msec(s->t, 1000, 0, &update_status, s);
	//tickit_watch_timer_after_msec(s->t, 1000, 0, &update_main_window, s);

	tickit_run(s->t);
}

static int
on_key_event(TickitTerm *tt, TickitEventFlags flags, void *_info, void *data)
{
	TickitKeyEventInfo *info = _info;

	struct seamus_frontend *seamus = (struct seamus_frontend*) data;
	const char *key_pressed = info->str;

	if (strcmp(key_pressed, "q") == 0) {
		tickit_window_close(seamus->main_window);
		tickit_window_close(seamus->status_window);

		tickit_stop(seamus->t);
		//tickit_finish(seamus);

	} else if (strcmp(key_pressed, "j") == 0) {
		log_info("Scroll down");
		// TODO: Replace with an enum
		update_scroll_position(seamus, +1);
	} else if (strcmp(key_pressed, "k") == 0) {
		log_info("Scroll up");
		// TODO: Replace with an enum
		update_scroll_position(seamus, -1);
	} else {
		log_info("Pressed something else %s", key_pressed);
	}

	return 1;
}

static int
update_scroll_position(struct seamus_frontend *seamus, int direction)
{
	log_info("Current position %d, direction: %d, length: %d", seamus->scroll_position, direction, seamus->status->length);

	if (seamus->scroll_position == 0 && direction == -1) {
		return 0;
	}

	// Can't go further than whats queued
	// TODO: This will cause problems when dealing with the library
	// scrolling
	if (seamus->scroll_position == seamus->status->length - 1 && direction == 1) {
		return 0;
	}

	seamus->scroll_position += direction;
	// TODO: Deal with the current window focused
	tickit_window_scroll(seamus->main_window, direction, 0);
}

static int
update_status(Tickit *t, TickitEventFlags flags, void *_info, void *data)
{
	struct seamus_frontend *seamus = (struct seamus_frontend*) data;

	tickit_window_expose(seamus->status_window, NULL);
	tickit_watch_timer_after_msec(t, 1000, 0, &update_status, data);

	return 0;
}

static int
update_main_window(Tickit *t, TickitEventFlags flags, void *_info, void *data)
{
	struct seamus_frontend *seamus = (struct seamus_frontend*) data;

	tickit_window_expose(seamus->main_window, NULL);
	tickit_watch_timer_after_msec(t, 1000, 0, &update_main_window, data);

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
		tickit_renderbuffer_goto(render_buffer, 0, 0);
		{
			tickit_renderbuffer_savepen(render_buffer);

			TickitPen *pen = tickit_pen_new_attrs(
					TICKIT_PEN_FG, 4,
					TICKIT_PEN_BOLD, 1,
					0);

			tickit_renderbuffer_setpen(render_buffer, pen);
			tickit_renderbuffer_text(render_buffer, seamus->status->description);
			tickit_renderbuffer_restore(render_buffer);
		}
	}

	return 1;
}

static int
render_main_window(TickitWindow *win, TickitEventFlags flags, void *_info, void *data)
{
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

	if (seamus->status->length == 0) {
		tickit_renderbuffer_goto(render_buffer, 4, 0);
		tickit_renderbuffer_text(render_buffer, "No songs queued.");
	} else {
		log_info("There are items on queue");
		if (seamus->status->version != seamus->version) {
			log_info(
				"The current rendered version is %d and there's a new one %d",
				seamus->version,
				seamus->status->version
			);

			// Update the current version rendered
			seamus->version = seamus->status->version;
			int max_window_size = tickit_window_lines(seamus->main_window);
			fetch_current_queue(seamus, max_window_size);
		}

		log_info("Rendering queue again");
		render_queue(seamus, render_buffer);
	}

	return 1;
}

static int
render_queue(struct seamus_frontend *seamus, TickitRenderBuffer *render_buffer)
{
	for (size_t i = 0; i < seamus->queue_size; ++i) {
		struct seamus_song *song = &seamus->queue[i];

		if (song == NULL) {
			log_info("Nothing to see here...");
		} else {
			int left_padding = 0;
			char *song_str = malloc(
					sizeof(char) *
					(strlen(song->artist) + strlen(song->title) + 6));

			sprintf(song_str, "%s - %s", song->artist, song->title);

			if (seamus->status->current_song_id == song->song_id) {
				left_padding = 2;
			}

			tickit_renderbuffer_goto(render_buffer, 4 + i, left_padding);
			if (seamus->status->current_song_id == song->song_id && seamus->scroll_position == i) {
				tickit_renderbuffer_savepen(render_buffer);
				TickitPen *scrolling_playing_pen = tickit_pen_clone(seamus->scrolling_pen);
				tickit_pen_copy_attr(scrolling_playing_pen, seamus->playing_pen, TICKIT_PEN_BOLD);

				tickit_renderbuffer_setpen(render_buffer, scrolling_playing_pen);
				tickit_renderbuffer_text(render_buffer, song_str);
				tickit_renderbuffer_restore(render_buffer);
			} else if (seamus->status->current_song_id == song->song_id) {
				tickit_renderbuffer_savepen(render_buffer);
				tickit_renderbuffer_setpen(render_buffer, seamus->playing_pen);
				tickit_renderbuffer_text(render_buffer, song_str);
				tickit_renderbuffer_restore(render_buffer);
			} else if (seamus->scroll_position == i) {
				tickit_renderbuffer_savepen(render_buffer);
				tickit_renderbuffer_setpen(render_buffer, seamus->scrolling_pen);
				tickit_renderbuffer_text(render_buffer, song_str);
				tickit_renderbuffer_restore(render_buffer);
			} else {
				tickit_renderbuffer_text(render_buffer, song_str);
			}



		//	if (seamus->status->current_song_id == song->song_id) {
		//		tickit_renderbuffer_goto(render_buffer, 4 + i, 2);
		//		{
		//			tickit_renderbuffer_savepen(render_buffer);

		//			tickit_renderbuffer_setpen(render_buffer, seamus->playing_pen);
		//			tickit_renderbuffer_text(render_buffer, song_str);
		//			tickit_renderbuffer_restore(render_buffer);
		//		}
		//	} else {
		//		tickit_renderbuffer_goto(render_buffer, 4 + i, 0);
		//		tickit_renderbuffer_text(render_buffer, song_str);
		//	}

			free(song_str);
		}
	}
}

int
tickit_finish(struct seamus_frontend *s)
{
	TickitWindow *root = tickit_get_rootwin(s->t);

	tickit_window_close(root);
	tickit_unref(s->t);

	return 0;
}
