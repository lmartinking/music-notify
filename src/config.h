#ifndef __CONFIG
#define __CONFIG

#define PROGRAM		"music-notify"
#define PROG_NAME	"Music Notify"
#define PROG_VERSION	"0.01"
#define PROG_CLASS	"x-music-notify"

#define PROG_DESC	"A program for music notifications (using libnotify)"

#define	AUTHOR_EMAIL	"<lmartinking@gmail.com>"
#define AUTHOR_NAME	"Lucas Martin-King"
#define COPYSTRING	"Copyright (C) 2006 " AUTHOR_NAME

#define DISCLAIMER	"This is free software.  You may redistribute copies of it under the terms of\n" \
			"the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n" \
			"There is NO WARRANTY, to the extent permitted by law.\n"


#define	PREFIX	"/usr"

#define LIBDIR	PREFIX "/lib/" PROGRAM
#define ICONDIR	PREFIX "/share/icons"
#define DOCDIR	PREFIX "/share/doc/" PROGRAM
#define ETCDIR	"~/." PROGRAM

#define	CONFIG_FILE	ETCDIR "/" PROGRAM ".cfg"

#define PACKAGE		PROGRAM
#define LOCALEDIR	"./"

#endif /* __CONFIG */
