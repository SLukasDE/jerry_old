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

#ifndef C_JERRY_PLUGIN_WEB_PLUGIN_H
#define C_JERRY_PLUGIN_WEB_PLUGIN_H

#include <map>
#include <list>
#include <zsystem/zsstream.h>
#include <zsystem/zssemaphore.h>
#include "jerry/znode/zn_api.h"
#include <libxml/tree.h>

namespace jerry {
namespace plugin {
namespace web {

extern struct znp__api plugin_api;
void init_plugin(struct zn__api *a_srv_api, xmlNode * a_config_node);

#define SERVER_API          ( ((struct znp__plugin_data*) plugin_api.priv_data)\
                              ->server_api )
#define REGISTRY            ( ((struct znp__plugin_data*) plugin_api.priv_data)\
                              ->registry )
#define SEMAPHORE           ( ((struct znp__plugin_data*) plugin_api.priv_data)\
                              ->semaphore )
#define LOCK_SESSION_LIST   ( ((struct znp__plugin_data*) plugin_api.priv_data)\
                              ->lock_session_list )
#define UNLOCK_SESSION_LIST ( ((struct znp__plugin_data*) plugin_api.priv_data)\
                              ->unlock_session_list )
/*
#define URL_TO_SESSION      ( ((struct znp__plugin_data*) plugin_api.priv_data)\
                              ->url_to_session )
#define REGISTER_URL       ( ((struct znp__plugin_data*) plugin_api.priv_data)\
                             ->register_url )
#define UNREGISTER_URL     ( ((struct znp__plugin_data*) plugin_api.priv_data)\
                             ->unregister_url )
#define UNREGISTER_SESSION ( ((struct znp__plugin_data*) plugin_api.priv_data)\
                             ->unregister_session )
*/


class CSession;
class SessionRegistry;

//typedef void (*register_url_t)       (const std::string &, CSession *);
//typedef void (*unregister_url_t)     (const std::string &);
//typedef void (*unregister_session_t) (CSession *);
typedef std::list<CSession*>* (*lock_session_list_t)  (void);
typedef void                  (*unlock_session_list_t)(void);

struct znp__plugin_data {
	struct zn__api                   *server_api;
	std::map<std::string, SessionRegistry*> registry;
	ZSSemaphore                      *semaphore;
	std::list<CSession*>             session_list;
	lock_session_list_t              lock_session_list;
	unlock_session_list_t            unlock_session_list;
};

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */

#endif
