#ifndef __BACKEND_H_
#define __BACKEND_H_

/* backend.h */

#define SONG_FIELD_LEN	400

typedef struct {
	char file[SONG_FIELD_LEN];
	char title[SONG_FIELD_LEN];
	char artist[SONG_FIELD_LEN];
	char album[SONG_FIELD_LEN];
	char track[SONG_FIELD_LEN];
	char name[SONG_FIELD_LEN];
	int time;
	int pos;
} Music_Song;

enum {
	VOLUME_UNAVAILABLE=-1
};

typedef enum {
	STATE_UNKNOWN,
	STATE_STOP,
	STATE_PLAY,
	STATE_PAUSE
} Music_PlayState;

/* a slight hack... we wanna support multiple changes at once */
typedef enum {
	CHANGE_DISCONNECT=-1,
	CHANGE_NONE=0,
	CHANGE_SONG=1,
	CHANGE_VOLUME=2,
	CHANGE_PLAYSTATE=4
} Music_StateChange;

#ifndef __MODULE__

int music_init(const char *backend, const char *options);
Music_StateChange music_checkforstatechange(void);
int music_quit(void);

int music_get_songinfo(Music_Song *s);
int music_get_volume(int *vol);
int music_get_playstate(Music_PlayState *p);

#endif

#endif /* __BACKEND_H_ */
