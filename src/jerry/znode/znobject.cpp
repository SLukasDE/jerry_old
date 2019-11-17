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

#include "jerry/znode/znobject.h"

#include <iostream>

#include "jerry/znode/znode.h"
#include "jerry/znode/znsession.h"

namespace jerry {
namespace znode {

void ZNObject::ValueMasterToSlave(const zn__value &master, znp__value &slave, unsigned char slave_id) {
	slave.v_type = master.v_type;
	switch(master.v_type) {
	case zn__vtUInt:
		slave.v_uint = master.v_uint;
		break;
	case zn__vtSInt:
		slave.v_sint = master.v_sint;
		break;
	case zn__vtString:
		slave.v_string = master.v_string;
		break;
	case zn__vtBool:
		slave.v_bool = master.v_bool;
		break;
	case zn__vtPointer:
		if(master.v_pointer)
			slave.v_pointer = ((ZNObject*) master.v_pointer->priv_data)->plugin_objects[slave_id];
		else
			slave.v_pointer = 0;
		break;
	case zn__vtData:
		slave.v_blob.data = master.v_blob.data;
		slave.v_blob.size = master.v_blob.size;
		break;
	default:
		break;
	}
}

void ZNObject::API_Fill(zn__api &a_api) {
#ifndef API_DEBUG
	a_api.object_new         = ZNObject::api_ObjectNew;
	a_api.object_constructor = ZNObject::api_ObjectConstructor;
	a_api.object_delete      = ZNObject::api_ObjectDelete;
	a_api.object_destructor  = ZNObject::api_ObjectDestructor;
	a_api.object_call        = ZNObject::api_ObjectCall;
	a_api.object_set         = ZNObject::api_ObjectSet;
#else
	a_api.object_new         = ZNObject::api_DEBUG_ObjectNew;
	a_api.object_constructor = ZNObject::api_DEBUG_ObjectConstructor;
	a_api.object_delete      = ZNObject::api_DEBUG_ObjectDelete;
	a_api.object_destructor  = ZNObject::api_DEBUG_ObjectDestructor;
	a_api.object_call        = ZNObject::api_DEBUG_ObjectCall;
	a_api.object_set         = ZNObject::api_DEBUG_ObjectSet;
#endif
}

#ifdef API_DEBUG
void ZNObject::Plugin_API_Fill(znp__api &a_plugin_api) {
	a_plugin_api.scheme_request = ZNObject::api_plugin_DEBUG_SchemeRequest;
	a_plugin_api.scheme_get     = ZNObject::api_plugin_DEBUG_SchemeGet;

	a_plugin_api.object_new         = ZNObject::api_plugin_DEBUG_object_new;
	a_plugin_api.object_constructor = ZNObject::api_plugin_DEBUG_object_constructor;
	a_plugin_api.object_delete      = ZNObject::api_plugin_DEBUG_object_delete;
	a_plugin_api.object_destructor  = ZNObject::api_plugin_DEBUG_object_destructor;
	a_plugin_api.object_call        = ZNObject::api_plugin_DEBUG_object_call;
	a_plugin_api.object_set         = ZNObject::api_plugin_DEBUG_object_set;
}
#endif

ZNObject::ZNObject(ZNSession &a_session, znp__object *a_object, const std::string &a_name)
: session(a_session),
  type(a_session.GetType(a_name)),
  object_prev(0),
  object_next(a_session.object_first),
  name(a_name),
  plugin_objects(a_session.plugin_sessions.size(), 0)
{
	object_base.priv_data = this;
	session.object_first = this;
	if(object_next) {
		object_next->object_prev = this;
	}

	a_object->master = &object_base;

	for(unsigned int i=0; i<session.plugin_sessions.size(); ++i) {
		if(!session.plugin_sessions[i]) {
			continue;
		}
		if(session.plugin_sessions[i] == a_object->session) {
			plugin_objects[i] = a_object;
			continue;
		}
		session.slave_current = session.plugin_sessions[i];
		plugin_objects[i] = session.plugin_sessions[i]->plugin_api->object_new(session.plugin_sessions[i], &GetObjectBase(), name.c_str());
	}
}

ZNObject::~ZNObject() {
	const znp__session *a_current_session = session.slave_current;

	if(session.object_first == this) {
		session.object_first = object_next;
	}
	if(object_next) {
		object_next->object_prev = object_prev;
	}
	if(object_prev) {
		object_prev->object_next = object_next;
	}
	for(unsigned int i=0; i<plugin_objects.size(); ++i) {
		if(!plugin_objects[i]
		|| plugin_objects[i]->session == a_current_session) {
			continue;
		}
		session.slave_current = plugin_objects[i]->session;
		plugin_objects[i]->session->plugin_api->object_delete(plugin_objects[i]);
	}
}

void ZNObject::Constructor(const std::string &a_name, unsigned int _argc, const zn__value *_argv) {
	const znp__session *a_current_session = session.slave_current;
	znp__value *a_argv;

	a_argv = _argc ? new znp__value[_argc] : 0;
	for(unsigned int i=0; i<plugin_objects.size(); ++i) {
		if(!plugin_objects[i]
		|| plugin_objects[i]->session == a_current_session)
			continue;
		for(unsigned int j=0; j<_argc; ++j)
			ZNObject::ValueMasterToSlave(_argv[j], a_argv[j], (unsigned char) i);
		session.slave_current = plugin_objects[i]->session;
		plugin_objects[i]->session->plugin_api->object_constructor(plugin_objects[i], a_name.c_str(), _argc, a_argv);
	}
	delete[] a_argv;
}

void ZNObject::Destructor(const std::string &a_type) {
	const znp__session *a_current_session = session.slave_current;

	for(unsigned int i=0; i<plugin_objects.size(); ++i) {
		if(!plugin_objects[i]
		|| plugin_objects[i]->session == a_current_session)
			continue;
		session.slave_current = plugin_objects[i]->session;
		plugin_objects[i]->session->plugin_api->object_destructor(plugin_objects[i], a_type.c_str());
	}
}

void ZNObject::Call(const std::string &a_name, znp__value *a_back, unsigned int _argc, const zn__value *_argv) {
	const znp__session *a_current_session = session.slave_current;
	znp__value *a_argv;

	a_argv = _argc ? new znp__value[_argc] : 0;
	for(unsigned int i=0; i<plugin_objects.size(); ++i) {
		if(!plugin_objects[i]
		|| plugin_objects[i]->session == a_current_session)
			continue;
		for(unsigned int j=0; j<_argc; ++j)
			ZNObject::ValueMasterToSlave(_argv[j], a_argv[j], (unsigned char) i);
		session.slave_current = plugin_objects[i]->session;
		plugin_objects[i]->session->plugin_api->object_call(plugin_objects[i], a_name.c_str(), a_back, _argc, a_argv);
	}
	delete[] a_argv;
}

void ZNObject::Set(const std::string &a_name, const zn__value *_value) {
	const znp__session *a_current_session = session.slave_current;
	znp__value a_value;

	for(unsigned int i=0; i<plugin_objects.size(); ++i) {
		if(!plugin_objects[i]
		|| plugin_objects[i]->session == a_current_session)
			continue;
		session.slave_current = plugin_objects[i]->session;
		ZNObject::ValueMasterToSlave(*_value, a_value, (unsigned char) i);
		plugin_objects[i]->session->plugin_api->object_set(plugin_objects[i], a_name.c_str(), &a_value);
	}
}

void ZNObject::PluginSessionAdd() {
	for(ZNObject *a_obj = this; a_obj; a_obj = a_obj->object_next) {
		a_obj->plugin_objects.push_back(0);
	}
}

void ZNObject::PluginSessionDelete(unsigned int i) {
	for(ZNObject *a_obj = this; a_obj; a_obj = a_obj->object_next) {
		a_obj->plugin_objects[i] = 0;
	}
}

void ZNObject::api_ObjectNew(const zn__session *a_session, znp__object *a_object, const char *a_name) {
	ZNSession *session = (ZNSession*) a_session->priv_data;
	const znp__session *a_slave_last;

	a_slave_last = session->slave_current;
	session->slave_current = a_object->session;
	new ZNObject(*(ZNSession*)a_session->priv_data, a_object, a_name);
	session->slave_current = a_slave_last;
}

void ZNObject::api_ObjectConstructor(const znp__object *a_object, const char *a_name, unsigned int a_argc, const zn__value *a_argv) {
	ZNSession *session = &((ZNObject*) a_object->master->priv_data)->session;
	const znp__session *a_slave_last;

	a_slave_last = session->slave_current;
	session->slave_current = a_object->session;
	((ZNObject*) a_object->master->priv_data)->Constructor(a_name, a_argc, a_argv);
	session->slave_current = a_slave_last;
}

void ZNObject::api_ObjectDelete(const znp__object *a_object) {
	ZNSession *session = &((ZNObject*) a_object->master->priv_data)->session;
	const znp__session *a_slave_last;

	a_slave_last = session->slave_current;
	session->slave_current = a_object->session;
	delete (ZNObject*) a_object->master->priv_data;
	session->slave_current = a_slave_last;
}

void ZNObject::api_ObjectDestructor(const znp__object *a_object, const char *a_type) {
	ZNSession *session = &((ZNObject*) a_object->master->priv_data)->session;
	const znp__session *a_slave_last;

	a_slave_last = session->slave_current;
	session->slave_current = a_object->session;
	((ZNObject*) a_object->master->priv_data)->Destructor(a_type);
	session->slave_current = a_slave_last;
}

void ZNObject::api_ObjectCall(const znp__object *a_object, const char *a_name, znp__value *a_back, unsigned int a_argc, const zn__value *a_argv) {
	ZNSession *session = &((ZNObject*) a_object->master->priv_data)->session;
	const znp__session *a_slave_last;

	a_slave_last = session->slave_current;
	session->slave_current = a_object->session;
	((ZNObject*) a_object->master->priv_data)->Call(a_name, a_back, a_argc, a_argv);
	session->slave_current = a_slave_last;
}

void ZNObject::api_ObjectSet(const znp__object *a_object, const char *a_name, const zn__value *a_value) {
	ZNSession *session = &((ZNObject*) a_object->master->priv_data)->session;
	const znp__session *a_slave_last;

	a_slave_last = session->slave_current;
	session->slave_current = a_object->session;
	((ZNObject*) a_object->master->priv_data)->Set(a_name, a_value);
	session->slave_current = a_slave_last;
}


#ifdef API_DEBUG
void ZNObject::api_DEBUG_ObjectNew(const zn__session *_session, znp__object *a_object, const char *a_name) {
	if(!_session
	|| !_session->priv_data) {
		std::cerr << "Error: call of object_new with illegal session handle." << std::endl;
		return;
	}

	DEBUG_server_api(_session);
	std::cout << "object_new(zn__session(" << _session << "), znp__object(" << a_object << "), name = \"" << a_name << "\")" << std::endl;

	api_ObjectNew(_session, a_object, a_name);
}

void ZNObject::api_DEBUG_ObjectConstructor(const znp__object *a_object, const char *a_name, unsigned int a_argc, const zn__value *a_argv) {
	if(!a_object
	|| !a_object->master
	|| !a_object->master->priv_data) {
		std::cerr << "Error: call of object_constructor with illegal session handle." << std::endl;
		return;
	}

	DEBUG_server_api(&((ZNObject*)a_object->master->priv_data)->session.GetData());
	std::cout << "object_constructor(znp__object(" << a_object << "), name = \"" << a_name << "\", argc = " << a_argc << ", argv = ...)" << std::endl;

	api_ObjectConstructor(a_object, a_name, a_argc, a_argv);
}

void ZNObject::api_DEBUG_ObjectDelete(const znp__object *a_object) {
	if(!a_object
	|| !a_object->master
	|| !a_object->master->priv_data) {
		std::cerr << "Error: call of object_delete with illegal session handle." << std::endl;
		return;
	}

	DEBUG_server_api(&((ZNObject*)a_object->master->priv_data)->session.GetData());
	std::cout << "object_delete(znp__object(" << a_object << "))" << std::endl;

	api_ObjectDelete(a_object);
}

void ZNObject::api_DEBUG_ObjectDestructor(const znp__object *a_object, const char *a_name) {
	if(!a_object
	|| !a_object->master
	|| !a_object->master->priv_data) {
		std::cerr << "Error: call of object_destructor with illegal session handle." << std::endl;
		return;
	}

	DEBUG_server_api(&((ZNObject*)a_object->master->priv_data)->session.GetData());
	std::cout << "object_destructor(znp__object(" << a_object << "), name = \"" << a_name << "\")" << std::endl;

	api_ObjectDestructor(a_object, a_name);
}

void ZNObject::api_DEBUG_ObjectCall(const znp__object *a_object, const char *a_name, znp__value *a_back, unsigned int a_argc, const zn__value *a_argv) {
	if(!a_object
	|| !a_object->master
	|| !a_object->master->priv_data) {
		std::cerr << "Error: call of object_call with illegal session handle." << std::endl;
		return;
	}

	DEBUG_server_api(&((ZNObject*)a_object->master->priv_data)->session.GetData());
	std::cout << "object_call(znp__object(" << a_object << "), name = \"" << a_name << "\", back = ?,  argc = " << a_argc << ", argv = ...)" << std::endl;

	api_ObjectCall(a_object, a_name, a_back, a_argc, a_argv);
}

void ZNObject::api_DEBUG_ObjectSet(const znp__object *a_object, const char *a_name, const zn__value *a_value) {
	if(!a_object
	|| !a_object->master
	|| !a_object->master->priv_data) {
		std::cerr << "Error: call of object_set with illegal session handle." << std::endl;
		return;
	}

	DEBUG_server_api(&((ZNObject*)a_object->master->priv_data)->session.GetData());
	std::cout << "object_set(znp__object(" << a_object << "), name = \"" << a_name << "\", value = ...)" << std::endl;

	api_ObjectSet(a_object, a_name, a_value);
}


/**
 * Plugin API
 */
void ZNObject::api_plugin_DEBUG_SchemeRequest(znp__session *a_session, const char *a_type) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_session->plugin_api, a_session);

