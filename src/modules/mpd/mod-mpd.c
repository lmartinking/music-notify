/* mod-mpd.c - MPD backend module =) */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define __MODULE__ /* Hi, I'm a module! */
#include "backend.h"
#include "backend-module.h"

/* Module Meta Info */
MOD_DECLARE()
MOD_NAME("MPD Backend Module")
MOD_VERSION("0.1")
MOD_AUTHOR("Lucas Martin-King")
MOD_COPYRIGHT("(C) 2006")

#include "libmpdclient.h"
#include "utils.h"

/* Globals */
static char	mpd_host[100];
static int	mpd_port;
static int	mpd_timeout = 10;
static mpd_Connection *conn = NULL;

/* Prototypes */
int mpd_init(int n, Module_Param **parms);
int mpd_check(void);
int mpd_get(Module_Request request, void *data);
int mpd_quit(void);

static int mpd_getsonginfo(Music_Song *s);
static int mpd_getvolume(int *vol);
static int mpd_getplaystate(Music_PlayState *sta);

/* Exports */
MOD_FN_INIT(mpd_init)
MOD_FN_POLL(mpd_check)
MOD_FN_GET(mpd_get)
MOD_FN_QUIT(mpd_quit)

static int mpd_connect(void)
{
	INFO("Trying to connect to mpd (%s:%d)... ", mpd_host, mpd_port);

	if (conn != NULL)
		mpd_closeConnection(conn);
	
	conn = mpd_newConnection(mpd_host, mpd_port,
					(float)mpd_timeout);
	if (conn->error) {
		ERROR("Failed to connect to mpd: %s\n", conn->errorStr);
		mpd_closeConnection(conn);
		return 0;
	} else {
		INFO("Connected!\n");
		return 1;
	}
}

/*** INIT ***/
int mpd_init(int n, Module_Param **parms)
{
	Module_Param *p;
	int i;

	for (i = 0; i < n; i++) {
		p = parms[i];

		if (strcmp(p->name, "mpd_host") == 0) {
			strcpy(mpd_host, p->value);
		}

		if (strcmp(p->name, "mpd_port") == 0) {
			mpd_port = atoi(p->value);
		}
	}

	if (mpd_connect())
		return MOD_OKAY;
	else
		return MOD_FAIL;
}

/*** POLL ***/
int mpd_check(void)
{
	static mpd_Status *old;
	static mpd_Status *new;
	int change = CHANGE_NONE;

	mpd_sendStatusCommand(conn);	

	if (conn->error) {
		WARN("MPD Error: %s\n", conn->errorStr);
		change = CHANGE_DISCONNECT;
	} else {
		new = mpd_getStatus(conn);

		if (!old) { /* assume first time? */
			change = CHANGE_NONE;
		} else {
			if (old->songid != new->songid) {
				change |= CHANGE_SONG;
			}
			if (old->state != new->state) {
				change |= CHANGE_PLAYSTATE;
			}
			if (old->volume != new->volume) {
				change |= CHANGE_VOLUME;
			}
		}
	}
	
	if (old) { mpd_freeStatus(old); }
	if (new) { old = new; }

	return change;
}

/*** GET ***/
int mpd_get(Module_Request request, void *data)
{
	if (!data)
		return MOD_FAIL;
	
	switch (request) {
		case GET_SONGINFO:
			return mpd_getsonginfo((Music_Song *)data);

		case GET_VOLUME:
			return mpd_getvolume((int *)data);

		case GET_PLAYSTATE:
			return mpd_getplaystate((Music_PlayState *)data);

		default:
			return MOD_FAIL;
	}

	return MOD_FAIL;
}
	
/*** QUIT ***/
int mpd_quit(void)
{
	INFO("Closing connection...\n");
	
	mpd_closeConnection(conn);		

	return MOD_OKAY;
}

/****************** INTERNALS *********************/

static
int mpd_getsonginfo(Music_Song *s)
{
	mpd_InfoEntity *i;
	mpd_Song *sng;
	int ret;
	
	if (s == NULL)
		return MOD_FAIL;

	mpd_sendCurrentSongCommand(conn);	

	if (conn->error)
		return MOD_FAIL;
	
	i = mpd_getNextInfoEntity(conn);

	if (i && i->type == MPD_INFO_ENTITY_TYPE_SONG) {
		DEBUG("Song info\n");
		sng = i->info.song;

#define SAFE_CLONE(src, dest)	if (src) { strcpy(dest, src); } else { strcpy(dest, "Unknown"); }
		
		SAFE_CLONE(sng->file,	s->file);
		SAFE_CLONE(sng->title,	s->title);
		SAFE_CLONE(sng->artist,	s->artist);
		SAFE_CLONE(sng->album,	s->album);
		SAFE_CLONE(sng->track,	s->track);
		SAFE_CLONE(sng->name,	s->name);

		s->time = sng->time;
		s->pos = sng->pos;

		mpd_freeInfoEntity(i);
		ret = MOD_OKAY;
	} else {
		ret = MOD_FAIL;
	}

	return ret;
}

static
int mpd_getvolume(int *vol)
{
	mpd_Status *s;
	int ret;

	if (!vol)
		return MOD_FAIL;

	mpd_sendStatusCommand(conn);
	s = mpd_getStatus(conn);

	if (conn->error)
		ret = MOD_FAIL;
	else {
		*vol = s->volume;
		ret = MOD_OKAY;
	}

	mpd_freeStatus(s);
	return ret;
}

static int mpd_getplaystate(Music_PlayState *sta)
{
	mpd_Status *s;
	int ret;

	if (!sta)
		return MOD_FAIL;

	mpd_sendStatusCommand(conn);
	s = mpd_getStatus(conn);

	if (conn->error)
		ret = MOD_FAIL;
	else {
		*sta = (Music_PlayState)s->state;
		ret = MOD_OKAY;
	}

	mpd_freeStatus(s);
	return ret;
}

static int mpd_ping(int tryreconnect)
{
	mpd_Status *s;

	mpd_sendStatusCommand(conn);
	s = mpd_getStatus(conn);

	if (conn->error) {
		INFO("mpd_ping(): Connection error: %s\n", conn->errorStr);

		if (tryreconnect) {
			INFO("mpd_ping(): Trying to reconnect...\n");
			sleep(5);
			mpd_connect();
			
			mpd_freeStatus(s);
			return mpd_ping(0);
		}
		
		mpd_freeStatus(s);
		return 0;
	} else {
		INFO("mpd_ping(): Okay\n");
		mpd_freeStatus(s);
		return 1;
	}
}
