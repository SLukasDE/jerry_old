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

#include "jerry/znode/znode.h"

#include <zsystem/zswait.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <iostream>
#include <set>
#include <map>
#include <list>

#include <unistd.h>

#include "jerry/znode/znobject.h"
#include "jerry/znode/znserver.h"
#include "jerry/znode/znsession.h"
#include "jerry/znode/zntype.h"

#include "jerry/plugin/web/cplugin.h" // void init_plugin(zn__api *a_srv_api, xmlNode *a_config_node);
#include "jerry/plugin/calculator/ccustomsession.h" // void init_plugin(zn__api *a_srv_api, xmlNode *a_config_node);

namespace jerry {
namespace znode {

// ************************************************************************************************
// ***  Alle Funktionen deklarieren                                                             ***
// ************************************************************************************************

static void  api_PluginRegister     (znp__api *a_plugin_api);
static int   api_PluginRegisterTCPIP(int port, const char *a_prefix, const char *a_plugin_name);
static int   api_PluginRegisterLocal(const char *a_name, const char *a_prefix, const char *a_plugin_name);
static void* api_PluginOperation    (const char *a_plugin, const char *a_op_name, void *a_op_param);

#ifdef API_DEBUG
// ************************************************************************************************
// ***  Server DEBUG API                                                                        ***
// ************************************************************************************************

void DEBUG_server_api(const zn__session *a_session);

static void  api_DEBUG_PluginRegister     (znp__api *a_plugin_api);
static int   api_DEBUG_PluginRegisterTCPIP(int port, const char *a_prefix, const char *a_plugin_name);
static int   api_DEBUG_PluginRegisterLocal(const char *a_name, const char *a_prefix, const char *a_plugin_name);
static void* api_DEBUG_PluginOperation    (const char *a_plugin, const char *a_op_name, void *a_op_param);

// ************************************************************************************************
// ***  Plugin DEBUG API                                                                        ***
// ************************************************************************************************

znp__api* DEBUG_get_plugin_api(znp__api *a_plugin_api, znp__session *a_session);

static void  api_plugin_DEBUG_response(znp__api *a_plugin_api, zs__stream *a_stream);
static void* api_plugin_DEBUG_operation(znp__api *a_plugin_api, const char *a_op_name, void *a_data);
static void  api_plugin_DEBUG_destroy(znp__api *a_plugin_api);


#endif

// ************************************************************************************************
// ***  Alle lokalen Variablen definieren                                                       ***
// ************************************************************************************************

#ifdef API_DEBUG
std::map<znp__api*, znp__api*> debug_plugins;
#endif
static zn__api api;
std::map<std::string, znp__api*> plugins;
std::set<std::string> session_types;
static std::map<int, ZNServer*> servers_tcpip;
static std::map<std::string, ZNServer*> servers_local;

bool zn__is_daemon = false;
const char *zn__config_file = "znode.xml";






// ************************************************************************************************
// ***  Alle Strukturen definieren                                                              ***
// ************************************************************************************************

/*
struct zn__implementation {
	const char *name;
//	unsigned char slave_id;
	enum zn__value_type back;
	unsigned int parameter_count;
	enum zn__value_type *parameters;
	struct zn__implementation *next;
};

struct zn__session_type_information {
  zn__implementation *implementation;
  std::map<std::string, zn__implementation*> *element_to_implementation;
};

enum ZNSessionItemType {
	aitPlugin,
	aitLocal,
	aitConsole,
	aitExtern
};
*/
// ************************************************************************************************
// ***  Alle Funktionen implementieren                                                          ***
// ************************************************************************************************


int Main(void *a_null) {
	xmlDoc  *config_doc;
	xmlNode *config_node;

#ifndef API_DEBUG
	api.plugin_register        = &api_PluginRegister;
	api.plugin_register_tcpip  = &api_PluginRegisterTCPIP;
	api.plugin_register_local  = &api_PluginRegisterLocal;
	api.plugin_operation       = &api_PluginOperation;
#else
	api.plugin_register        = &api_DEBUG_PluginRegister;
	api.plugin_register_tcpip  = &api_DEBUG_PluginRegisterTCPIP;
	api.plugin_register_local  = &api_DEBUG_PluginRegisterLocal;
	api.plugin_operation       = &api_DEBUG_PluginOperation;
#endif
	ZNSession::API_Fill(api);
	ZNType::API_Fill(api);
	ZNObject::API_Fill(api);

	/* parse the file and get the DOM */
	config_doc = xmlParseFile(zn__config_file);

	if ( !config_doc ) {
		std::cerr << "error: could not parse file " << zn__config_file << std::endl;
	}

	/* Get the root element node */
	config_node = xmlDocGetRootElement(config_doc);

	for(; config_node; config_node = config_node->next) {
		if(config_node->type != XML_ELEMENT_NODE)
			continue;
		if(config_node->name == 0)
			continue;
		if(strcmp((const char*) config_node->name, "znode") == 0)
			break;
		std::cerr << "Error in configuration file: there is an unknown node \"<" << config_node->name << ">\" defined." << std::endl;
	}
	if(config_node) {
		xmlNode *cur_node;
		for(cur_node = config_node->next; cur_node; cur_node = cur_node->next) {
			if(cur_node->type != XML_ELEMENT_NODE)
				continue;
			if(cur_node->name == 0)
				continue;
			std::cerr << "Error in configuration file: there is an unknown node \"<" << cur_node->name << ">\" defined." << std::endl;
		}
	}
	else {
		std::cerr << "Error in configuration file: there is no node \"<znode>\" defined." << std::endl;
	}





	/************************************
	 *** Install Session-Types        ***
	 ************************************/

	if(config_node) {
		xmlNode *cur_node;
		for (cur_node = config_node->children; cur_node; cur_node = cur_node->next) {
			if (cur_node->type != XML_ELEMENT_NODE)
				continue;
			if(cur_node->name == 0)
				continue;
			if(strcmp((const char*) cur_node->name, "session_type") != 0)
				continue;
			for(_xmlAttr * cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {
				if(cur_attr->name == 0)
					continue;
				if(strcmp((const char*) cur_attr->name, "name") != 0) {
					std::cerr << "Error in configuration file: there is an unknown attribute \"" << cur_attr->name << "\" in node \"<" << cur_node->name << ">\" specified." << std::endl;
					continue;
				}

				/*** Register Session-Type ***/
				session_types.insert( (const char*) xmlGetProp(cur_node, cur_attr->name) );
				std::cout << "Session-Type:" << xmlGetProp(cur_node, cur_attr->name) << std::endl;
			}
		}
	}


	/********************************
	 *** Install Libraries        ***
	 ********************************/

	if(config_node) {
		void *a_lib;
		xmlNode *cur_node;

		for (cur_node = config_node->children; cur_node; cur_node = cur_node->next) {
			if (cur_node->type != XML_ELEMENT_NODE)
				continue;
			if(cur_node->name == 0)
				continue;
			if(strcmp((const char*) cur_node->name, "plugin") != 0)
				continue;
			for(_xmlAttr * cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {
				if(cur_attr->name == 0)
					continue;
				if(strcmp((const char*) cur_attr->name, "library") != 0) {
					std::cerr << "Error in configuration file: there is an unknown attribute \"" << cur_attr->name << "\" in node \"<" << cur_node->name << ">\" specified." << std::endl;
					continue;
				}

				init_plugin_t init_plugin;
				xmlChar* ptr1 = xmlGetProp(cur_node, cur_attr->name);
				unsigned char* ptr2 = static_cast<unsigned char*>(ptr1);
				char* ptr3 = reinterpret_cast<char*>(ptr2);

				std::string libToLoad = static_cast<const char*>(ptr3);

				if (libToLoad.find("calc") != std::string::npos) {
					std::cout << "Build-In-Plugin CALC\n";
					init_plugin = jerry::plugin::calculator::init_plugin;
				}
				else if (libToLoad.find("web") != std::string::npos) {
					std::cout << "Build-In-Plugin WEB\n";
					init_plugin = jerry::plugin::web::init_plugin;
				}
				else {
					/*** Load Library ***/
					std::cout << "Library:" << xmlGetProp(cur_node, cur_attr->name) << std::endl;
					a_lib = dlopen((const char*)xmlGetProp(cur_node, cur_attr->name), RTLD_NOW);
					//a_lib = dlopen(a_libstr, RTLD_LAZY);
					if(!a_lib) {
						std::cout << "Was not able to load " << xmlGetProp(cur_node, cur_attr->name) << ": " << dlerror() << std::endl;
						continue;
					}

					init_plugin = (init_plugin_t) dlsym(a_lib, "init_plugin");
					if(!init_plugin) {
						std::cout << "Fehler: Bei der geladenen Bibliothek \"" << xmlGetProp(cur_node, cur_attr->name) << "\" handelt es sich nicht um ein ZNode-Plugin, weil die Funktion \"init_plugin\" nicht gefunden wurde." << std::endl;
						continue;
					}
				}
				init_plugin(&api, cur_node->children);
			}
		}
	}

	/* free the document */
	xmlFreeDoc(config_doc);

	/*
	 * Free the global variables that may
	 * have been allocated by the parser.
	 */
	xmlCleanupParser();

#if 0
	if(zn__is_daemon)
		zs__wait_for_sigterm();
	else {
#endif
//		zs__wait_for_sigterm();
		while(true) {
			std::string a_string;
			std::cout << "> ";
			std::cin >> a_string;
			if(a_string == "quit")
				break;
		}
#if 0
	}
#endif

	for(std::map<int, ZNServer*>::iterator iter=servers_tcpip.begin(); iter != servers_tcpip.end(); ++iter) {
		ZSThread::Handle a_handle = iter->second->GetHandle();
		ZSThread::Abort(a_handle);
		ZSThread::Wait(a_handle);
	}
	
	for(std::map<std::string, ZNServer*>::iterator iter=servers_local.begin(); iter != servers_local.end(); ++iter) {
		ZSThread::Handle a_handle = iter->second->GetHandle();
		ZSThread::Abort(a_handle);
		ZSThread::Wait(a_handle);
	}

	ZNSession::Release();

	for(std::map<std::string, znp__api*>::iterator iter = plugins.begin(); iter != plugins.end(); ++iter) {
		iter->second->destroy(iter->second);
	}

	return EXIT_SUCCESS;
}



static void api_PluginRegister(znp__api *a_plugin_api) {
	if(!a_plugin_api)
		return;
	//  std::cout << "Register " << a_plugin_api->name << " at " << a_plugin_api << std::endl;
	plugins.insert(std::make_pair(a_plugin_api->name, a_plugin_api));
}

static int api_PluginRegisterTCPIP(int a_port, const char *a_prefix, const char *a_plugin_name) {
	std::map<int, ZNServer*>::iterator serverIter;
	std::map<std::string, znp__api*>::iterator pluginIter;
	ZNServer *a_server;

	pluginIter = plugins.find(a_plugin_name);
	if(pluginIter == plugins.end())
		return 0;

	serverIter = servers_tcpip.find(a_port);
	if(serverIter == servers_tcpip.end()) {
		a_server = new ZNServer(0, a_port);
		servers_tcpip.insert(std::make_pair(a_port, a_server));
		ZSThread::Execute(a_server->GetHandle());
	}
	else
		a_server = serverIter->second;
	if(a_server->Register(a_prefix, pluginIter->second))
		return -1;
	return 0;
}

static int api_PluginRegisterLocal(const char *a_name, const char *a_prefix, const char *a_plugin_name) {
	std::map<std::string, ZNServer*>::iterator iter1;
	std::map<std::string, znp__api*>::iterator iter2;
	ZNServer *a_server;

	iter2 = plugins.find(a_plugin_name);
	if(iter2 == plugins.end())
		return 0;
	iter1 = servers_local.find(a_name);
	if(iter1 == servers_local.end()) {
		a_server = new ZNServer(a_name);
		servers_local.insert(std::make_pair(a_name, a_server));
		ZSThread::Execute(a_server->GetHandle());
	}
	else
		a_server = iter1->second;
	if(a_server->Register(a_prefix, iter2->second))
		return -1;
	return 0;
}

static void* api_PluginOperation(const char *a_plugin, const char *a_op_name, void *a_op_param) {
	std::map<std::string, znp__api*>::iterator iter;

	iter = plugins.find(a_plugin);
	if(iter == plugins.end())
		return 0;
	return iter->second->operation(iter->second, a_op_name, a_op_param);
}

// ### ### ### ### ### ### ### ### ###
// ###  S   E   S   S   I   O   N  ###
// ### ### ### ### ### ### ### ### ###

#ifdef API_DEBUG
// ************************************************************************************************
// ***  Plugin DEBUG API                                                                        ***
// ************************************************************************************************

znp__api* DEBUG_get_plugin_api(znp__api *a_plugin_api, znp__session *a_session) {
	std::map<znp__api*, znp__api*>::iterator a_iter(debug_plugins.find(a_plugin_api));

	if(a_session)
		std::cout << "[" << a_plugin_api->name << ": " << a_session << "] ";
	else
		std::cout << "[" << a_plugin_api->name << "] ";
	return a_iter->second;
}

static void api_plugin_DEBUG_response(znp__api *a_plugin_api, zs__stream *a_stream) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_plugin_api, 0);

	std::cout << "response(znp__api(" << orig_plugin_api << "), zs__stream(" << a_stream << "))" << std::endl;
	orig_plugin_api->response(orig_plugin_api, a_stream);
}

static void* api_plugin_DEBUG_operation(znp__api *a_plugin_api, const char *a_op_name, void *a_data) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_plugin_api, 0);