	std::cout << "scheme_request(znp__session(" << a_session << "), type = \"" << a_type << "\")" << std::endl;
	orig_plugin_api->scheme_request(a_session, a_type);
}

const znp__scheme* ZNObject::api_plugin_DEBUG_SchemeGet(znp__session *a_session, const char *a_type) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_session->plugin_api, a_session);

	std::cout << "scheme_get(znp__session(" << a_session << "), type = \"" << a_type << "\")" << std::endl;
	return orig_plugin_api->scheme_get(a_session, a_type);
}

const znp__object* ZNObject::api_plugin_DEBUG_object_new(znp__session *a_session, const zn__object *a_object, const char *type) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_session->plugin_api, a_session);

	std::cout << "object_new(znp__session(" << a_session << "), zn__object(" << a_object << "), type = \"" << type << "\")" << std::endl;
	return orig_plugin_api->object_new(a_session, a_object, type);
}

void ZNObject::api_plugin_DEBUG_object_constructor(const znp__object *a_object, const char *type, unsigned int a_argc, const znp__value *a_argv) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_object->session->plugin_api, a_object->session);

	std::cout << "object_constructor(znp__object(" << a_object << "), type = \"" << type << "\", argc = " << a_argc << ", argv = ...)" << std::endl;
	orig_plugin_api->object_constructor(a_object, type, a_argc, a_argv);
}

