/*
MIT License

Copyright (c) 2003 Sven Lukas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**

  ZNode
  ZPlugin
  ZApplication
  ZObject
  
  */

#ifndef ZN_API_H
#define ZN_API_H
#include <zsystem/zslib.h>
#include <zsystem/zssemaphore.h>
#include <map>
#include <libxml/tree.h>

enum zn__value_type { zn__vtUInt, zn__vtSInt, zn__vtString, zn__vtBool, zn__vtPointer, zn__vtData, zn__vtUnknown };

struct zn__api;
struct zn__session;
struct zn__object;
struct zn__value;

struct znp__api;
struct znp__session;
struct znp__object;
struct znp__value;
struct znp__type_scheme;
//struct znp__implementation;


struct zn__api {
	// Plugin functions
	void          (*plugin_register)       (struct znp__api*);
	int           (*plugin_register_tcpip) (int a_port, const char *a_prefix, const char *a_plugin_name);
	int           (*plugin_register_local) (const char *a_name, const char *a_prefix, const char *a_plugin_name);
	void*         (*plugin_operation)      (const char *a_plugin_name, const char *a_op_name, void *a_op_param);
	struct znp__session* (*plugin_session_create) (const struct zn__session *a_session, const char *a_plugin, const char *a_session_type);
	void          (*plugin_session_destroy)(struct znp__session *a_session);

	// Session functions
	const struct zn__session* (*session_create) (const char *a_session_type, const struct znp__api *);
	void         (*session_init)   (const struct zn__session *a_session);
	void         (*session_event)  (const struct znp__session *a_slave_session, void *a_data);
	void         (*session_destroy)(const struct zn__session *a_session);

	// Object functions
	void(*object_new)         (const struct zn__session *a_session, struct znp__object *a_object, const char *a_name);
	void(*object_constructor) (const struct znp__object *a_object, const char *a_name, unsigned int a_argc, const struct zn__value *a_argv);
	void(*object_delete)      (const struct znp__object *a_object);
	void(*object_destructor)  (const struct znp__object *a_object, const char *a_name);
	void(*object_call)        (const struct znp__object *a_object, const char *a_name, struct znp__value *a_back, unsigned int a_argc, const struct zn__value *a_argv);
	void(*object_set)         (const struct znp__object *a_object, const char *a_name, const struct zn__value *a_value);
};

struct zn__session {
	void *priv_data;
};

struct zn__object {
	void *priv_data;
};

struct zn__value {
	enum zn__value_type v_type;
	union {
		unsigned int v_uint;
		signed int v_sint;
		const char* v_string;
		int v_bool;
		const struct zn__object *v_pointer;
		struct {
			void *data;
			unsigned int size;
		} v_blob;
	};
};








typedef void  (*response_t)  (struct znp__api*, struct zs__stream*);
typedef void* (*operation_t) (struct znp__api*, const char*, void*);
typedef void  (*destroy_t)   (struct znp__api*);

typedef struct znp__session* (*session_create_t) (struct znp__api *, const struct zn__session *, const char *session_type);
typedef void                 (*session_init_t)   (struct znp__session *);
typedef void                 (*session_event_t)  (struct znp__session *, void *data);
typedef void                 (*session_destroy_t)(struct znp__session *);

typedef void                       (*scheme_request_t) (struct znp__session *, const char *type);
typedef const struct znp__scheme*  (*scheme_get_t)     (struct znp__session *, const char *type);

typedef const struct znp__object* (*object_new_t)(struct znp__session *, const struct zn__object *a_object, const char *type);
typedef void (*object_constructor_t) (const struct znp__object *a_object, const char *type,
                                      unsigned int a_argc, const struct znp__value *a_argv);
typedef void (*object_delete_t)      (const struct znp__object *a_object);
typedef void (*object_destructor_t)  (const struct znp__object *a_object, const char *type);
typedef void (*object_call_t)        (const struct znp__object *a_object, const char *method,
                                      struct znp__value *a_back, unsigned int a_argc, const struct znp__value *a_argv);
typedef void (*object_set_t)         (const struct znp__object *a_object, const char *property, const struct znp__value *a_value);




struct znp__api {
	const char *name;

	response_t  response;
	operation_t operation;
	destroy_t   destroy;

	session_create_t  session_create;
	session_init_t    session_init;
	session_event_t   session_event;
	session_destroy_t session_destroy;

	scheme_request_t     scheme_request;
	scheme_get_t         scheme_get;

	object_new_t         object_new;
	object_constructor_t object_constructor;
	object_delete_t      object_delete;
	object_destructor_t  object_destructor;
	object_call_t        object_call;
	object_set_t         object_set;

	void *priv_data;
};

struct znp__session {
	const struct zn__session *master;
	struct znp__api *plugin_api;
	void *priv_data;
};

struct znp__object {
	const struct zn__object *master;
	struct znp__session *session;
	void *priv_data;
};

struct znp__value {
	enum zn__value_type v_type;
	union {
		unsigned int v_uint;
		signed int v_sint;
		const char* v_string;
		int v_bool;
		const struct znp__object *v_pointer;
		struct {
			void *data;
			unsigned int size;
		} v_blob;
	};
};


enum znp__type_source { zn__tsNotAvaliable, zn__tsOneWay, zn__tsCharged };

struct znp__type {
	enum zn__value_type v_type;
	union {
		const char* v_pointer;
	};
};

struct znp__constructor_scheme {
	unsigned int      argc;
	struct znp__type *argv;
	enum znp__type_source availability;

	unsigned int      base_call_argc;
	struct znp__type *base_call_argv;
};

struct znp__scheme {
	const char *base;
	unsigned int constructor_count;
	struct znp__constructor_scheme *constructors;
};

#endif
