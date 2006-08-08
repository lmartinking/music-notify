#ifndef __UTILS_H_
#define __UTILS_H_

/* utils.h - misc stuff */

#include <stdio.h>	/* for fprintf, etc */
#include <stdlib.h>	/* for exit() */

//#include <locale.h>
//#include <libintl.h>	/* for i18n! */
//#define _(string)	gettext(string)
#define _(string)	string

#define ERROR(args...)	fprintf(stderr, args)
#define WARN(args...)	fprintf(stderr, args)
#define INFO(args...)	printf(args)
#define DIE()		exit(EXIT_FAILURE)

#endif /* __UTILS_H_ */
