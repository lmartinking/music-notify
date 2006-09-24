/* conf.c - configuration manager */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "utils.h"
#include "conf.h"

typedef union {
	int		int_d;
	float		float_d;
	char *		str_d;
	void *		void_d;
} Config_Value_Data;

typedef struct {
	Config_Value_Type	type;
	int			len;
	Config_Value_Data	data;
} Config_Value;

typedef struct Config_Property {
	char *				name;	
	Config_Value			value;
	struct Config_Property * 	next;		
} Config_Property;

static Config_Property *root = NULL;

#define ROOT_NODE	"__root__"

/* Internals */
static void
prop_list_append(Config_Property *new)
{
	Config_Property *p;

	if (!new) {
		return;
	}

	if (root == NULL) {	/* special case! */
		root = new;
		return;
	}

	for (p = root; p->next != NULL; p = p->next);

	new->next = NULL; /* should we really override this? */
	p->next = new;

	return;
}

/* XXX: Check to see if exists? */
static
Config_Property *
prop_create(const char *name)
{
	Config_Property *np;

	np = malloc(sizeof(Config_Property));

	if (np == NULL) {
		ERROR("Couldn't allocate memory: %s ; Seppuku!\n", strerror(errno));
		DIE();
	}
	
	memset(np, 0, sizeof(Config_Property));
	
	np->name = strdup(name);
	np->value.type = TYPE_NONE;
	np->next = NULL;

	return np;
}

static
Config_Property *
prop_get_fromkey(const char *key, int create)
{
	Config_Property *p;	

	if (!key) {
		ERROR("WTF! No key!\n");
		return NULL;
	}

	for (p = root; p != NULL; p = p->next) {
		if (!p)
			break;

		if (!p->name) {
			ERROR("WTF. No keyname!?!\n");
		}

		/* found it! */
		if (strcmp(p->name, key) == 0) {
			return p;
		}
	}

	if (create) {	/* doesn't exist? */
		p = prop_create(key);
		prop_list_append(p);
		return p;
	}

	return NULL;
}

static void
prop_set_type(Config_Property *p, const Config_Value_Type t)
{
	if (!p)
		return;

	/* old type */
	switch (p->value.type) {
		case	TYPE_VOID:
		case	TYPE_STRING:	{
						if (p->value.data.void_d)
							free(p->value.data.void_d);
					} break;

		case	TYPE_INTEGER:	{
						if (t == TYPE_FLOAT) /* int -> float conversion */
							p->value.data.float_d = (float)p->value.data.int_d;
					} break;

		case	TYPE_FLOAT:	{
						if (t == TYPE_INTEGER) /* float -> int conversion */
							p->value.data.int_d = (int)p->value.data.float_d;
					} break;

		default:
					break;
	}

	/* new type stuffs */
	switch(t) {
		case	TYPE_VOID:	
		case	TYPE_STRING:	{
						p->value.data.void_d = NULL;
						p->value.len = 0;
					} break;
					
		default:
					break;
	}
	
	p->value.type = t;
}

static void
prop_set_data(Config_Property *p, const void *data, const int len)
{
	if (!p || !data)
		return;

	if (p->value.type == TYPE_NONE)
		return;
	
	switch (p->value.type) {
		case	TYPE_INTEGER:
			p->value.data.int_d = *((int *)data);
			break;

		case	TYPE_FLOAT:
			p->value.data.float_d = *((float *)data);
			break;

		case	TYPE_STRING:
			{
				p->value.data.str_d = realloc(p->value.data.str_d, strlen(data) + 1);
				strcpy(p->value.data.str_d, data);
			} break;

		case	TYPE_VOID:
			{
				p->value.len = len;
				if (p->value.data.void_d == NULL)
					p->value.data.void_d = calloc(len, 1);
				else
					p->value.data.void_d = realloc(p->value.data.void_d, len);
			} break;

		case	TYPE_NONE:
			break;
	}
}


/* API funcs */
int
config_init(void)
{
	root = prop_create(ROOT_NODE);
	root->next = NULL;

	return 1;
}

void
config_set_default(const char *key, const Config_Value_Type type, const void *val)
{
	Config_Property *p;	

	p = prop_get_fromkey(key, 1);
	prop_set_type(p, type);	
	prop_set_data(p, val, -1);
}

int
config_set_value_string(const char *key, const char *newstr)
{
	Config_Property *p;

	p = prop_get_fromkey(key, 0);

	if (!p || p->value.type != TYPE_STRING)
		return 0;
	else
		prop_set_data(p, newstr, 0);
	
	return 1;
}

int
config_set_value_integer(const char *key, const int newint)
{
	Config_Property *p;

	p = prop_get_fromkey(key, 0);

	if (!p || p->value.type != TYPE_INTEGER)
		return 0;
	else {
		int i = newint;
		int *ptr = &i;
		prop_set_data(p, ptr, 0);
	}
	
	return 1;
}

