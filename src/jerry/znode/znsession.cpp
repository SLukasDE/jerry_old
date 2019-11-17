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

#include "jerry/znode/znsession.h"

#include <iostream>
#include <algorithm>

#include "jerry/znode/znobject.h"
#include "jerry/znode/znode.h"
#include "jerry/znode/znserver.h"
#include "jerry/znode/zntype.h"

namespace jerry {
namespace znode {

// ************************************************************************************************
// ***  Class ZNSession                                                                          ***
// ************************************************************************************************

std::list<ZNSession*> ZNSession::sessions;

ZNSession::ZNSession(const char *a_session_name, const znp__api *a_plugin)
: object_first(0),
  slave_current(0),
  semaphore(1)
{
#ifdef API_DEBUG
	std::map<znp__api*, znp__api*>::iterator a_DEBUG_iter(debug_plugins.find((znp__api *) a_plugin));
#endif
	data.priv_data = this;

	semaphore.Set(0, 1);

	ZNSession::sessions.push_back(this);

	// now we append the corresponding slave sessions to the master session
	for(std::map<std::string, znp__api*>::iterator a_iter=plugins.begin(); a_iter!=plugins.end(); ++a_iter) {
		// check if the current session is the calling session. Then we have to skip the creation.
		// The calling session must append it's slave session later by itself.
#ifndef API_DEBUG
		if(a_iter->second == a_plugin)
			continue;
		ZNSession::api_PluginSessionCreate(&data, a_iter->first.c_str(), a_session_name);
#else
		if(a_iter->second == a_DEBUG_iter->second)
			continue;
		ZNSession::api_DEBUG_PluginSessionCreate(&data, a_iter->first.c_str(), a_session_name);
#endif
	}
}

ZNSession::~ZNSession() {
	for(unsigned int i=0; i<plugin_sessions.size(); ++i) {
		if(plugin_sessions[i])
			ZNSession::api_PluginSessionDestroy(plugin_sessions[i]);
	}
	while(!types.empty()) {
		delete types.begin()->second;
	}

	ZNSession::sessions.erase(std::find(ZNSession::sessions.begin(), ZNSession::sessions.end(), this));
}

ZNType& ZNSession::GetType(const std::string &a_name) {
	std::map<std::string, ZNType*>::const_iterator a_iter;

	a_iter = types.find(a_name);
	if(a_iter == types.end()) {
		ZNType *a_type;

		a_type = new ZNType(*this, a_name);
		for(unsigned int i=0; i<plugin_sessions.size(); ++i)
			a_type->PluginSessionAdd();
		return *a_type;
	}
	return *a_iter->second;
}

void ZNSession::RegisterType(ZNType &a_type) {
	types.insert(std::make_pair(a_type.GetName(), &a_type));
}

void ZNSession::RemoveType(ZNType &a_type) {
	types.erase(a_type.GetName());
}

void ZNSession::API_Fill(zn__api &a_api) {
#ifndef API_DEBUG
	a_api.plugin_session_create  = ZNSession::api_PluginSessionCreate;
	a_api.plugin_session_destroy = ZNSession::api_PluginSessionDestroy;

	a_api.session_create         = ZNSession::api_SessionCreate;
	a_api.session_init           = ZNSession::api_SessionInit;
	a_api.session_event          = ZNSession::api_SessionEvent;
	a_api.session_destroy        = ZNSession::api_SessionDestroy;
#else
	a_api.plugin_session_create  = ZNSession::api_DEBUG_PluginSessionCreate;
	a_api.plugin_session_destroy = ZNSession::api_DEBUG_PluginSessionDestroy;

	a_api.session_create         = ZNSession::api_DEBUG_SessionCreate;
	a_api.session_init           = ZNSession::api_DEBUG_SessionInit;
	a_api.session_event          = ZNSession::api_DEBUG_SessionEvent;
	a_api.session_destroy        = ZNSession::api_DEBUG_SessionDestroy;
#endif
}

void ZNSession::Release() {
	while(!ZNSession::sessions.empty()) {
		delete *ZNSession::sessions.begin();
	}
}

#ifdef API_DEBUG
void ZNSession::Plugin_API_Fill(znp__api &a_plugin_api) {
	a_plugin_api.session_create  = ZNSession::api_plugin_DEBUG_session_create;
	a_plugin_api.session_init    = ZNSession::api_plugin_DEBUG_session_init;
	a_plugin_api.session_event   = ZNSession::api_plugin_DEBUG_session_event;
	a_plugin_api.session_destroy = ZNSession::api_plugin_DEBUG_session_destroy;
}
#endif

znp__session* ZNSession::api_PluginSessionCreate(const zn__session *_session, const char *a_plugin, const char *a_session_name) {
	znp__session *a_plugin_session;
	std::map<std::string, znp__api*>::iterator a_iter;
	ZNSession *a_session;

	if(!_session
	|| !_session->priv_data) {
		std::cerr << "Error: call of plugin_session_create with illegal session handle." << std::endl;
		return 0;
	}
	a_session = (ZNSession*) _session->priv_data;
	a_iter = plugins.find(a_plugin);
	if(a_iter == plugins.end()) {
		std::cerr << "Error: call of plugin_session_create with unknown session name \"" << a_session_name << "\"." << std::endl;
		return 0;
	}
	a_plugin_session = a_iter->second->session_create(a_iter->second, _session, a_session_name);
	a_session->plugin_sessions.push_back(a_plugin_session);

	// if we have already objects created, then it's necessary to extend their vecor of referenced
	// plugin sessions to have an equal index mapping for all old objects and new objects.
	if(a_session->object_first) {
		a_session->object_first->PluginSessionAdd();
	}
	for(std::map<std::string, ZNType*>::iterator a_iter=a_session->types.begin(); a_iter!=a_session->types.end(); ++a_iter) {
		a_iter->second->PluginSessionAdd();
	}

	return a_plugin_session;
}

void ZNSession::api_PluginSessionDestroy(znp__session *_session) {
	const znp__session *a_slave_last;
	ZNSession *a_session;
	unsigned int i;

	if(!_session
	|| !_session->master
	|| !_session->master->priv_data) {
		std::cerr << "Error: call of plugin_session_destroy with illegal session handle." << std::endl;
		return;
	}
	a_session = (ZNSession*) _session->master->priv_data;

	a_slave_last = a_session->slave_current;
	a_session->slave_current = _session;

	_session->plugin_api->session_destroy(_session);
	for(i=0; i<a_session->plugin_sessions.size(); ++i) {
		if(a_session->plugin_sessions[i] == _session)
			break;
	}
	if(i != a_session->plugin_sessions.size()) {
		a_session->plugin_sessions[i] = 0;
		// if we have already objects created, then it's necessary to
		// set the corresponding index in their vector list to 0.
		if(a_session->object_first) {
			a_session->object_first->PluginSessionDelete(i);
		}
		for(std::map<std::string, ZNType*>::iterator a_iter=a_session->types.begin(); a_iter!=a_session->types.end(); ++a_iter) {
			a_iter->second->PluginSessionDelete(i);
		}
	}
	a_session->slave_current = a_slave_last;
}

const zn__session* ZNSession::api_SessionCreate(const char *a_session_name, const znp__api *a_plugin) {
	std::set<std::string>::iterator a_iter;

	// before we do anything, first check if there was a valid session-name requested
	a_iter = a_session_name ? session_types.find(a_session_name) : session_types.end();
	if(a_iter == session_types.end()) {
		return 0;
	}

	// next we can create an empty Master-Session
	return &(new ZNSession(a_session_name, a_plugin))->GetData();
}

void ZNSession::api_SessionInit(const zn__session *_session) {
	const znp__session *a_slave_last;
	ZNSession *a_session((ZNSession*) _session->priv_data);

	if(!a_session) {
		std::cerr << "Error: call of session_init with illegal session handle." << std::endl;
		return;
	}
	a_slave_last = a_session->slave_current;
	for(unsigned int i=0; i<a_session->plugin_sessions.size(); ++i) {
		if(!a_session->plugin_sessions[i])
			continue;
		a_session->slave_current = a_session->plugin_sessions[i];
		a_session->plugin_sessions[i]->plugin_api->session_init(a_session->plugin_sessions[i]);
	}
	a_session->slave_current = a_slave_last;
}

void ZNSession::api_SessionEvent(const znp__session *a_slave_session, void *a_data) {
	ZNSession *a_session;
	const znp__session *a_slave_last;

	if(!a_slave_session
	|| !a_slave_session->master
	|| !a_slave_session->master->priv_data) {
		std::cerr << "Error: call of session_event with illegal session handle." << std::endl;
		return;
	}
	a_session = (ZNSession*) a_slave_session->master->priv_data;
	a_session->semaphore.Op(0, -1);
	a_slave_last = a_session->slave_current;
	a_session->slave_current = a_slave_session;
	a_slave_session->plugin_api->session_event((znp__session *)a_slave_session, a_data);
	a_session->slave_current = a_slave_last;
	a_session->semaphore.Op(0, 1);
}

void ZNSession::api_SessionDestroy(const zn__session *_session) {
	ZNSession *a_session((ZNSession*) _session->priv_data);

	if(!a_session) {
		std::cerr << "Error: call of session_destroy with illegal session handle." << std::endl;
		return;
	}
	ZSThread::Execute((new ZNSessionDestroy(*a_session))->GetHandle());
}


#ifdef API_DEBUG
// ************************************************************************************************
// ***  Server DEBUG API                                                                        ***
// ************************************************************************************************

znp__session* ZNSession::api_DEBUG_PluginSessionCreate(const zn__session *a_session, const char *a_plugin_name, const char *a_session_name) {
	znp__session *a_back;

	DEBUG_server_api(a_session);
	std::cout << "plugin_session_create(zn__session(" << a_session << "), plugin_name = \"" << a_plugin_name << "\", session_name = \"" << a_session_name << "\")" << std::endl;

	a_back = ZNSession::api_PluginSessionCreate(a_session, a_plugin_name, a_session_name);
	DEBUG_server_api(a_session);
	std::cout << "-> plugin_session_create(...) = znp__session(" << a_back << ")" << std::endl;
	return a_back;
}


void ZNSession::api_DEBUG_PluginSessionDestroy(znp__session *a_session) {
	DEBUG_server_api(a_session->master);
	std::cout << "plugin_session_destroy(znp__session(" << a_session << "))" << std::endl;

	ZNSession::api_PluginSessionDestroy(a_session);
}




const zn__session* ZNSession::api_DEBUG_SessionCreate(const char *a_session_type, const znp__api *a_plugin) {
	DEBUG_server_api(0);
	std::cout << "session_create(session_type = \"" << a_session_type << "\", znp__api(" << a_plugin << "))" << std::endl;

	return api_SessionCreate(a_session_type, a_plugin);
}

void ZNSession::api_DEBUG_SessionInit(const zn__session *_session) {
	DEBUG_server_api(_session);
	std::cout << "session_init(zn__session(" << _session << "))" << std::endl;

	api_SessionInit(_session);
}

void ZNSession::api_DEBUG_SessionEvent(const znp__session *a_session, void *a_data) {
	DEBUG_server_api(a_session->master);
	std::cout << "session_event(zn__session(" << a_session << "), data = " << a_data << ")" << std::endl;

	api_SessionEvent(a_session, a_data);
}

void ZNSession::api_DEBUG_SessionDestroy(const zn__session *_session) {
	DEBUG_server_api(_session);
	std::cout << "session_destroy(zn__session(" << _session << "))" << std::endl;

	api_SessionDestroy(_session);
}





// ************************************************************************************************
// ***  Plugin DEBUG API                                                                        ***
// ************************************************************************************************

znp__session* ZNSession::api_plugin_DEBUG_session_create(znp__api *a_plugin_api, const zn__session *a_session, const char *session_type) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_plugin_api, 0);