	std::cout << "operation(znp__api(" << orig_plugin_api << "), op_name = \"" << a_op_name << "\", data = " << a_data << ")" << std::endl;
	return orig_plugin_api->operation(orig_plugin_api, a_op_name, a_data);
}

static void api_plugin_DEBUG_destroy(znp__api *a_plugin_api) {
	znp__api *orig_plugin_api = DEBUG_get_plugin_api(a_plugin_api, 0);

	std::cout << "destroy(znp__api(" << orig_plugin_api << "))" << std::endl;
	orig_plugin_api->destroy(orig_plugin_api);
}

// ************************************************************************************************
// ***  Server DEBUG API                                                                        ***
// ************************************************************************************************

void DEBUG_server_api(const zn__session *a_session) {
	if(a_session)
		std::cout << "[Server: " << a_session << "] ";
	else
		std::cout << "[Server] ";
}

static void api_DEBUG_PluginRegister(znp__api *a_plugin_api) {
	struct znp__api *orig_api = new struct znp__api;

	*orig_api = *a_plugin_api;
	debug_plugins.insert(std::make_pair(a_plugin_api, orig_api));
	ZNSession::Plugin_API_Fill(*a_plugin_api);
	ZNObject::Plugin_API_Fill(*a_plugin_api);
	a_plugin_api->response  = api_plugin_DEBUG_response;
	a_plugin_api->operation = api_plugin_DEBUG_operation;
	a_plugin_api->destroy   = api_plugin_DEBUG_destroy;

	DEBUG_server_api(0);
	std::cout << "plugin_register(znp__api(" << a_plugin_api << "))     [original API is stored at " << orig_api << " now.]" << std::endl;

	api_PluginRegister(orig_api);
}

