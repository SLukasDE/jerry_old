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

#ifndef ZNSESSION_H
#define ZNSESSION_H

#include <list>
#include <vector>
#include <map>
#include <string>
#include <libxml/tree.h>
#include <zsystem/zssemaphore.h>
#include <zsystem/zsthread.h>
#include "jerry/znode/zn_api.h"

namespace jerry {
namespace znode {

class ZNObject;
class ZNType;

class ZNSession {
friend class ZNObject;
public:
	static void API_Fill(zn__api &a_api);
	static void Release();

#ifdef API_DEBUG
	static void Plugin_API_Fill(znp__api &a_plugin_api);
#endif
	ZNSession(const char *a_session_name, const znp__api *a_plugin);
	~ZNSession();
	const zn__session& GetData() const { return data; }
	const std::vector<znp__session*> GetPluginSessions() const { return plugin_sessions; }


	ZNType& GetType(const std::string &a_type);
	void RegisterType(ZNType &a_type);
	void RemoveType(ZNType &a_type);

private:
	ZNObject *object_first;
	mutable std::map<std::string, ZNType*> types;

	zn__session data;

	const znp__session *slave_current;
	ZSSemaphore semaphore;
	std::vector<znp__session*> plugin_sessions;

	static std::list<ZNSession*> sessions;

	static znp__session* api_PluginSessionCreate(const zn__session *a_session, const char *a_plugin, const char *a_session_name);
	static void api_PluginSessionDestroy(znp__session *a_session);

	static const zn__session* api_SessionCreate(const char *a_session_type, const znp__api *a_plugin);
	static void api_SessionInit      (const zn__session *a_session);
	static void api_SessionEvent     (const znp__session *a_slave_session, void *a_data);
	static void api_SessionDestroy   (const zn__session *a_session);

#ifdef API_DEBUG
	static znp__session* api_DEBUG_PluginSessionCreate(const zn__session *a_session, const char *a_plugin_name, const char *a_session_name);
	static void api_DEBUG_PluginSessionDestroy(znp__session *a_session);

	static const zn__session* api_DEBUG_SessionCreate(const char *a_session_type, const znp__api *a_plugin);
	static void api_DEBUG_SessionInit      (const zn__session *a_session);
	static void api_DEBUG_SessionEvent     (const znp__session *a_session, void *a_data);
	static void api_DEBUG_SessionDestroy   (const zn__session *a_session);

	// Redirect for plugin APIs
	static znp__session* api_plugin_DEBUG_session_create(znp__api *a_plugin_api, const zn__session *_session, const char *session_type);
	static void api_plugin_DEBUG_session_init      (znp__session *a_session);
	static void api_plugin_DEBUG_session_event     (znp__session *a_session, void *data);
	static void api_plugin_DEBUG_session_destroy   (znp__session *a_session);
#endif
};

class ZNSessionDestroy : public ZSThread {
public:
	ZNSessionDestroy(ZNSession &a_session);

private:
	static int internRun(ZNSessionDestroy &a_this);
	ZNSession &session;
};

} /* namespace znode */
} /* namespace jerry */

#endif