	std::cout << "session_create(znp__api(" << orig_plugin_api << "), zn__session(" << a_session << "), session_type = \"" << session_type << "\")" << std::endl;
	return orig_plugin_api->session_create(orig_plugin_api, a_session, session_type);
}

void ZNSession::api_plugin_DEBUG_session_init(znp__session *a_session) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_session->plugin_api, a_session);

	std::cout << "session_init(znp__session(" << a_session << "))" << std::endl;
	orig_plugin_api->session_init(a_session);
}

void ZNSession::api_plugin_DEBUG_session_event(znp__session *a_session, void *data) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_session->plugin_api, a_session);

	std::cout << "session_event(znp__session(" << a_session << "), data = " << data << ")" << std::endl;
	orig_plugin_api->session_event(a_session, data);
}

void ZNSession::api_plugin_DEBUG_session_destroy(znp__session *a_session) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_session->plugin_api, a_session);

	std::cout << "session_destroy(znp__session(" << a_session << "))" << std::endl;
	orig_plugin_api->session_destroy(a_session);
}
#endif


ZNSessionDestroy::ZNSessionDestroy(ZNSession &a_session)
: ZSThread((int(*)(ZSThread &)) &ZNSessionDestroy::internRun, 0),
  session(a_session)
{
}

int ZNSessionDestroy::internRun(ZNSessionDestroy &a_this) {
	delete &a_this.session;
	return 0;
}

} /* namespace znode */
} /* namespace jerry */
