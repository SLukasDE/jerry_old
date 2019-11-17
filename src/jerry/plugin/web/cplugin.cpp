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

#include "jerry/plugin/web/cplugin.h"
#include "jerry/plugin/web/csession.h"
#include "jerry/plugin/web/cobject.h"
#include "jerry/plugin/web/chttprequest.h"
#include "jerry/plugin/web/SessionRegistry.h"

#include <zsystem/zswait.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <map>
#include <list>
#include <sstream>

namespace jerry {
namespace plugin {
namespace web {


static void  api_Plugin_Response  (znp__api *, zs__stream *);
static void  api_Plugin_Destroy   (znp__api *);
static void* api_Plugin_Operation (znp__api *, const char *, void *);

/*
static void  api_RegisterURL(const std::string &, CSession *);
static void  api_UnregisterURL(const std::string &);
static void  api_UnregisterSession(CSession *);
*/
static std::list<CSession*>* api_LockSessionList(void);
static void api_UnlockSessionList(void);

struct znp__api plugin_api;

static void init_config_server(xmlNode *cur_node, zn__api *a_srv_api) {
	for(_xmlAttr * cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {
		if(cur_attr->name == 0)
			continue;
		if(strcmp((const char*) cur_attr->name, "port") != 0) {
			std::cerr << "Error in configuration file: there is an unknown attribute \"" << cur_attr->name << "\" in node \"<" << cur_node->name << ">\" specified." << std::endl;
			continue;
		}

		/*** Register TCP/IPS port ***/
		std::cout << "Session-Type:" << xmlGetProp(cur_node, cur_attr->name) << "/" << atoi((const char*) xmlGetProp(cur_node, cur_attr->name)) << std::endl;
		if(a_srv_api->plugin_register_tcpip(atoi((const char*) xmlGetProp(cur_node, cur_attr->name)), "", "web") == 0)
			std::cerr << "There is already a presenter registered for TCP/IP-Port " << xmlGetProp(cur_node, cur_attr->name) << " with the same match code as this presenter has!" << std::endl;
	}
}

void init_plugin(zn__api *a_srv_api, xmlNode *a_config_node) {
	static struct znp__plugin_data a_plugin_data;

	plugin_api.name = "web";

	plugin_api.response  = api_Plugin_Response;
	plugin_api.operation = api_Plugin_Operation;
	plugin_api.destroy   = api_Plugin_Destroy;

	plugin_api.session_create  = CSession::api_Create;
	plugin_api.session_init    = CSession::api_Init;
	plugin_api.session_event   = CSession::api_Event;
	plugin_api.session_destroy = CSession::api_Destroy;

	plugin_api.scheme_request = CObjectData::api_SchemeRequest;
	plugin_api.scheme_get     = CObjectData::api_SchemeGet;

	plugin_api.object_new         = CObjectData::api_New;
	plugin_api.object_constructor = CObjectData::api_Constructor;
	plugin_api.object_delete      = CObjectData::api_Delete;
	plugin_api.object_destructor  = CObjectData::api_Destructor;
	plugin_api.object_call        = CObjectData::api_Call;
	plugin_api.object_set         = CObjectData::api_Set;
	plugin_api.priv_data          = &a_plugin_data;

	a_plugin_data.server_api          = a_srv_api;

	a_plugin_data.semaphore           = new ZSSemaphore(1);
	a_plugin_data.lock_session_list   = api_LockSessionList;
	a_plugin_data.unlock_session_list = api_UnlockSessionList;

	a_srv_api->plugin_register(&plugin_api);

	a_plugin_data.semaphore->Set(0, 1);

	/************************************
	 *** Register TCP/IP Port         ***
	 ************************************/

	if(!a_config_node) {
		return;
	}

	for (; a_config_node; a_config_node = a_config_node->next) {
		if (a_config_node->type != XML_ELEMENT_NODE)
			continue;
		if(a_config_node->name == 0)
			continue;
		if(strcmp((const char*) a_config_node->name, "server") == 0) {
			init_config_server(a_config_node, a_srv_api);
			continue;
		}
		if(strcmp((const char*) a_config_node->name, "session") == 0) {
			SessionRegistry *a_registry = new SessionRegistry(a_config_node);
			a_plugin_data.registry.insert(std::make_pair(a_registry->getSessionName(), a_registry));

			continue;
		}
		if(strcmp((const char*) a_config_node->name, "mime_map") == 0) {
			continue;
		}
		std::cerr << "Error in configuration file: there is an unknown node \"<" << a_config_node->name << ">\" specified." << std::endl;
	}

	for(std::map<std::string, SessionRegistry*>::iterator a_iter = a_plugin_data.registry.begin(); a_iter != a_plugin_data.registry.end(); ++a_iter) {
		a_iter->second->LinkInheritance(a_plugin_data.registry);
	}

	for(std::map<std::string, SessionRegistry*>::iterator a_iter = a_plugin_data.registry.begin(); a_iter != a_plugin_data.registry.end(); ++a_iter) {
		if(a_iter->second->CheckCycle()) {
			std::cerr << "Error: There has been a cycle detected in inheritances of session descriptor \"" << a_iter->first << "\"." << std::endl;
			return;
		}
	}
}

static void api_Plugin_Response(znp__api *, zs__stream *a_stream_struct) {
	CHttpRequest *a_http_request;

	a_http_request = new CHttpRequest(a_stream_struct);

	// Is this request matching to a running session?
	if(a_http_request->GetSession()) {
		// If yes, then forward the request to the appropriate session
		// with this plugin-session as originator
		SERVER_API->session_event(
			&a_http_request->GetSession()->GetPluginSession(), a_http_request);
		return;
	}

	// Is this request matching to a application that should be created?
	if(a_http_request->GetConstructor().second) {
		const zn__session *a_master_session;
		znp__session *a_web_session;
		std::string a_session_name;
		SessionRegistry::Constructor *a_session_constructor;

		a_session_name = a_http_request->GetConstructor().first;
		a_session_constructor = a_http_request->GetConstructor().second;

		std::cout << "[WEB]: application \"" << a_session_name << "\" found!" << std::endl;

		std::cout << "[WEB]: Local= "
		<< (((int) a_http_request->GetLocalIPv4Address()[0]) & 0xff) << "."
		<< (((int) a_http_request->GetLocalIPv4Address()[1]) & 0xff) << "."
		<< (((int) a_http_request->GetLocalIPv4Address()[2]) & 0xff) << "."
		<< (((int) a_http_request->GetLocalIPv4Address()[3]) & 0xff) << std::endl;

		std::cout << "[WEB]: Remote= "
		<< (((int) a_http_request->GetRemoteIPv4Address()[0]) & 0xff) << "."
		<< (((int) a_http_request->GetRemoteIPv4Address()[1]) & 0xff) << "."
		<< (((int) a_http_request->GetRemoteIPv4Address()[2]) & 0xff) << "."
		<< (((int) a_http_request->GetRemoteIPv4Address()[3]) & 0xff) << std::endl;

		// first, create a session for this application.
		// This call will create a master_id that indicates the master session of the framework engine.
		// The framwork will also create all plugin session for each registered session with
		// exception of the session given by the last argument that we are (because we don't want
		// it at this point - we do it manualy later).
		// Each plugin can decide itself if it realy wants to create a plugin session by checking the
		// requested session_name given as first argument.
		a_master_session = SERVER_API->session_create(a_session_name.c_str(), &plugin_api);

		// here we create the web presenter session by our own and
		// connect it to the master session before calling init. We
		// have to do this step because we need the pointer to the
		// corresponding web presenter session to call a special Init-
		// function before the server is allowed to call the general
		// init-function for all plugins.
		a_web_session = SERVER_API->plugin_session_create(a_master_session, "web", a_session_name.c_str());
		GET_SESSION(a_web_session)->Init(a_session_constructor->session_prefix);

		// We are not finished. All plugin-sessions are still in waiting-state. So we must
		// wake-up the sessions now.
		SERVER_API->session_init(a_master_session);

std::cout << "[WEB]: Sending redirect" << std::endl;
		// Now we answer to the HTTP-request with a redirect to the new URL
		a_http_request->OpenAnswer(200, "text/html", "");
		a_http_request->GetStreamWriter() << "<meta http-equiv=\"expires\" content=\"0\">\r\n";
		a_http_request->GetStreamWriter() << "<meta http-equiv=\"refresh\" content=\"0; URL=http://" << GET_SESSION(a_web_session)->GetBaseURI() << "\">\r\n";
		a_http_request->GetStreamWriter() << "<html>\r\n";
		a_http_request->GetStreamWriter() << "<head>\r\n";
		a_http_request->GetStreamWriter() << "<META HTTP-EQUIV=\"expires\" CONTENT=\"0\">\r\n";
		a_http_request->GetStreamWriter() << "<title>Jerry Portpass</title>\r\n";
		a_http_request->GetStreamWriter() << "</head>\r\n";
		a_http_request->GetStreamWriter() << "<body bgcolor=\"#663333\" text=\"#FFCC99\" link=\"#FF9966\" vlink=\"#FF9900\" alink=\"#FFFFFF\">\r\n";
		a_http_request->GetStreamWriter() << "Einen Moment bitte...\r\n";
		a_http_request->GetStreamWriter() << "</body>\r\n";
		a_http_request->GetStreamWriter() << "</html>\r\n";
		delete a_http_request;

		return;
	}

	if(a_http_request->AnswerFile() < 0) {
		a_http_request->OpenAnswer(404, "text/html", "");
		a_http_request->GetStreamWriter() << "<html>\r\n";
		a_http_request->GetStreamWriter() << "<body>\r\n";
		a_http_request->GetStreamWriter() << "Page " << a_http_request->GetUrl() << " not found!\r\n";
		a_http_request->GetStreamWriter() << "</body>\r\n";
		a_http_request->GetStreamWriter() << "</html>\r\n";
	}
	delete a_http_request;
}

static void api_Plugin_Destroy(znp__api *) {
	delete SEMAPHORE;
}

static void* api_Plugin_Operation(znp__api *, const char *op_name, void *op_param) {
	return 0;
}


static std::list<CSession*>* api_LockSessionList(void) {
	SEMAPHORE->Op(0, -1);
	return &((struct znp__plugin_data*) plugin_api.priv_data)->session_list;
}

static void api_UnlockSessionList(void) {
	SEMAPHORE->Op(0, 1);
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
