#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "seamus.h"

int
fetch_current_queue(struct seamus_frontend *seamus, int max_count)
{
	assert(seamus->conn != NULL);

	bool queue_status = mpd_send_list_queue_meta(seamus->conn);

	if (queue_status == true) {
		struct mpd_entity *entity;
		int index;

		seamus->queue = calloc(max_count, sizeof(struct seamus_song));

		for (index = 0; index < max_count; index++) {
			entity = mpd_recv_entity(seamus->conn);
			if (entity == NULL) {
				continue;
			} else {
				enum mpd_entity_type type = mpd_entity_get_type(entity);

				if (type == MPD_ENTITY_TYPE_SONG) {
					struct mpd_song *song = mpd_entity_get_song(entity);

					enum mpd_tag_type tag_title = mpd_tag_name_iparse("Title");
					const char *stitle = mpd_song_get_tag(song, tag_title, 0);

					enum mpd_tag_type tag_artist = mpd_tag_name_iparse("AlbumArtist");
					const char *sartist = mpd_song_get_tag(song, tag_artist, 0);

					struct seamus_song *new = &seamus->queue[index];
					memset(new, 0, sizeof(*new));

					char *title = malloc(sizeof(char) * strlen(stitle) + 1);
					strcpy(title, stitle);

					char *artist = malloc(sizeof(char) * strlen(sartist) + 1);
					strcpy(artist, sartist);

					new->title = title;
					new->artist = artist;
					seamus->queue_size++;
				}

				// When freeing the entity, it'll automatically
				// free the song for us
				mpd_entity_free(entity);
			}
		}

		return 0;
	} else {
		log_fatal("Couldn't fetch the queue");
		return 1;
	}
}
