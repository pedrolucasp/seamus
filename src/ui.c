#include <stdio.h>
#include "ui.h"

int
tickit_init(struct seamus_frontend *s)
{

	Tickit *t = tickit_new_stdtty();
	TickitWindow *root = tickit_get_rootwin(t);

	if (!root) {
		log_debug("Cannot create TickitTerm - %d\n", strerror(errno));
		return 1;
	}

	s->t = t;

	TickitWindow *main_window = tickit_window_new(root, (TickitRect){
		.top = 2, .left = 2, .lines = tickit_window_lines(root) - 2,
		.cols = tickit_window_cols(root) - 2
	}, 0);

	s->main_window = main_window;

	return 0;
}

int
tickit_finish(struct seamus_frontend *s)
{
	TickitWindow *root = tickit_get_rootwin(s->t);

	tickit_window_close(root);
	tickit_unref(s->t);

	return 0;
}
