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

#include "jerry/plugin/calculator/ccustomsession.h"
#include "jerry/plugin/calculator/cobject.h"
#include "jerry/plugin/calculator/ccustomsession.h"
/*
#include "jerry/plugin/calculator/cbutton.h"
#include jerry/plugin/calculator/"cbuttononclick.h"
#include "jerry/plugin/calculator/cedit.h"
#include "jerry/plugin/calculator/clabel.h"
#include "jerry/plugin/calculator/cwebpanel.h"
#include "jerry/plugin/calculator/cpanel.h"
#include "jerry/plugin/calculator/cform.h"
*/
#include <iostream>

namespace jerry {
namespace plugin {
namespace calculator {

struct znp__api plugin_api;

void CCustomSession::api_Init(znp__session *a_plugin_session) {
	CCustomSession *a_session;

	a_session = (CCustomSession*) GET_SESSION(a_plugin_session);

	a_session->screen = new CForm(CObjectData::New(*a_session));
	a_session->SessionInit();
}

void CCustomSession::api_Event(znp__session *a_plugin_session, void *a_data) {
}

void CCustomSession::api_Destroy(znp__session *a_plugin_session) {
	while(!GET_SESSION(a_plugin_session)->objects.empty()) {
		delete (*GET_SESSION(a_plugin_session)->objects.begin())->GetObject();
	}
	delete GET_SESSION(a_plugin_session);
}

CCustomSession::CCustomSession(const zn__session *a_session)
: screen(0)
{
	plugin_session.master     = a_session;
	plugin_session.plugin_api = &plugin_api;
	plugin_session.priv_data  = this;
/*
	type_registry.insert(std::make_pair("customsession", sizeof(CCustomSession)));
	type_registry.insert(std::make_pair("button",        sizeof(CButton)));
	type_registry.insert(std::make_pair("buttononclick", sizeof(CButtonOnClick)));
	type_registry.insert(std::make_pair("edit",          sizeof(CEdit)));
	type_registry.insert(std::make_pair("label",         sizeof(CLabel)));
	type_registry.insert(std::make_pair("webpanel",      sizeof(CWebPanel)));
	type_registry.insert(std::make_pair("panel",         sizeof(CPanel)));
	type_registry.insert(std::make_pair("form",          sizeof(CForm)));
*/
}

CCustomSession::~CCustomSession() {
//	delete screen;
}

CForm & CCustomSession::GetScreen() const {
	return *screen;
}
/*
CObjectData* CCustomSession::ObjectLookup(unsigned int a_object_id) const
{
	std::map<unsigned int, CObjectData*>::const_iterator iter=objects.find(a_object_id);
	return iter == objects.end() ? 0 : iter->second;
}
*/







static void api_Plugin_Response(znp__api *, zs__stream *a_stream_struct) {
	zs__stream_destroy(a_stream_struct);
}

static void api_Plugin_Destroy(znp__api *) {
}

static void* api_Plugin_Operation(znp__api *, const char *op_name, void *op_param) {
	return 0;
}

void init_plugin(struct zn__api *a_srv_api, xmlNode * a_config_node) {
	static struct znp__plugin_data a_plugin_data;

	plugin_api.name = "calculator";

	plugin_api.destroy   = &api_Plugin_Destroy;
	plugin_api.response  = &api_Plugin_Response;
	plugin_api.operation = &api_Plugin_Operation;

	plugin_api.session_create  = CCustomSession::api_Create;
	plugin_api.session_init    = CCustomSession::api_Init;
	plugin_api.session_event   = CCustomSession::api_Event;
	plugin_api.session_destroy = CCustomSession::api_Destroy;

	plugin_api.scheme_request = CObjectData::api_SchemeRequest;
	plugin_api.scheme_get     = CObjectData::api_SchemeGet;

	plugin_api.object_new         = CObjectData::api_New;
	plugin_api.object_constructor = CObjectData::api_Constructor;
	plugin_api.object_delete      = CObjectData::api_Delete;
	plugin_api.object_destructor  = CObjectData::api_Destructor;
	plugin_api.object_call        = CObjectData::api_Call;
	plugin_api.object_set         = CObjectData::api_Set;
	plugin_api.priv_data          = &a_plugin_data;

	a_plugin_data.server_api = a_srv_api;

	a_srv_api->plugin_register(&plugin_api);
}



#if 0
void CSession::ImplementationRequest(const char *a_type_name) {
  zn__value_type argv_control_parent[] = {zn__vtPointer};
  zn__value_type argv_control_left[] = {zn__vtSInt};
  zn__value_type argv_control_top[] = {zn__vtSInt};
  zn__value_type argv_control_height[] = {zn__vtUInt};
  zn__value_type argv_control_width[] = {zn__vtUInt};
  
  zn__value_type argv_button_constructor[] = {};
  zn__value_type argv_button_destructor[] = {};
  zn__value_type argv_button_caption[] = {zn__vtString};
  zn__value_type argv_button_onclick[] = {zn__vtPointer};
  
  zn__value_type argv_buttononclick_constructor[] = {};
  zn__value_type argv_buttononclick_destructor[] = {};
  zn__value_type argv_buttononclick_onclick[] = {zn__vtPointer};

  if(strcmp(a_type_name, "button") == 0) {
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "button", "@C", zn__vtVoid, 0, argv_button_constructor );
    
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "button", "@D", zn__vtVoid, 0, argv_button_destructor);
    
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "button", "@Pparent", zn__vtVoid, 1, argv_control_parent);
    
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "button", "@Pleft", zn__vtVoid, 1, argv_control_left);
    
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "button", "@Ptop", zn__vtVoid, 1, argv_control_top);
    
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "button", "@Pheight", zn__vtVoid, 1, argv_control_height);
    
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "button", "@Pwidth", zn__vtVoid, 1, argv_control_width);
    
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "button", "@Pcaption", zn__vtVoid, 1, argv_button_caption);
    
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "button", "@Ponclick", zn__vtVoid, 1, argv_button_onclick);
  }
  else if(strcmp(a_type_name, "buttononclick") == 0) {
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "buttononclick", "@C", zn__vtVoid, 0, argv_buttononclick_constructor);
    
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, 0, "buttononclick", "@D", zn__vtVoid, 0, argv_buttononclick_destructor);
    
    plugin_session.plugin_api->server_api->implementation_set(plugin_session.master_id, -1, "buttononclick", "OnClick", zn__vtVoid, 1, argv_buttononclick_onclick);
  }
}
#endif

} /* namespace calculator */
} /* namespace plugin */
} /* namespace jerry */
