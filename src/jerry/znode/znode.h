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

#ifndef ZNODE_H
#define ZNODE_H
#include <map>
#include <set>
#include <string>
#include <libxml/tree.h>
#include "jerry/znode/zn_api.h"

namespace jerry {
namespace znode {

typedef void(*init_plugin_t)(struct zn__api *a_api, xmlNode *plugin_node);

extern std::map<std::string, znp__api*> plugins;
extern std::set<std::string> session_types;

#ifdef API_DEBUG
extern std::map<znp__api*, znp__api*> debug_plugins;
void DEBUG_server_api(const zn__session *a_session);
znp__api* DEBUG_get_plugin_api(znp__api *a_plugin_api, znp__session *a_session);
#endif

extern bool zn__is_daemon;
extern const char *zn__config_file;

int Main(void *a_null);

} /* namespace znode */
} /* namespace jerry */

#endif

