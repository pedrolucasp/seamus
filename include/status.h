#ifndef SEAMUS_STATUS
#define SEAMUS_STATUS
#include "seamus.h"

int generate_description(struct seamus_status *seamus, struct mpd_status *status);
int fetch_current_status(struct seamus_frontend *seamus);
#endif
