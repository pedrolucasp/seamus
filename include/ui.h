#ifndef SEAMUS_UI
#define SEAMUS_UI
#include <errno.h>
#include <tickit.h>
#include "seamus.h"

int tickit_init(struct seamus_frontend *s);
int tickit_finish(struct seamus_frontend *s);
#endif