static int api_DEBUG_PluginRegisterTCPIP(int port, const char *a_prefix, const char *a_plugin_name) {
	DEBUG_server_api(0);
	std::cout << "plugin_register_tcpip(port = " << port << ", a_prefix = \"" << a_prefix << "\", a_plugin_name = \"" << a_plugin_name << "\")" << std::endl;

	return api_PluginRegisterTCPIP(port, a_prefix, a_plugin_name);
}

static int api_DEBUG_PluginRegisterLocal(const char *a_name, const char *a_prefix, const char *a_plugin_name) {
	DEBUG_server_api(0);
	std::cout << "plugin_register_local(name = " << a_name << ", a_prefix = \"" << a_prefix << "\", a_plugin_name = \"" << a_plugin_name << "\")" << std::endl;

	return api_PluginRegisterLocal(a_name, a_prefix, a_plugin_name);
}

static void* api_DEBUG_PluginOperation(const char *a_plugin, const char *a_op_name, void *a_op_param) {
	DEBUG_server_api(0);
	std::cout << "plugin_operation(plugin_name = " << a_plugin << ", op_name = \"" << a_op_name << "\", data = " << a_op_param << ")" << std::endl;

	return api_PluginOperation(a_plugin, a_op_name, a_op_param);
}
#endif

} /* namespace znode */
} /* namespace jerry */
