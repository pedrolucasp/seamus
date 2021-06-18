#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <tickit.h>
#include <string.h>
#include <assert.h>
#include <mpd/client.h>

struct seamus_song {
	char *title;
	char *artist;
};

struct seamus_frontend {
	struct mpd_connection *conn;
	struct seamus_song *queue;
	size_t queue_size;
	TickitWindow *main_window;
	Tickit *t;
};

int
setup_connection(struct seamus_frontend *s)
{
	struct mpd_connection *connection = mpd_connection_new(NULL, 0, 0);

	// XXX: Check which conditions could lead to this being NULL and notify
	// accordingly
	if (connection == NULL) {
		return 1;
	}

	if (mpd_connection_get_error(connection) != MPD_ERROR_SUCCESS) {
		const char *message = mpd_connection_get_error_message(connection);
		mpd_connection_free(connection);

		fprintf(stderr, "MPD connection error: %s\n", message);
		return 1;
	}

	s->conn = connection;
	return 0;
}

int
fetch_mpd_from_current_queue(struct seamus_frontend *seamus, int max_count)
{
	assert(seamus->conn != NULL);
	int queue_status = mpd_send_list_queue_meta(seamus->conn);

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
		fprintf(stderr, "Could not fetch queue");
		return 1;
	}
}

void
print_songs_from_queue(struct seamus_frontend *seamus)
{
	for (size_t i = 0; i < seamus->queue_size; ++i) {
		struct seamus_song *s = &seamus->queue[i];

		if (s == NULL) {
			printf("Nothing here...");
		} else {
			printf("Song queued: %s - %s \n", s->artist, s->title);
		}
	}
}

void
seamus_finish(struct seamus_frontend *seamus)
{
	int i;
	for (i = 0; i < seamus->queue_size; i++) {
		struct seamus_song *s = &seamus->queue[i];
		if (s->title != NULL && s->artist != NULL) {
			free(s->title);
			free(s->artist);
		}

		seamus->queue_size--;
	}

	free(seamus->queue);
}

int
seamus_init(struct seamus_frontend *s)
{
	// Setup MPD connection
	int mpd_con = setup_connection(s);
	if (mpd_con != 0) {
		// replace with a unified log
		fprintf(stderr, "Weren't able to connect with MPD");
		return 1;
	}
}

int
main(int argc, char *argv[])
{
	struct seamus_frontend seamus = {0};

	seamus_init(&seamus);

	fetch_mpd_from_current_queue(&seamus, 10);

	print_songs_from_queue(&seamus);

	//return 0;

	/*
	t = tickit_new_stdtty();

	TickitWindow *root = tickit_get_rootwin(t);

	if (!root) {
		fprintf(stderr, "Cannot create TickitTerm - %d\n", strerror(errno));
		return 1;
	}

	main_window = tickit_window_new(root, (TickitRect){
		.top = 2, .left = 2, .lines = tickit_window_lines(root) - 5,
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
	*/
}