void ZNObject::api_plugin_DEBUG_object_delete(const znp__object *a_object) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_object->session->plugin_api, a_object->session);

	std::cout << "object_delete(znp__object(" << a_object << "))" << std::endl;
	orig_plugin_api->object_delete(a_object);
}

void ZNObject::api_plugin_DEBUG_object_destructor(const znp__object *a_object, const char *type) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_object->session->plugin_api, a_object->session);

	std::cout << "object_destructor(znp__object(" << a_object << "), type = \"" << type << "\")" << std::endl;
	orig_plugin_api->object_destructor(a_object, type);
}

void ZNObject::api_plugin_DEBUG_object_call(const znp__object *a_object, const char *method, znp__value *a_back, unsigned int a_argc, const znp__value *a_argv) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_object->session->plugin_api, a_object->session);

	std::cout << "object_call(znp__object(" << a_object << "), method = \"" << method << "\", back = ..., argc = " << a_argc << ", argv = ...)" << std::endl;
	orig_plugin_api->object_call(a_object, method, a_back, a_argc, a_argv);
}

void ZNObject::api_plugin_DEBUG_object_set(const znp__object *a_object, const char *property, const znp__value *a_value) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_object->session->plugin_api, a_object->session);

	std::cout << "object_set(znp__object(" << a_object << "), property = \"" << property << "\", a_value = ...)" << std::endl;
	orig_plugin_api->object_set(a_object, property, a_value);
}

#endif

} /* namespace znode */
} /* namespace jerry */
