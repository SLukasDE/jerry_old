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

#ifndef C_JERRY_PLUGIN_WEB_HTTPREQUEST_H
#define C_JERRY_PLUGIN_WEB_HTTPREQUEST_H

#include "jerry/plugin/web/csession.h"
#include "jerry/plugin/web/SessionRegistry.h"
#include <zsystem/zsstream.h>
#include <string>
#include <map>
#include <list>
#include "jerry/znode/zn_api.h"

namespace jerry {
namespace plugin {
namespace web {

class CSession;

struct http_property {
	CObjectData *object;
	std::string property;
	znp__value value;
};

class CHttpRequest {
public:
	CHttpRequest(zs__stream *a_stream_struct);
	~CHttpRequest();

	void OpenAnswer(unsigned int a_http_code, const std::string &a_content_type, const std::string &a_location);
	int AnswerFile();

	const std::string& GetHost() const {
		return host;
	}

	const std::string& GetUrl() const {
		return http_url;
	}

	const std::string& GetReferer() const {
		return http_referer;
	}

	const std::string& GetHttpContent() const {
		return http_content;
	}

	ZSWriter& GetStreamWriter() {
		return writer;
	}

	const CSession* GetSession() const {
		return session;
	}

	const std::string&  GetSubUrl() const {
		return sub_url;
	}

	const SessionRegistry* GetRegistry() const {
		return registry;
	}

	const std::pair<std::string, SessionRegistry::Constructor*>& GetConstructor() const {
		return constructor;
	}

	const std::string& GetPath() const {
		return path;
	}

	const char* GetRemoteIPv4Address() {
		return reader.GetRemoteIPv4Address();
	}

	const char* GetLocalIPv4Address() {
		return reader.GetLocalIPv4Address();
	}

	const std::list<http_property>& GetProperties() const {
		return http_properties;
	}

	CObjectData* GetCallObject() const {
		return call_object;
	}

	const std::string& GetCallFunction() const {
		return call_function;
	}

	const znp__value* GetArguments() const {
		return call_arguments;
	}

	unsigned int GetArgumentCount() const {
		return call_argument_count;
	}

private:

	void ReadHeader();

	ZSReader reader;
	ZSWriter writer;
	bool connection_close;
	bool answer_opened;
	const CSession *session;
	std::string sub_url;
	const SessionRegistry *registry;

	std::pair<std::string, SessionRegistry::Constructor*> constructor;
	std::string path;

	std::string post_param;
	std::string get_param;
	std::string http_url;
	std::string host;
	std::string http_referer;
	std::string http_protocol;
	std::string http_method;
	std::string http_content;
	std::string http_content_type;

	std::list<http_property> http_properties;
	std::list<std::string*> string_values;
	CObjectData* call_object;
	std::string call_function;
	znp__value *call_arguments;
	unsigned int call_argument_count;
};

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */

#endif
