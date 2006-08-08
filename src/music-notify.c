/* music-notify.c - main program */

#include "utils.h"
#include "backend.h"
#include "popup.h"
#include "show-song.h"

#include "conf.h"

int music_notify_main(void);

#define PACKAGE		"music-notify"
#define LOCALEDIR	"./"

#define DEFAULT_ICON_VOL_MUTE	"/usr/share/icons/Tango/48x48/status/audio-volume-muted.png"
#define DEFAULT_ICON_VOL_LOW	"/usr/share/icons/Tango/48x48/status/audio-volume-low.png"
#define DEFAULT_ICON_VOL_MED	"/usr/share/icons/Tango/48x48/status/audio-volume-medium.png"
#define DEFAULT_ICON_VOL_HIGH	"/usr/share/icons/Tango/48x48/status/audio-volume-high.png"

#define DEFAULT_ICON_PLAY	"/usr/share/icons/Tango/22x22/actions/player_play.png"
#define DEFAULT_ICON_PAUSE	"/usr/share/icons/Tango/22x22/actions/player_pause.png"
#define DEFAULT_ICON_STOP	"/usr/share/icons/Tango/22x22/actions/player_stop.png"
#define DEFAULT_ICON_UNKNOWN	"/usr/share/icons/Tango/22x22/emblems/emblem_system.png"
#define DEFAULT_ICON_SONG	"/usr/share/icons/Tango/48x48/mimetypes/sound.png"

int main(int argc, char **argv)
{
	/* gettext fun */
	//setlocale(LC_MESSAGES, "");	
	//bindtextdomain(PACKAGE, LOCALEDIR);
	//textdomain(PACKAGE);

	config_init();

	/* backend defaults */
	config_set_default("backend.module", TYPE_STRING, "mpd");
	config_set_default("backend.optstring", TYPE_STRING, "mpd_host=192.168.1.1 mpd_port=6600");

	/* icon defaults */
	config_set_default("icon.volume.low",	TYPE_STRING,	DEFAULT_ICON_VOL_LOW);
	config_set_default("icon.volume.med",	TYPE_STRING,	DEFAULT_ICON_VOL_MED);
	config_set_default("icon.volume.high",	TYPE_STRING,	DEFAULT_ICON_VOL_HIGH);
	config_set_default("icon.volume.mute",	TYPE_STRING,	DEFAULT_ICON_VOL_MUTE);

	config_set_default("icon.song",		TYPE_STRING,	DEFAULT_ICON_SONG);

	config_set_default("icon.song.play",	TYPE_STRING,	DEFAULT_ICON_PLAY);
	config_set_default("icon.song.stop",	TYPE_STRING,	DEFAULT_ICON_STOP);
	config_set_default("icon.song.pause",	TYPE_STRING,	DEFAULT_ICON_PAUSE);
	config_set_default("icon.song.unknown",	TYPE_STRING,	DEFAULT_ICON_UNKNOWN);

	config_write_file("config.txt");

	if (!popup_init("Music Notify", "x-music-notify")) {
		INFO(_("Couldn't init popups!\n"));
		DIE();
	} else {
		INFO(_("Initialised popups...\n"));
	}

	if (!music_init(config_get_value_string("backend.module"), config_get_value_string("backend.optstring"))) {
		INFO(_("Couldn't init backend!\n"));
		DIE();
	} else {
		INFO(_("Initialised backend...\n"));
	}

	music_notify_main();
	
	if (!music_quit()) {
		ERROR(_("Couldn't shutdown backend?!?\n"));
		DIE();
	} else {
		INFO(_("Shutdown backend...\n"));
	}

	return 0;
}

void show_error(const char *title, const char *mesg)
{
	Popup *p;

	p = popup_new_stock(STOCK_ERROR);

	popup_set_title(p, title);
	popup_set_mesg(p, mesg);

	popup_show(p);
	popup_destroy(p);
}


void show_current_song(void)
{
	Music_Song *s;
	s = malloc(sizeof(Music_Song));

	if (!music_get_songinfo(s)) {
		show_error(_("Music Error:"), _("Couldn't get current song info!"));
	} else {
		show_song(s);
	}

	free(s);
}


void show_current_volume()
{
	Popup *p;
	char buf[100];
	char *icon;
	int vol;

	music_get_volume(&vol);

	if (vol)
		sprintf(buf, _("Volume is now at %d%%"), vol);
	else
		sprintf(buf, _("Volume is muted"));

	if (vol == 0)
		icon = config_get_value_string("icon.volume.mute");
	else if (vol > 0 && vol <= 33)	/* 0 - 33% */
		icon = config_get_value_string("icon.volume.low");
	else if (vol > 33 && vol <= 66)	/* 34 - 66% */
		icon = config_get_value_string("icon.volume.med");
	else if (vol > 66)		/* 67 - 100% */
		icon = config_get_value_string("icon.volume.high");

	p = popup_new();
	popup_set_subcategory(p, "volume");
	popup_set_title(p, _("Volume Change:"));
	popup_set_mesg(p, buf);
	popup_set_icon(p, icon);

	popup_show(p);
	popup_destroy(p);
}


void show_current_playstate()
{
	Music_PlayState s;

	if (!music_get_playstate(&s)) {
		show_error(_("Music Error:"), _("Couldn't get playstate!"));
		return;
	} else {
		Music_Song *song = malloc(sizeof(Music_Song));

		if (!music_get_songinfo(song)) {
			show_error(_("Music: Error"), _("Couldn't get song info!"));
			return;
		} else {
			Popup *p;
			char buf[200]; char buf2[200];
			char *desc, *cat, *icon;

			switch (s) {
				case STATE_STOP: {
					desc=_("Music Stopped:"); cat="stop";
					icon=config_get_value_string("icon.song.stop");
				} break;
				case STATE_PLAY: {
					desc=_("Music Playing:"); cat="play";
					icon=config_get_value_string("icon.song.play");
				} break;
				case STATE_PAUSE: {
					desc=_("Music Paused:"); cat="pause";
					icon=config_get_value_string("icon.song.pause");
				} break;
				default: {
					desc=_("Music Unknown:"); cat="unknown";
					icon=config_get_value_string("icon.song.unknown");
				} break;
			}
		
			p = popup_new();
			popup_set_title(p, desc);
	
			sprintf(buf2, "<i>%s</i> - <b>%s</b>", song->artist, song->title);
			popup_set_mesg(p, buf2);
			
			popup_set_subcategory(p, cat);
			popup_set_icon(p, icon);

			popup_show(p);
			popup_destroy(p);

			free(song);

			return;
		}
	}
}

int music_notify_main(void)
{
	int running = 1;
	
	show_current_song();	

	while (running) {
		Music_StateChange diff=0;

		diff = music_checkforstatechange();

		if (diff == CHANGE_DISCONNECT) {
			show_error(_("Music Error:"), _("Disconnected!"));
			DIE();
		}

		if ((diff & CHANGE_SONG) != 0) {
			show_current_song();
		}

		if ((diff & CHANGE_VOLUME) != 0) {
			show_current_volume();
		}

		if ((diff & CHANGE_PLAYSTATE) != 0) {
			show_current_playstate();
		}

		sleep(1);
	}
}
