#ifndef __BACKEND_MODULE_H_
#define __BACKEND_MODULE_H_

/* backend-module.h - module internals */

#define MOD_MAGIC		42424242
#define MOD_SYM_NAME(n)		"_mod_" #n

#define MOD_OKAY		1
#define MOD_FAIL		0

#ifdef __MODULE__

#define MOD_SYMBOL(t, s, v)	t _mod_ ## s = v;

#define MOD_DECLARE()		MOD_SYMBOL(const unsigned int, magic, MOD_MAGIC)
#define MOD_SYM_STR(n, v)	MOD_SYMBOL(const char, n[], v)

#define MOD_NAME(n)		MOD_SYM_STR(name, n)
#define MOD_VERSION(v)		MOD_SYM_STR(version, v)	
#define MOD_AUTHOR(a)		MOD_SYM_STR(author, a)	
#define MOD_COPYRIGHT(c)	MOD_SYM_STR(copyright, c)	

#define MOD_FN_EXPORT(n, f)	MOD_SYMBOL(const char, func_ ## n[], #f)
#define MOD_FN_INIT(f)		MOD_FN_EXPORT(init, f)
#define MOD_FN_QUIT(f)		MOD_FN_EXPORT(quit, f)
#define MOD_FN_POLL(f)		MOD_FN_EXPORT(poll, f)
#define MOD_FN_GET(f)		MOD_FN_EXPORT(get,  f)

#endif

typedef struct {
	char name[20];
	char value[100];
} Module_Param;

typedef enum {
	GET_VOLUME,
	GET_SONGINFO,
	GET_PLAYSTATE
} Module_Request;

#endif /* __BACKEND_MODULE_H_ */