int
config_set_value_data(const char *key, const void *data, const int len)
{
	Config_Property *p;	

	p = prop_get_fromkey(key, 0);

	if (!p || p->value.type != TYPE_VOID)
		return 0;
	else {
		prop_set_data(p, data, len);
	}
	
	return 1;
}

char *
config_get_value_string(const char *key)
{
	Config_Property *p;

	p = prop_get_fromkey(key, 0);
	if (!p || p->value.type != TYPE_STRING) {
		return NULL;
	} else
		return p->value.data.str_d;
}

float
config_get_value_float(const char *key)
{
	Config_Property *p;

	p = prop_get_fromkey(key, 0);
	if (!p || p->value.type != TYPE_FLOAT) {
		return 0.0;
	} else
		return p->value.data.float_d;
}

void *
config_get_value_data(const char *key, int *len)
{
	Config_Property *p;

	p = prop_get_fromkey(key, 0);
	if (!p || p->value.type != TYPE_VOID || !len) {
		return NULL;
	} else {
		*len = p->value.len;
		return p->value.data.void_d;
	}
}

int
config_get_value_integer(const char *key)
{
	Config_Property *p;

	p = prop_get_fromkey(key, 0);
	if (!p || p->value.type != TYPE_INTEGER) {
		return 0;
	} else {
		return p->value.data.int_d;
	}
}

static int
is_number(const char *s)
{
	int i;		

	for (i = 0; i < strlen(s); i++) {
		if (!isdigit(s[i]))
			return 0;
	}

	return 1;
}

static int
is_text(const char *s)
{
	int i;

	for (i = 0; i < strlen(s); i++) {
		/* is this pedantic enough? */
		if ( ! (isalnum(s[i]) || isspace(s[i]) || ispunct(s[i]) ))
			return 0;
	}

	return 1;
}

int
config_load_file(const char *filename)
{
	FILE *f;
	char *fn;
	char linebuf[200];
	int lineno = 0;

	if (!filename)
		return 0;

	if (filename[0] == '~' && filename[1] == '/') {
		char path[250];
		strcpy(path, getenv("HOME"));
		filename++;
		strcat(path, filename);
		fn = path;
		DEBUG("Path expanded to %s\n", path);
	} else {
		fn = (char *)filename;
	}
	
	if ((f = fopen(fn, "r")) == NULL) {
		ERROR("Couldn't open config file: %s\n", filename);
		return 0;
	}

	INFO("Opened config file: %s\n", filename);

	while (fgets(linebuf, 198, f)) {
		char key[150], value[150];
		char *v;

		if (!is_text(linebuf)) {
			ERROR("Config file contains non-text characters! Maybe it's a binary?\n");
			return 0;
		}
		
		lineno++;

		if (linebuf[0] == '\0' || linebuf[0] == '\n')	/* empty line */
			continue;
		if (linebuf[0] == '#')				/* comment */
			continue;

		/* try to get key */
		sscanf(linebuf, "%s =", key);

		/* try to get value */
		if ((v = strchr(linebuf, '='))) {
			v++;
			while (v[0] == ' ') v++; /* skip whitespace */
			strcpy(value, v);
			value[strlen(v) - 1] = '\0'; /* a little hackish */
		}

		DEBUG("key: '%s' value: '%s'\n", key, value);

		if (is_number(value)) {	
			int i = atoi(value);
			if (!config_set_value_integer(key, i)) {
				ERROR("Couldn't set key (%s) to '%d'\n", key, i);
			}
		} else {			/* assume string for now */
			if (!config_set_value_string(key, value)) {
				ERROR("Couldn't set key (%s) to '%s'\n", key, value);
			}
		}
	}

	fclose(f);

	return 1;
}

static const char *
print_time(void)
{
	time_t now;
	now = time(NULL);
	return ctime(&now);
}

int
config_write_file(const char *filename)
{
	FILE *f;		
	Config_Property *p;

	if (strcmp(filename, "stdout") == 0) {
		f = stdout;
	} else if ((f = fopen(filename, "w")) == NULL) {
		ERROR("Couldn't open config file for writing: %s\n", filename);
		return 0;
	}

	fprintf(f, "# Auto-generated config file.\n");
	fprintf(f, "# Generated: %s\n", print_time());

	for (p = root; p != NULL; p = p->next) {
		char *key;
		
		if (strcmp(p->name, ROOT_NODE) == 0) continue;	/* ignore root */
		if (p->name[0] == '.') continue; /* ignore names starting with . */

		key = p->name;

		fprintf(f, "%s = ", key);

		switch (p->value.type) {
			case TYPE_INTEGER:
					{
						fprintf(f, "%d\n", p->value.data.int_d); 
					} break;

			case TYPE_STRING:
					{
						fprintf(f, "%s\n", p->value.data.str_d);
					} break;

			case TYPE_FLOAT:
					{
						fprintf(f, "%f\n", p->value.data.float_d);
					} break;
			default:
					{
						fprintf(f, "<other>\n");
					} break;
		}
	}

	if (f != stdout)
		fclose(f);

	return 1;
}
