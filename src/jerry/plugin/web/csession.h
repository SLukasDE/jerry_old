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

#ifndef C_JERRY_PLUGIN_WEB_SESSION_H
#define C_JERRY_PLUGIN_WEB_SESSION_H

#include <map>
#include <set>
#include <list>
#include <iostream>
#include <string>
#include <zsystem/zsthread.h>
#include <zsystem/zsstream.h>
#include <zsystem/zssemaphore.h>
#include <zsystem/zsmessageloop.h>

#include "jerry/znode/zn_api.h"

#include "jerry/plugin/web/SessionRegistry.h"

#define GET_SESSION(ps) ((CSession*) ps->priv_data)

namespace jerry {
namespace plugin {
namespace web {

class CForm;
class CObjectData;
class CSessionMessageLoop;
class CSessionTimer;
class CHttpRequest;

class CSession {
friend class CObjectData;
friend class CSessionTimer;
friend class CSessionMessageLoop;
friend class CForm;
public:
	static znp__session* api_Create(znp__api *, const zn__session *a_session, const char *a_session_type);
	static void api_Destroy(znp__session *a_plugin_session);
	static void api_Init(znp__session *a_plugin_session);
	static void api_Event  (znp__session *a_plugin_session, void *a_data);
	static unsigned int ReadObjectID(std::string::const_iterator &a_begin, const std::string::const_iterator &a_end);

	CSession(const zn__session *a_session, const std::string &a_session_name);
	~CSession();

	void Init(const std::string& a_base_uri);
	void Response(CHttpRequest &a_http_request);

	void Lock() const { semaphore->Op(0, -1); }
	void Unlock() const { semaphore->Op(0, 1); }
	const znp__session& GetPluginSession() const { return plugin_session; }
	const std::string& GetSessionName() const { return session_name; }

	const std::string& GetBaseURI() const;

	unsigned int GetTimeout() const;
	CObjectData* ObjectLookup(unsigned int a_object_id) const;

	CSessionTimer& GetTimer() { return *session_timer; }

private:
	bool IsAccessable(CHttpRequest &a_http_request);

	znp__session plugin_session;
	std::string session_name;
	unsigned int session_id;
	unsigned int object_id_producer;

	std::string base_uri;
	ZSSemaphore *semaphore;
	bool initialized;
	std::map<unsigned int, CObjectData*> objects;
	std::set<CForm*> *forms;

	unsigned int timeout;
	CSessionTimer *session_timer;

	// this variable is still not thread safe !!!
	static unsigned int session_id_producer;
};



class CSessionTimer : public ZSThread {
public:
	CSessionTimer(const zn__session *a_master_session, unsigned int a_timeout);
	~CSessionTimer();

	void Stop();
	void Restart();

private:
	static int internRun(CSessionTimer &a_this);
	static void internAbort(CSessionTimer &a_this);

	unsigned int timeout;
	const zn__session *master_session;
	ZSReader reader;
	ZSWriter writer;
};

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */

#endif
