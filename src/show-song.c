/* popup-songchange.c */

#include <string.h>

#include "backend.h"
#include "popup.h"
#include "utils.h"

#include "conf.h"

static char *
fmt_file(const char *path)
{
	char *substr;
	char *file;

	for (substr = (char *)path;
		(substr = strchr(substr, '/')); substr++) {
		file = ++substr;
	}

	return file;
}

static char *
fmt_time(int secs)
{
	if (secs < 0) {
		return _("Unknown Length");
	} else {
		char buf[200];
		int m;
		int s;

		m = secs / 60;
		s = secs - (m * 60);

		sprintf(buf, _("Length:\t%d:%02d"), m, s);

		return strdup(buf);
	}
}

static char *
fmt_italic(const char *s)
{
	char buf[200];	

	strcpy(buf, "<i>");
	strcat(buf, s);
	strcat(buf, "</i>");

	return (char *)buf;
}
	
static char *
format_song(Music_Song *s)
{
	char buf[2000];
	char t[100];
	
	if (strcmp(s->title, "Unknown") == 0) {
		strcpy(t, fmt_file(s->file));
	} else {
		strcpy(t, s->title);
	}

	sprintf(buf, "<b>%s</b>\n%s:\t<i>%s</i>\n%s:\t<i>%s</i>\n%s", t, _("Album"), s->album, _("Artist"), s->artist, fmt_time(s->time));

	return buf;
}


int show_song(Music_Song *s)
{
	Popup *p;

	p = popup_new();

	popup_set_title(p, _("Current Song:"));
	popup_set_mesg(p,format_song(s));	
	popup_set_subcategory(p, "song");
	popup_set_icon(p, config_get_value_string("icon.song"));

	popup_show(p);
	popup_destroy(p);
}
