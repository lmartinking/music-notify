/* music-notify.c - main program */

#include <unistd.h>
#include <string.h>

#include "utils.h"
#include "backend.h"
#include "popup.h"
#include "show-song.h"

#include "conf.h"

int debug = 0;
int music_notify_main(void);

#include "defaults.h" /* icon defaults */
#include "config.h"

static void config_set_defaults () {
	/* backend defaults */
	config_set_default("backend.module",	TYPE_STRING, "mpd");
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

	/* private */
	config_set_default(".config.file",	TYPE_STRING,	CONFIG_FILE);
	config_set_default(".config.outfile",	TYPE_STRING,	"stdout");
	config_set_default(".config.writecfg",	TYPE_INTEGER,	0);
	
	config_set_default(".program.name",	TYPE_STRING,	PROG_NAME);
	config_set_default(".program.unixname",	TYPE_STRING,	PROGRAM);
	config_set_default(".program.class",	TYPE_STRING,	PROG_CLASS);

	return;
}

static void print_help(void)
{
	printf("Usage: %s [OPTION]...\n", PROGRAM);
	printf("%s.\n", PROG_DESC);
	
	printf("\n");

	printf(" --debug, -d		Print debugging messages to stderr\n"); 
	printf(" --config, -c <file>	Use <file> for configuration\n");	
	
	printf("\n");
	
	printf(" --help, -h		Show this message\n");
	printf(" --version, -v		Show version information\n");

	printf("\n");

	printf("Report bugs to %s.\n", AUTHOR_EMAIL);
}

static void print_version(void)
{
	printf("%s (%s) %s\n", PROGRAM, PROG_NAME, PROG_VERSION);
	printf("%s.\n", COPYSTRING);	
	printf("%s", DISCLAIMER);
}

/* maybe use getopt? */
static void parse_commandline(int argc, char **argv)
{
	int i;
	char *arg;

	for (i = 0; i < argc; i++) {
		arg = argv[i];

		if (arg[0] != '-') continue;
		if (arg[0] == '-' && arg[1] == '-') arg++;
	
		#define ARG_IS(s)		((strcmp(arg, s) == 0))
		#define NEXT_ARG_EXISTS()	((i < (argc - 1)) ? 1 : 0)
		#define NEXT_ARG()		argv[i+1]
		#define NEXT_ARG_NOTFLAG()	(NEXT_ARG_EXISTS() && NEXT_ARG()[0] != '-')
		#define GET_NEXT_ARG()		((i < (argc - 1)) ? argv[++i] : "<error>") /* INCREMENTS i !!! */
		
		if (ARG_IS("-debug") || ARG_IS("-d")) {
			debug = 1;
			DEBUG("-debug set\n");
		} else
		if (ARG_IS("-config") || ARG_IS("-c")) {
			DEBUG("-config set\n");
			if (NEXT_ARG_NOTFLAG()) {
				config_set_value_string(".config.file", GET_NEXT_ARG());
			} else {
				print_help();
				ERROR("\nError: -config requires a valid argument\n");
				DIE();
			}
		} else
		if (ARG_IS("-print-default-config") || ARG_IS("-p")) {
			char *outfile;

			DEBUG("-print-default-config set\n");
			
			config_set_value_integer(".config.writecfg", 1);

			if (NEXT_ARG_NOTFLAG())  {
				outfile = GET_NEXT_ARG();
			} else {
				outfile = "stdout";
			}

			config_set_value_string(".config.outfile", outfile);
		} else 
		if (ARG_IS("-version") || ARG_IS("-v")) {
			DEBUG("-version\n");		
			print_version();
			EXIT();
		} else
		if (ARG_IS("-help") || ARG_IS("-h")) {
			DEBUG("-help\n");	
			print_help();
			EXIT();
		} else {
			DEBUG("unknown arg: %s\n", arg);
		}
	}
	
	return;
}

int main(int argc, char **argv)
{

#ifdef NLS
	/* gettext fun */
	setlocale(LC_MESSAGES, "");	
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	config_init();
	config_set_defaults();

	parse_commandline(argc, argv);

	if (config_get_value_integer(".config.writecfg")) {
		if (!config_write_file(config_get_value_string(".config.outfile"))) {
			ERROR(_("Could not write config file!\n"));
		} else {
			EXIT();
		}
	}

	if (!config_load_file(config_get_value_string(".config.file"))) {
		ERROR(_("Couldn't read config file!\n"));
		DIE();
	} else {
		INFO(_("Read config file.\n"));
	}

	if (!popup_init(config_get_value_string(".program.name"), config_get_value_string(".program.class"))) {
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

void show_message(const char *title, const char *mesg)
{
	Popup *p;

	p = popup_new_stock(STOCK_INFO);

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

	return 0;
}
