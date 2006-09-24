/* popup.c - simple abstraction of libnotify */

#include <string.h>
#include <libnotify/notify.h>

#include "popup.h"
#include "utils.h"

static char *_category;

int popup_init(const char *appname, const char *category)
{
	if (!appname) {
		ERROR(_("popup_init: appname not specified\n"));
		return FALSE;
	}
	
	if (!notify_init(appname)) {
		ERROR(_("Failed to init libnotify\n"));
		return FALSE;
	}

	_category = strdup(category);

	return TRUE;
}

Popup * popup_new(void)
{
	Popup *p;

	p = malloc(sizeof(Popup));
	memset(p, 0, sizeof(Popup));

	p->title	= NULL;
	p->mesg		= NULL;
	p->category	= strdup(_category);
	p->icon		= NULL;
	p->icondata	= NULL;
	p->priority	= PRIORITY_NORMAL;
	p->timeout	= TIMEOUT_DEFAULT;

	p->x = p-> y	= -1; /* hack */

	return p;
}

Popup * popup_new_stock(Popup_Stock_Type t)
{
	Popup *p;
	
	p = popup_new();

	switch (t) {
		case STOCK_INFO: {
			popup_set_icon(p, "gtk-dialog-info");
			popup_set_subcategory(p, "info");
			
		      } break;

		case STOCK_WARNING: {
			popup_set_icon(p, "gtk-dialog-warning");
			P_PRIORITY(p) = PRIORITY_HIGH;	
			popup_set_subcategory(p, "warning");

		      } break;

		case STOCK_ERROR: {
			popup_set_icon(p, "gtk-dialog-error");
			P_PRIORITY(p) = PRIORITY_HIGH;	
			popup_set_subcategory(p, "error");
		      } break;
	
		default:
		case STOCK_NORMAL: {

		      } break;
	}

	return p;
}

struct esc {
	char c;
	char *e;
};

struct esc e_tbl[] = {
	{ '&', "&amp;" },
	{ 0, "" }
};

static char s[2];

static
const char * filter_char(const char c)
{
	int i = 0;	
	
	while (e_tbl[i].c != 0) {
		if (e_tbl[i].c == c) return e_tbl[i].e;
		i++;
	}

	s[0] = c; s[1] = '\0';

	return s;
}

/* XXX: This needs to be much better! */
static
char *
filter_string(const char *src)
{
	int i;
	char buf[1000];

	if (!src)
		return NULL;

	strcpy(buf, "");

	for (i = 0; i < strlen(src) ; i++) {
		char c = src[i];
	
		strcat(buf, filter_char(c));
	}

	return strdup(buf);
}

int	popup_show(Popup *p)
{
	NotifyNotification *n;
	int ret;
	char *body;

	DEBUG(_("\nPopup:\n"));
	DEBUG(_(" Title:    %s\n"), p->title);
	DEBUG(_(" Message:  %s\n"), p->mesg);
	DEBUG(_(" Category: %s\n"), p->category);
	DEBUG(_(" Icon:     %s\n"), p->icon);
	DEBUG("\n");

	if (P_MESSAGE(p))
		body = filter_string(P_MESSAGE(p));
	else
		body = NULL;

	n = notify_notification_new(P_TITLE(p), body, P_ICON(p), NULL);

	if (body) free(body);

	notify_notification_set_timeout(n, P_TIMEOUT(p));
	notify_notification_set_urgency(n, P_PRIORITY(p));
	
	if (P_CATEGORY(p)) {
		notify_notification_set_hint_string(
			n, "category", P_CATEGORY(p));
	}

	if (P_XPOS(p) >= 0 && P_YPOS(p) >= 0) {
		notify_notification_set_hint_int32(n, "x", P_XPOS(p));
		notify_notification_set_hint_int32(n, "y", P_YPOS(p));
	}	

	ret = notify_notification_show(n, NULL);
	g_object_unref(G_OBJECT(n));

	return ret;
}

void	popup_set_title (Popup *p, const char *newtitle)
{
	if (P_TITLE(p)) {
		free(P_TITLE(p));
	}

	P_TITLE(p) = strdup(newtitle);
}

void	popup_set_mesg  (Popup *p, const char *newbody)
{
	if (P_MESSAGE(p))
		free(P_MESSAGE(p));

	P_MESSAGE(p) = strdup(newbody);
}

void	popup_set_icon	(Popup *p, const char *newicon)
{
	if (P_ICON(p))
		free(P_ICON(p));

	P_ICON(p) = strdup(newicon);
}

void	popup_set_subcategory(Popup *p, const char *subc)
{
	char buf[100];
	
	if (P_CATEGORY(p))
		free(P_CATEGORY(p));

	strcpy(buf, _category);
	strcat(buf, ".");
	strcat(buf, subc);

	P_CATEGORY(p) = strdup(buf);
}

void	popup_destroy(Popup *p)
{
	if (!p) return;
	
	#define FREE_IF(e)	if (e) free(e)

	FREE_IF(P_TITLE(p));
	FREE_IF(P_MESSAGE(p));
	FREE_IF(P_CATEGORY(p));
	FREE_IF(P_ICON(p));
	FREE_IF(P_ICONDATA(p));

	free(p); /* Free Willy! */
}
