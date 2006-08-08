/* mod-mpd.c - MPD backend module =) */
#include <stdio.h>
#include <string.h>

#define __MODULE__ /* Hi, I'm a module! */
#include "backend.h"
#include "backend-module.h"

#include "xmmsctrl.h"

/* Module Meta Info */
MOD_DECLARE()
MOD_NAME("XMMS Backend Module")
MOD_VERSION("0.1")
MOD_AUTHOR("Lucas Martin-King")
MOD_COPYRIGHT("(C) 2006")

#include "utils.h"

/* Globals */
static int xs = 0;	/* xmms session */

/* Prototypes */
//int mpd_init(int n, Module_Param **parms);
//int mpd_check(void);
//int mpd_get(Module_Request request, void *data);
//int mpd_quit(void);

static int xmms_getsonginfo(Music_Song *s);
static int xmms_getvolume(int *vol);
static int xmms_getplaystate(Music_PlayState *sta);

/* Exports */
MOD_FN_INIT(xmms_init)
MOD_FN_POLL(xmms_check)
MOD_FN_GET(xmms_get)
MOD_FN_QUIT(xmms_quit)

static void
xmms_find_session(void)
{
	int i;

	for (i = 0 ; i < 16 ; i++) {
		if (xmms_remote_is_running(i)) {
			xs = i;
			break;
		}
	}
	
	return;
}
	
/*** INIT ***/
int xmms_init(int n, Module_Param **parms)
{
	Module_Param *p;
	int i;

	xmms_find_session();

	for (i = 0; i < n; i++) {
		p = parms[i];

		if (strcmp(p->name, "xmms_session") == 0) {
			xs = atoi(p->value);
		}
	}

	if (xmms_remote_is_running(xs))
		return MOD_OKAY;
	else
		return MOD_FAIL;
}

static int list_pos=-1;
static int volume=-1;
static Music_PlayState state;

/*** POLL ***/
int xmms_check(void)
{
	int n_list_pos;
	int n_volume;
	Music_PlayState n_state;

	int change = CHANGE_NONE;

	if (!xmms_remote_is_running(xs)) {
		change = CHANGE_DISCONNECT;
	} else {
		n_list_pos = xmms_remote_get_playlist_pos(xs);
		n_volume   = xmms_remote_get_main_volume(xs);
		xmms_getplaystate(&n_state);

		if (list_pos == -1) {
			change = CHANGE_NONE;
		} else {
			if (n_list_pos != list_pos) {
				change |= CHANGE_SONG;
			}
			if (n_volume != volume) {
				change |= CHANGE_VOLUME;
			}
			if (n_state != state) {
				change |= CHANGE_PLAYSTATE;
			}
		}

		list_pos = n_list_pos;
		volume   = n_volume;
		state    = n_state;
	}
	
	return change;
}

/*** GET ***/
int xmms_get(Module_Request request, void *data)
{
	if (!data)
		return MOD_FAIL;
	
	switch (request) {
		case GET_SONGINFO:
			return xmms_getsonginfo((Music_Song *)data);

		case GET_VOLUME:
			return xmms_getvolume((int *)data);

		case GET_PLAYSTATE:
			return xmms_getplaystate((Music_PlayState *)data);

		default:
			return MOD_FAIL;
	}

	return MOD_FAIL;
}
	
/*** QUIT ***/
int xmms_quit(void)
{
	INFO("Closing connection...\n");
	
	return MOD_OKAY;
}

/****************** INTERNALS *********************/

static
int xmms_getsonginfo(Music_Song *s)
{
	int ret;

	int pos;
	char *title;
	char *file;
	
	if (s == NULL)
		return MOD_FAIL;

	#define SAFE_CLONE(src, dest)	if (src) strcpy(dest, src)

	pos = xmms_remote_get_playlist_pos(xs);
	
	title = xmms_remote_get_playlist_title(xs, pos);
	file  = xmms_remote_get_playlist_file (xs, pos);

	SAFE_CLONE(title, s->title);
	SAFE_CLONE(file,  s->file);

	s->time = xmms_remote_get_playlist_time(xs, pos) / 1000;
	s->pos  = xmms_remote_get_output_time(xs) / 1000;

	ret = MOD_OKAY;

	return ret;
}

static
int xmms_getvolume(int *vol)
{
	int ret;

	if (!vol)
		return MOD_FAIL;

	*vol = xmms_remote_get_main_volume(xs);

	return ret;
}

static int xmms_getplaystate(Music_PlayState *sta)
{
	if (!sta)
		return MOD_FAIL;

	if (xmms_remote_is_playing(xs)) {
		*sta = STATE_PLAY;
	} else
	if (xmms_remote_is_paused(xs)) {
		*sta = STATE_PAUSE;
	} else {
		*sta = STATE_STOP;
	}

	return MOD_OKAY;
}
