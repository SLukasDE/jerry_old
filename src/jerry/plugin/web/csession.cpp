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
#include "jerry/plugin/web/chttprequest.h"
#include "jerry/plugin/web/cform.h"

#include <zsystem/zswait.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <sstream>


namespace jerry {
namespace plugin {
namespace web {


// *********************
// ***  Session-API  ***
// *********************

unsigned int CSession::session_id_producer = 0;

znp__session* CSession::api_Create(znp__api *, const zn__session *_session, const char *a_session_type) {
	CSession *a_session;

	a_session = new CSession(_session, a_session_type ? a_session_type : "");
	return &a_session->plugin_session;
}

void CSession::api_Destroy(znp__session *a_plugin_session) {
std::cout << "[Web]: session_destroy" << std::endl;

	while(!GET_SESSION(a_plugin_session)->objects.empty()) {
		std::cout << "[Web]: PluginSessionDestroy, delete obj(" << GET_SESSION(a_plugin_session)->objects.begin()->second->GetID() << ")(" << GET_SESSION(a_plugin_session)->objects.begin()->second->GetTypeName() << ")" << std::endl;
		delete GET_SESSION(a_plugin_session)->objects.begin()->second->GetObject();
//		SERVER_API->object_delete(a_plugin_session->master_id,  GET_SESSION(a_plugin_session)->objects->begin()->second->GetID());
	}
	delete GET_SESSION(a_plugin_session);
}

void CSession::api_Init(znp__session *a_plugin_session) {
std::cout << "[Web]: Init..." << std::endl;
	GET_SESSION(a_plugin_session)->initialized = true;
	GET_SESSION(a_plugin_session)->Unlock();

//	REGISTER_URL(GET_SESSION(a_plugin_session)->main_uri, GET_SESSION(a_plugin_session));

	ZSThread::Execute(GET_SESSION(a_plugin_session)->GetTimer().GetHandle());
	GET_SESSION(a_plugin_session)->GetTimer().Restart();
std::cout << "[Web]: Init done" << std::endl;
}

void CSession::Response(CHttpRequest &a_http_request) {
	CForm *a_screen(0);

#if 1
// DEBUG-Output
for(std::list<http_property>::const_iterator a_content_iter(a_http_request.GetProperties().begin()); a_content_iter!=a_http_request.GetProperties().end(); ++a_content_iter) {
	std::cout << "[WEB]: Set Obj(" << a_content_iter->object->GetID() << ")->" << a_content_iter->property << " = ";
	switch(a_content_iter->value.v_type) {
	case zn__vtSInt:
		std::cout << "SInt(" << a_content_iter->value.v_sint << ")" << std::endl;
		break;
	case zn__vtUInt:
		std::cout << "UInt(" << a_content_iter->value.v_uint << ")" << std::endl;
		break;
	case zn__vtBool:
		std::cout << "Bool(" << a_content_iter->value.v_bool << ")" << std::endl;
		break;
	case zn__vtString:
		std::cout << "String(" << a_content_iter->value.v_string << ")" << std::endl;
		break;
	case zn__vtPointer:
		std::cout << "Pointer(" << a_content_iter->value.v_uint << ")" << std::endl;
		break;
	default:
		std::cout << "Unknown(?)" << std::endl;
		break;
	}
}
#endif
	// Next we analyse the arguments that are already parsed by the HTTP-Request-Class
	// Here it's our job to set properties eventually.
	for(std::list<http_property>::const_iterator a_content_iter(a_http_request.GetProperties().begin()); a_content_iter!=a_http_request.GetProperties().end(); ++a_content_iter) {
		if(!a_content_iter->object
		|| !a_content_iter->object->GetObject())
			continue;
		a_content_iter->object->GetObject()->Set(a_content_iter->property.c_str(), a_content_iter->value);
	}

	// And here it's our job to call a method eventually.
	if(a_http_request.GetCallFunction() != "") {
		if(a_http_request.GetCallObject()
		&& a_http_request.GetCallObject()->GetObject()) {
			znp__value a_back_value;

			a_back_value.v_type = zn__vtUnknown;
			a_http_request.GetCallObject()->GetObject()
				->Call(a_http_request.GetCallFunction().c_str(),
				       &a_back_value,
				       a_http_request.GetArgumentCount(),
				       a_http_request.GetArguments());
		}
	}

	if(a_http_request.GetSubUrl() != "") {
		if(a_http_request.AnswerFile() != 0) {
			a_http_request.OpenAnswer(404, "text/html", "");
			a_http_request.GetStreamWriter() << "<meta http-equiv=\"expires\" content=\"0\">\r\n";
			a_http_request.GetStreamWriter() << "<html>\r\n";
			a_http_request.GetStreamWriter() << "<head>\r\n";
			a_http_request.GetStreamWriter() << "<title></title>\r\n";
			a_http_request.GetStreamWriter() << "</head>\r\n";
			a_http_request.GetStreamWriter() << "<body>\r\n";
			a_http_request.GetStreamWriter() << "File not found\r\n";
			a_http_request.GetStreamWriter() << "</body>\r\n";
			a_http_request.GetStreamWriter() << "</html>\r\n";
		}
		return;
	}


	for(std::set<CForm*>::iterator iter(forms->begin()); iter!=forms->end(); ++iter) {
		if( (*iter)->GetParent() == 0 ) {
			a_screen = *iter;
			break;
		}
	}

	if(!a_screen) {
		a_http_request.OpenAnswer(200, "text/html", "");
		a_http_request.GetStreamWriter() << "<meta http-equiv=\"expires\" content=\"0\">\r\n";
		a_http_request.GetStreamWriter() << "<html>\r\n";
		a_http_request.GetStreamWriter() << "<head>\r\n";
		a_http_request.GetStreamWriter() << "<title>No Title</title>\r\n";
		a_http_request.GetStreamWriter() << "</head>\r\n";
		a_http_request.GetStreamWriter() << "<body>\r\n";
		a_http_request.GetStreamWriter() << "</body>\r\n";
		a_http_request.GetStreamWriter() << "</html>\r\n";
		return;
	}

	a_http_request.OpenAnswer(200, "text/html", "");
	a_http_request.GetStreamWriter() << "<meta http-equiv=\"expires\" content=\"0\">\r\n";
	a_http_request.GetStreamWriter() << "<meta http-equiv=\"refresh\" content=\"" << timeout << "\">\r\n";
	a_http_request.GetStreamWriter() << "<html>\r\n";
	a_http_request.GetStreamWriter() << "<head>\r\n";
	a_http_request.GetStreamWriter() << "<title>" << a_screen->GetTitle() << "</title>\r\n";

	a_http_request.GetStreamWriter() << "<SCRIPT LANGUAGE=\"JavaScript\">\r\n";
	a_http_request.GetStreamWriter() << "<!-- Begin\r\n";
	a_http_request.GetStreamWriter() << "var ZList = {\n";
	a_http_request.GetStreamWriter() << "     objs : new Array(),\n";
	a_http_request.GetStreamWriter() << "     minZ : 0,\n";
	a_http_request.GetStreamWriter() << "     maxZ : 0,\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "     Init : function(o)\n";
	a_http_request.GetStreamWriter() << "     {\n";
	a_http_request.GetStreamWriter() << "          if (ZList.minZ > o.style.zIndex || ZList.objs.length == 0) ZList.minZ = o.style.zIndex;\n";
	a_http_request.GetStreamWriter() << "          if (ZList.maxZ < o.style.zIndex) ZList.maxZ = o.style.zIndex;\n";
	a_http_request.GetStreamWriter() << "          ZList.objs[ZList.objs.length] = o;\n";
	a_http_request.GetStreamWriter() << "     },\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "     GetZ : function(o)\n";
	a_http_request.GetStreamWriter() << "     {\n";
	a_http_request.GetStreamWriter() << "          for(var i=0; i<ZList.objs.length; i++)\n";
	a_http_request.GetStreamWriter() << "               if(ZList.objs[i] == o) return ZList.objs[i].style.zIndex;\n";
	a_http_request.GetStreamWriter() << "          return null;\n";
	a_http_request.GetStreamWriter() << "     },\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "     BringToFront : function(o)\n";
	a_http_request.GetStreamWriter() << "     {\n";
	a_http_request.GetStreamWriter() << "          if(ZList.GetZ(o) >= ZList.maxZ) return;\n";
	a_http_request.GetStreamWriter() << "          for(var i=0; i<ZList.objs.length; i++) {\n";
	a_http_request.GetStreamWriter() << "               if(ZList.objs[i] != o) ZList.objs[i].style.zIndex = ZList.objs[i].style.zIndex - 1;\n";
	a_http_request.GetStreamWriter() << "               else ZList.objs[i].style.zIndex = ZList.maxZ;\n";
	a_http_request.GetStreamWriter() << "          }\n";
	a_http_request.GetStreamWriter() << "     },\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "     SendToBack : function(o)\n";
	a_http_request.GetStreamWriter() << "     {\n";
	a_http_request.GetStreamWriter() << "          if(ZList.GetZ(o) <= ZList.minZ) return;\n";
	a_http_request.GetStreamWriter() << "          for(var i=0; i<ZList.objs.length; i++) {\n";
	a_http_request.GetStreamWriter() << "               if(ZList.objs[i] != o) ZList.objs[i].style.zIndex = ZList.objs[i].style.zIndex + 1;\n";
	a_http_request.GetStreamWriter() << "               else ZList.objs[i].style.zIndex = ZList.minZ;\n";
	a_http_request.GetStreamWriter() << "          }\n";
	a_http_request.GetStreamWriter() << "     }\n";
	a_http_request.GetStreamWriter() << "}\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "var Drag = {\n";
	a_http_request.GetStreamWriter() << "     obj : null,\n";
	a_http_request.GetStreamWriter() << "     init : function(slideObj,dragObj,minX,minY,maxX,maxY)\n";
	a_http_request.GetStreamWriter() << "     {\n";
	a_http_request.GetStreamWriter() << "          dragObj.onmousedown = Drag.front;\n";
	a_http_request.GetStreamWriter() << "          slideObj.onmousedown = Drag.start;\n";
	a_http_request.GetStreamWriter() << "          slideObj.root = dragObj;\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          if (isNaN(parseInt(dragObj.style.left))) dragObj.style.left = \"0px\";\n";
	a_http_request.GetStreamWriter() << "          if (isNaN(parseInt(dragObj.style.top))) dragObj.style.top = \"0px\";\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          // Definition des Bereichs innerhalb der die linke obere Ecke des Objects verschoben werden darf\n";
	a_http_request.GetStreamWriter() << "          // Wenn keine Einschränkung stattfinden soll, dann kann man den Wert \"null\" statt \"<Zahl>\" zuweisen.\n";
	a_http_request.GetStreamWriter() << "          dragObj.minX = minX;\n";
	a_http_request.GetStreamWriter() << "          dragObj.minY = minY;\n";
	a_http_request.GetStreamWriter() << "          dragObj.maxX = maxX;\n";
	a_http_request.GetStreamWriter() << "          dragObj.maxY = maxY;\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          dragObj.onDragStart = new Function();\n";
	a_http_request.GetStreamWriter() << "          dragObj.onDragEnd = new Function();\n";
	a_http_request.GetStreamWriter() << "          dragObj.onDrag = new Function();\n";
	a_http_request.GetStreamWriter() << "     },\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "     front : function(e)\n";
	a_http_request.GetStreamWriter() << "     {\n";
	a_http_request.GetStreamWriter() << "          ZList.BringToFront(this);\n";
	a_http_request.GetStreamWriter() << "     },\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "     start : function(e)\n";
	a_http_request.GetStreamWriter() << "     {\n";
	a_http_request.GetStreamWriter() << "          var o = Drag.obj = this.root;\n";
	a_http_request.GetStreamWriter() << "          var y = parseInt(o.style.top);\n";
	a_http_request.GetStreamWriter() << "          var x = parseInt(o.style.left);\n";
	a_http_request.GetStreamWriter() << "          o.onDragStart(x, y);\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          ZList.BringToFront(o);\n";
	a_http_request.GetStreamWriter() << "          o.lastMouseX = e.clientX;\n";
	a_http_request.GetStreamWriter() << "          o.lastMouseY = e.clientY;\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          if (o.minX != null) o.minMouseX = e.clientX - x + o.minX;\n";
	a_http_request.GetStreamWriter() << "          if (o.maxX != null) o.maxMouseX = o.minMouseX + o.maxX - o.minX;\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          if (o.minY != null) o.minMouseY = e.clientY - y + o.minY;\n";
	a_http_request.GetStreamWriter() << "          if (o.maxY != null) o.maxMouseY = o.minMouseY + o.maxY - o.minY;\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          document.onmousemove = Drag.drag;\n";
	a_http_request.GetStreamWriter() << "          document.onmouseup = Drag.end;\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          return false;\n";
	a_http_request.GetStreamWriter() << "     },\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "     drag : function(e)\n";
	a_http_request.GetStreamWriter() << "     {\n";
	a_http_request.GetStreamWriter() << "          var o = Drag.obj;\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          var ey = e.clientY;\n";
	a_http_request.GetStreamWriter() << "          var ex = e.clientX;\n";
	a_http_request.GetStreamWriter() << "          var y = parseInt(o.style.top );\n";
	a_http_request.GetStreamWriter() << "          var x = parseInt(o.style.left);\n";
	a_http_request.GetStreamWriter() << "          var nx, ny;\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          if (o.minX != null) ex = Math.max(ex, o.minMouseX);\n";
	a_http_request.GetStreamWriter() << "          if (o.maxX != null) ex = Math.min(ex, o.maxMouseX);\n";
	a_http_request.GetStreamWriter() << "          if (o.minY != null) ey = Math.max(ey, o.minMouseY);\n";
	a_http_request.GetStreamWriter() << "          if (o.maxY != null) ey = Math.min(ey, o.maxMouseY);\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          nx = x + ((ex - o.lastMouseX) * 1);\n";
	a_http_request.GetStreamWriter() << "          ny = y + ((ey - o.lastMouseY) * 1);\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          Drag.obj.style[\"left\"] = nx + \"px\";\n";
	a_http_request.GetStreamWriter() << "          Drag.obj.style[\"top\"] = ny + \"px\";\n";
	a_http_request.GetStreamWriter() << "          Drag.obj.lastMouseX = ex;\n";
	a_http_request.GetStreamWriter() << "          Drag.obj.lastMouseY = ey;\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          Drag.obj.onDrag(nx, ny);\n";
	a_http_request.GetStreamWriter() << "          return false;\n";
	a_http_request.GetStreamWriter() << "     },\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "     end : function()\n";
	a_http_request.GetStreamWriter() << "     {\n";
	a_http_request.GetStreamWriter() << "          document.onmousemove = null;\n";
	a_http_request.GetStreamWriter() << "          document.onmouseup   = null;\n";
	a_http_request.GetStreamWriter() << "          Drag.obj.onDragEnd( parseInt(Drag.obj.style[\"left\"]),\n";
	a_http_request.GetStreamWriter() << "                              parseInt(Drag.obj.style[\"top\"]));\n";
	a_http_request.GetStreamWriter() << "          Drag.obj = null;\n";
	a_http_request.GetStreamWriter() << "     }\n";
	a_http_request.GetStreamWriter() << "};\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "var Server = {\n";
	a_screen->CreateDataFormVar(a_http_request.GetStreamWriter());

	// Here we give a chance for all object to extend the Server-Variable
	// with their specific stuff if they are not linked in the tree
	// beginning with root "screen". This can be a timer object by
	// example.
	// Those objects are identified by the following algorithm:
	// 1. We "unmark" ALL objects
	// 2. We call the method "Mark" of the root "screen", that marks
	//    itself and calls "Mark" of it children and so on.
	// 3. Now we can identify all unlinked objects by checking them if
	//    they are still unmarked.
	for(std::map<unsigned int, CObjectData*>::iterator a_iter=objects.begin(); a_iter!=objects.end(); ++a_iter) {
		if(a_iter->second->GetObject())
			a_iter->second->GetObject()->Unmark();
	}
	a_screen->Mark();
	for(std::map<unsigned int, CObjectData*>::iterator a_iter=objects.begin(); a_iter!=objects.end(); ++a_iter) {
		if(!a_iter->second->GetObject()) {
			std::cout << "[Web]: Found ObjectData without refering object." << std::endl;
			continue;
		}
		if(!a_iter->second->GetObject()->IsMarked()) {
			std::cout << "Found unmarked Object \"" << a_iter->second->GetTypeName()
			          << "\" with ID=" << a_iter->second->GetID() << std::endl;
			a_iter->second->GetObject()->CreateDataFormVar(a_http_request.GetStreamWriter());
		}
	}

	a_http_request.GetStreamWriter() << "     Start : function()\n";
	a_http_request.GetStreamWriter() << "     {\n";

	a_screen->CreateDataFormAssignment(a_http_request.GetStreamWriter());
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"<html>\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"<head>\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"<title>\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"Test-App\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"</title>\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"</head>\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"<body bgcolor=\"+'\"'+\"#663333\"+'\"'+\" text=\"+'\"'+\"#FFCC99\"+'\"'+\" link=\"+'\"'+\"#FF9966\"+'\"'+\" vlink=\"+'\"'+\"#FF9900\"+'\"'+\" alink=\"+'\"'+\"#FFFFFF\"+'\"'+\">\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"<form name=\"+'\"'+\"data_form\"+'\"'+\" method=\"+'\"'+\"POST\"+'\"'+\" action=\"+'\"'+\"http://" << GetBaseURI() <<"\"+'\"'+\">\");\n";

	a_screen->CreateDataFormScript(a_http_request.GetStreamWriter());

	a_http_request.GetStreamWriter() << "     },\n";
	a_http_request.GetStreamWriter() << "\n";
	a_http_request.GetStreamWriter() << "     End : function()\n";
	a_http_request.GetStreamWriter() << "     {\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"</form>\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"<SCRIPT LANGUAGE=\"+'\"'+\"JavaScript\"+'\"'+\">\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"document.forms[0].submit();\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"</script>\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"</body>\");\n";
	a_http_request.GetStreamWriter() << "          document.writeln(\"</html>\");\n";
	a_http_request.GetStreamWriter() << "          document.close();\n";
	a_http_request.GetStreamWriter() << "          document.forms[0].submit();\n";
	a_http_request.GetStreamWriter() << "     }\n";
	a_http_request.GetStreamWriter() << "};\n";

	// self.moveTo(0,0);self.resizeTo(screen.availWidth,screen.availHeight);
	a_screen->CreateJavaScript(a_http_request.GetStreamWriter());

	a_http_request.GetStreamWriter() << "//-->\r\n";
	a_http_request.GetStreamWriter() << "</script>\r\n";

	a_http_request.GetStreamWriter() << "</head>\r\n";
	a_http_request.GetStreamWriter() << "<body bgcolor=\"#663333\" text=\"#FFCC99\" link=\"#FF9966\" vlink=\"#FF9900\" alink=\"#FFFFFF\">\r\n";

	a_screen->CreateHtmlBody(a_http_request.GetStreamWriter());

	a_http_request.GetStreamWriter() << "</body>\r\n";
	a_http_request.GetStreamWriter() << "</html>\r\n";
}

void CSession::api_Event(znp__session *a_plugin_session, void *a_data) {
	CHttpRequest *a_http_request = (CHttpRequest*) a_data;

	if(!a_data) {
		std::cout << "[WEB] Session-Timeout occurred." << std::endl;
		// Okay, this means that we must destroy the session
		SERVER_API->session_destroy(a_plugin_session->master);
		return;
	}

	// We must wait until Session_Init is done
	GET_SESSION(a_plugin_session)->Lock();
	GET_SESSION(a_plugin_session)->Unlock();

	// If we have got a request from the engine then first test if this
	// request is allowed to access this session. If not, then we answer
	// with an access violation message because we are frendly :)
	if(!GET_SESSION(a_plugin_session)->IsAccessable(*a_http_request)) {
		std::cerr << "Access forbidden for page " << a_http_request->GetUrl() << "!" << std::endl;

		a_http_request->OpenAnswer(403, "text/html", "");
		a_http_request->GetStreamWriter() << "<html>\r\n";
		a_http_request->GetStreamWriter() << "<body>\r\n";
		a_http_request->GetStreamWriter() << "Access forbidden for page " << a_http_request->GetHost() << "/" << a_http_request->GetUrl() << "!\r\n";
		a_http_request->GetStreamWriter() << "</body>\r\n";
		a_http_request->GetStreamWriter() << "</html>\r\n";
		delete a_http_request;
		return;
	}

	GET_SESSION(a_plugin_session)->GetTimer().Stop();
	GET_SESSION(a_plugin_session)->Response(*a_http_request);
	GET_SESSION(a_plugin_session)->GetTimer().Restart();
	delete a_http_request;
}


// ************************
// ***  Session-Object  ***
// ************************

CSession::CSession(const zn__session *a_session, const std::string &a_session_name)
: session_name(a_session_name),
  session_id(session_id_producer++),
  object_id_producer(0),
  semaphore(new ZSSemaphore(1)),
  initialized(false),
//  implementation_challanged(0),
  forms(new std::set<CForm*>()),
  timeout(3),
  session_timer(new CSessionTimer(a_session, 3))
{
	std::list<CSession*> *a_session_list;

	plugin_session.master     = a_session;
	plugin_session.plugin_api = &plugin_api;
	plugin_session.priv_data  = this;

	semaphore->Set(0, 0);

	a_session_list = LOCK_SESSION_LIST();
	a_session_list->push_back(this);
	UNLOCK_SESSION_LIST();
}

CSession::~CSession() {
	ZSThread::Handle a_handle;
	std::list<CSession*> *a_session_list;
	std::list<CSession*>::iterator a_iter;

	a_handle = session_timer->GetHandle();
	ZSThread::Abort(a_handle);
	ZSThread::Wait(a_handle);

	a_session_list = LOCK_SESSION_LIST();
	a_iter = std::find(a_session_list->begin(), a_session_list->end(), this);
	if(a_iter != a_session_list->end())
		a_session_list->erase(a_iter);
	UNLOCK_SESSION_LIST();

	delete forms;
	delete semaphore;
}

void CSession::Init(const std::string& a_base_uri) {
	std::ostringstream a_str_stream;

	a_str_stream.flags(std::ios::hex | std::ios::uppercase);
	a_str_stream << session_id;

	base_uri = a_base_uri + "/";
	for(unsigned int i=a_str_stream.str().size(); i<8; ++i)
		base_uri += "0";
	base_uri += a_str_stream.str();
}

CObjectData* CSession::ObjectLookup(unsigned int a_object_id) const
{
	std::map<unsigned int, CObjectData*>::const_iterator iter=objects.find(a_object_id);
	return iter == objects.end() ? 0 : iter->second;
}

const std::string& CSession::GetBaseURI() const {
	return base_uri;
}

unsigned int CSession::GetTimeout() const {
	return timeout;
}

unsigned int CSession::ReadObjectID(std::string::const_iterator &a_begin, const std::string::const_iterator &a_end) {
	unsigned int  a_back(0);

	if(a_begin == a_end)
		return 0;
	if(*a_begin != 'C')
		return 0;
	for(++a_begin; a_begin != a_end; ++a_begin) {
		if(*a_begin >= '0' && *a_begin <= '9')
			a_back = (a_back << 4)|((*a_begin-'0')&0x0f);
		else if(*a_begin >= 'A' && *a_begin <= 'F')
			a_back = (a_back << 4)|((*a_begin-'A'+10)&0x0f);
		else
			break;
	}
	return a_back;
}

bool CSession::IsAccessable(CHttpRequest &a_http_request) {
	return a_http_request.GetSession() == 0 || a_http_request.GetSession() == this;
}




CSessionTimer::CSessionTimer(const zn__session *a_master_session, unsigned int a_timeout)
: ZSThread((int(*)(ZSThread &)) &CSessionTimer::internRun, (void(*)(ZSThread &)) &CSessionTimer::internAbort),
  timeout(a_timeout),
  master_session(a_master_session)
{
	std::pair<ZSReader, ZSWriter> a_stream(ZSStream::CreatePipe());
	reader = ZSReader(a_stream.first);
	writer = ZSWriter(a_stream.second);
}

CSessionTimer::~CSessionTimer(){
}

int CSessionTimer::internRun(CSessionTimer &a_this) {
	bool timer_on(false);

	while(true) {
		ZSWait a_wait;
		unsigned int read_event_id;

		read_event_id = a_wait.AddStreamReader(a_this.reader);
		a_wait.AddTimer(2*a_this.timeout+5, 0);

		if(read_event_id == a_wait()) {
			char a_char;

			a_this.reader >> a_char;

			if(a_char == '\0') { // abort
std::cout << "[WEB-TIMER] Abort." << std::endl;
				break;
			}
			else if(a_char == '\1') { // Timer-Stop
				timer_on = false;
			}
			else if(a_char == '\2') { // Timer-Restart
				timer_on = true;
			}
		}
		else if(timer_on) { // timer abgelaufen
std::cout << "[WEB-TIMER] Timeout." << std::endl;
			// Okay, this means that we must destroy the session
			SERVER_API->session_destroy(a_this.master_session);
		}
	}
	return 0;
}

void CSessionTimer::Stop() {
	writer << '\1';
}

void CSessionTimer::Restart() {
	writer << '\2';
}

void CSessionTimer::internAbort(CSessionTimer &a_this) {
	a_this.writer << '\0';
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
