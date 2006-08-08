#ifndef __POPUP_H_
#define __POPUP_H_

/* popup.h */

typedef enum _Popup_Priority {
	PRIORITY_LOW,
	PRIORITY_NORMAL,
	PRIORITY_HIGH
} Popup_Priority;

enum {
	TIMEOUT_NEVER=0,
	TIMEOUT_DEFAULT=(-1)
};

typedef struct {
	char *		title;
	char *		mesg;
	char *		category;

	Popup_Priority	priority;
	int		timeout;

	char *		icon;
	void *		icondata;

	int		x, y;
} Popup;

typedef enum _Popup_Stock_Type {
	STOCK_NORMAL,
	STOCK_INFO,
	STOCK_WARNING,
	STOCK_ERROR
} Popup_Stock_Type;

#define	P_TITLE(p)	(p->title)
#define P_MESSAGE(p)	(p->mesg)
#define P_CATEGORY(p)	(p->category)
#define P_ICON(p)	(p->icon)
#define P_PRIORITY(p)	(p->priority)
#define P_TIMEOUT(p)	(p->timeout)
#define P_XPOS(p)	(p->x)
#define P_YPOS(p)	(p->y)
#define P_ICONDATA(p)	(p->icondata)

int popup_init(const char *appname, const char *category);
Popup * popup_new(void);
Popup * popup_new_stock(Popup_Stock_Type t);
int	popup_show(Popup *p);

void	popup_destroy	(Popup *p);

void	popup_set_title (Popup *p, const char *newtitle);
void	popup_set_mesg  (Popup *p, const char *newbody);
void	popup_set_icon	(Popup *p, const char *newicon);
void	popup_set_subcategory(Popup *p, const char *subc);

#endif /* __POPUP_H_ */
