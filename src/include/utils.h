#ifndef __UTILS_H_
#define __UTILS_H_

/* utils.h - misc stuff */

#include <stdio.h>	/* for fprintf, etc */
#include <stdlib.h>	/* for exit() */

#ifdef NLS
//#include <locale.h>
//#include <libintl.h>	/* for i18n! */
//#define _(string)	gettext(string)
#endif

#define _(string)	string

extern int debug;

#define DEBUG(args...) 	{ if (debug) { fprintf(stderr, "[%s:%d] %s(): ", __FILE__, __LINE__, __FUNCTION__); fprintf(stderr, args); } }
#define ERROR(args...)	fprintf(stderr, args)
#define WARN(args...)	fprintf(stderr, args)
#define INFO(args...)	printf(args)
#define DIE()		exit(EXIT_FAILURE)
#define EXIT()		exit(EXIT_SUCCESS)

#endif /* __UTILS_H_ */
