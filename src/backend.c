/* backend.c - music info backend */

#include "utils.h"

#include <dlfcn.h>
#include <string.h>

#include "backend.h"
#include "backend-module.h"

#define DL_ERROR(e)	ERROR("%s: %s\n", e, dlerror())

typedef struct {
	char *name;
	char *version;
	char *author;
	char *copyright;

	void *handle; /* DO NOT TOUCH */

	int (*init)(int n, Module_Param **p);
	int (*poll)(void);
	int (*get)(Module_Request r, void *data);
	int (*quit)(void);
} Module;

/*** MODULE STUFF ***/

static void * 
mod_load(const char *filename)
{
	void *handle;
	int flags;

	flags =	 RTLD_NOW;
	flags |= RTLD_GLOBAL;

	if ((handle = dlopen(filename, flags)) == NULL) {
		DL_ERROR(filename);
		return NULL;
	} else {
		return handle;
	}
}

#define mod_getsym(h, s, p)	(p = dlsym(h, s))

static
int mod_check(void *handle)
{
	unsigned int *magic;

	if (!mod_getsym(handle, MOD_SYM_NAME(magic), magic)) {
		ERROR(_("Error fetching magic"));
		return 0;
	}

	if (*magic != MOD_MAGIC) {
		ERROR(_("Magic incorrect! Was %d, Should be: %d\n"), *magic, MOD_MAGIC);
		return 0;
	} else 
		return 1;
}

static char *
mod_getfunc(void * handle, const char *fn)
{
	char *func;

	if (!mod_getsym(handle, fn, func)) {
		ERROR(_("Couldn't get real function name!\n"));
	}

	return func;
}

static
Module  * module_load(const char *filename)
{
	Module *m;
	void * handle = NULL;

	if (!(handle = mod_load(filename))) {
		return NULL;
	}

	if (!mod_check(handle)) {
		return NULL;
	}

	m = malloc(sizeof(Module));
	memset(m, 0, sizeof(Module));

#define MOD_BAIL()	{ free(m); return 0; }

	if (!mod_getsym(handle, MOD_SYM_NAME(name), m->name)) {
		ERROR(_("Couldn't get module name!\n"));
		MOD_BAIL()
	}

	if (!mod_getsym(handle, MOD_SYM_NAME(version), m->version)) {
		ERROR(_("Couldn't get module version!\n"));
		MOD_BAIL()
	}

	if (!mod_getsym(handle, MOD_SYM_NAME(author), m->author)) {
		ERROR(_("Couldn't get module author!\n"));
		MOD_BAIL()
	}

	if (!mod_getsym(handle, MOD_SYM_NAME(copyright), m->copyright)) {
		ERROR(_("Couldn't get module copyright!\n"));
		MOD_BAIL()
	}

#define MODULE_FUNC(f)	(mod_getfunc(handle, MOD_SYM_NAME(f)))
	
	mod_getsym(handle, MODULE_FUNC(func_init), m->init);
	mod_getsym(handle, MODULE_FUNC(func_quit), m->quit);
	mod_getsym(handle, MODULE_FUNC(func_poll), m->poll);
	mod_getsym(handle, MODULE_FUNC(func_get),  m->get);

	if (!m->init || !m->quit || !m->poll || !m->get) {
		ERROR(_("Couldn't get module functions!\n"));
		MOD_BAIL()
	}

	m->handle = handle;

	return m;
}

//#ifdef __TEST__

static void param_set(Module_Param *p, const char *name, const char *value)
{
	if (p) {
		if (p->name)
			strcpy(p->name, name);
		if (p->value)
			strcpy(p->value, value);
	}
}

/*** END MODULE STUFF ***/

static Module *back_end = NULL;

static
Module_Param **
parse_optstring(const char *options)
{
	char *cp;
	char *pair_sv, *opt_sv;
	char *pair_tok, *opt_tok;
	char *str1, *str2;

	/* XXX: Allocate parms more smartly? */
	Module_Param **parms = calloc(20, sizeof(Module_Param));
	int idx = 0;

	#define PAIR_DELIM	" ,;"
	#define OPT_DELIM	"="

	cp = calloc(strlen(options) + 1, sizeof(char));
	strcpy(cp, options);

	for (str1 = cp; ; str1 = NULL) {
		Module_Param *p;
		int i=0;
		
		pair_tok = strtok_r(str1, PAIR_DELIM, &pair_sv);
		if (pair_tok == NULL) break;

		p = malloc(sizeof(Module_Param));
		parms[idx] = p;

		for (str2 = pair_tok ; ; str2 = NULL) {
			opt_tok = strtok_r(str2, OPT_DELIM, &opt_sv);

			if (opt_tok == NULL) break;

			if (i == 0) {
				strcpy(p->name, opt_tok); i++;
			} else
			if (i == 1) {
				strcpy(p->value, opt_tok); i = 0;
			}
		}

		idx++;
	}

	parms[idx+1] = NULL;

	free(cp);

	return parms;
}

static int parms_len(Module_Param **parms)
{
	Module_Param *p;
	int i = 0;

	for (p = parms[i]; (p = parms[i]) != NULL; i++);
	
	return i;
}

#define	MODULE_DIR	""
#define	MODULE_EXT	".so"

/* heuristics for modules, etc */
int music_init(const char *backend, const char *options)
{
	char modname[300];
	Module_Param **opts;
	int ret;

	if (backend[0] == '/') {			/* absolute path */
		strcpy(modname, backend);
	} else
	if (backend[0] == '.' && backend[1] == '/') {	/* relative path */ 
		strcpy(modname, backend);
	} else {					/* bare */
		strcpy(modname, MODULE_DIR); /* XXX: change to use config */		
		strcat(modname, backend);
		strcat(modname, MODULE_EXT);
	}
	INFO(_("Using '%s' for backend module\n"), modname);

	if ((back_end = module_load(modname)) == NULL) {
		ERROR(_("Couldn't load backend module!\n"));
		return 0;
	}
	
	DEBUG(_("\nModule Info:\n"));
	DEBUG(_(" Name:       %s\n"), back_end->name);
	DEBUG(_(" Version:    %s\n"), back_end->version);
	DEBUG(_(" Author:     %s\n"), back_end->author);
	DEBUG(_(" Copyright:  %s\n"), back_end->copyright);
	DEBUG("\n");

	DEBUG(_("Option string: %s\n"), options);
	opts = parse_optstring(options);

	ret = (back_end->init)(parms_len(opts), opts);

	{
		int i;

		for (i = 0; i < parms_len(opts); i++) {
			free(opts[i]);
		}

		free(opts);
	}

	return ret;
}

int music_quit(void)
{
	return (back_end->quit)();
}

Music_StateChange
music_checkforstatechange(void)
{
	return (back_end->poll)();
}

int music_get_songinfo(Music_Song *s)
{
	Module_Request r = GET_SONGINFO;
	return (back_end->get)(r, s);
}

int music_get_volume(int *vol)
{
	Module_Request r = GET_VOLUME;
	return (back_end->get)(r, vol);
}

int music_get_playstate(Music_PlayState *p)
{
	Module_Request r = GET_PLAYSTATE;
	return (back_end->get)(r, p);
}

