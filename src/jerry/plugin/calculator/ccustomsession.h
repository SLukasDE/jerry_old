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

#ifndef C_JERRY_PLUGIN_CALCULATOR_CUSTOMSESSION_H
#define C_JERRY_PLUGIN_CALCULATOR_CUSTOMSESSION_H

#include "jerry/plugin/calculator/cform.h"
//#include <map>
#include <list>
#include <zsystem/zsstream.h>
#include "jerry/znode/zn_api.h"
#include <libxml/tree.h>

namespace jerry {
namespace plugin {
namespace calculator {

extern struct znp__api plugin_api;

void init_plugin(struct zn__api *a_srv_api, xmlNode * a_config_node);

#define SERVER_API ( ((struct znp__plugin_data*) plugin_api.priv_data)\
                     ->server_api )
#define GET_SESSION(ps) ((CCustomSession*) ps->priv_data)

class CSession;
class CObjectData;
class CForm;

struct znp__plugin_data {
	struct zn__api *server_api;
};

class CCustomSession {
friend class CObjectData;
public:
	static znp__session* api_Create(znp__api *, const zn__session *, const char *a_session_type);
	static void api_Destroy(znp__session *a_plugin_session);
	static void api_Init(znp__session *a_plugin_session);
	static void api_Event  (znp__session *a_plugin_session, void *a_data);

	CForm& GetScreen() const;
//	CObjectData* ObjectLookup(unsigned int a_object_id) const;

protected:
	CCustomSession(const zn__session *a_session);
	virtual ~CCustomSession();
	virtual void SessionInit() = 0;

private:
	znp__session plugin_session;
	std::list<CObjectData*> objects;

	CForm *screen;
};

} /* namespace calculator */
} /* namespace plugin */
} /* namespace jerry */

#endif
