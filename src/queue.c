#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "seamus.h"

int
fetch_current_queue(struct seamus_frontend *seamus, int max_count)
{
	assert(seamus->conn != NULL);

	bool queue_status = mpd_send_list_queue_meta(seamus->conn);

	if (queue_status) {
		struct mpd_entity *entity;
		int index = 0;
		int max_items = 0;

		// XXX: The logic here is. We check the amount of items queued.
		// Then we have the info o how many items we can fit into the
		// screen. The decision we need to make here is: if the amount
		// of items queued is larger than what we can display on the
		// screen, alloc up to the number of items on screen (we can't
		// show more than that anyway). If the number of items queued is
		// lesser than the amount of rows we have available, only alloc
		// up to the number of items on queue.

		if (seamus->status->length > max_count) {
			seamus->queue = calloc(max_count, sizeof(struct seamus_song));
			max_items = max_count;
		} else {
			seamus->queue = calloc(seamus->status->length, sizeof(struct seamus_song));
			max_items = seamus->status->length;
		}

		seamus->queue_size = max_items;

		while (index <= max_items) {
			entity = mpd_recv_entity(seamus->conn);

			if (entity == NULL) {
				// TODO: Discover if the queue have ended, or we
				// have received an error here
				break;

				return 0;
			} else {
				enum mpd_entity_type type = mpd_entity_get_type(entity);

				if (type == MPD_ENTITY_TYPE_SONG) {
					struct mpd_song *song = mpd_entity_get_song(entity);

					const char *stitle = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
					const char *sartist = mpd_song_get_tag(song, MPD_TAG_ALBUM_ARTIST, 0);

					struct seamus_song *new = &seamus->queue[index];
					memset(new, 0, sizeof(*new));

					char *title = malloc(sizeof(char) * strlen(stitle) + 1);
					strcpy(title, stitle);

					char *artist = malloc(sizeof(char) * strlen(sartist) + 1);
					strcpy(artist, sartist);

					new->song_id = mpd_song_get_id(song);
					new->title = title;
					new->artist = artist;
				}

				// When freeing the entity, it'll automatically
				// free the song for us
				mpd_entity_free(entity);
			}

			index++;
		}

		mpd_response_finish(seamus->conn);

		return 0;
	} else {
		const char *message = mpd_connection_get_error_message(seamus->conn);
		log_error("MPD Error - Queue: %s", message);

		mpd_response_finish(seamus->conn);

		return 1;
	}
}
