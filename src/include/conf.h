#ifndef __CONF_H_
#define __CONF_H_

/* conf.h - basic configuration manager */

typedef enum {
	TYPE_NONE,
	TYPE_INTEGER,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_VOID
} Config_Value_Type;


int		config_init(void);

void		config_set_default(const char *key, const Config_Value_Type type, const void *val);
int		config_set_value_string(const char *key, const char *newstr);
int		config_set_value_integer(const char *key, const int newint);
int		config_set_value_data(const char *key, const void *data, const int len);

char *		config_get_value_string(const char *key);
void *		config_get_value_data(const char *key, int *len);
int		config_get_value_integer(const char *key);
void *		config_get_value_data(const char *key, int *len);

int		config_load_file(const char *filename);
int		config_write_file(const char *filename);

#endif /* __CONF_H_ */
