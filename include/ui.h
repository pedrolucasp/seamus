#ifndef SEAMUS_UI
#define SEAMUS_UI
#include <errno.h>
#include <tickit.h>
#include "seamus.h"

int tickit_init(struct seamus_frontend *s);
int tickit_start(struct seamus_frontend *s);
int tickit_finish(struct seamus_frontend *s);

static int on_key_event(TickitTerm *t, TickitEventFlags flags, void *_info, void *data);
static int update_scroll_position(struct seamus_frontend *seamus, int direction);
static int update_status(Tickit *t, TickitEventFlags flags, void *_info, void *data);
static int render_main_window(TickitWindow *win, TickitEventFlags flags, void *_info, void *data);
static int render_status_window(TickitWindow *win, TickitEventFlags flags, void *_info, void *data);
static int render_root(TickitWindow *win, TickitEventFlags flags, void *_info, void *data);
#endif
