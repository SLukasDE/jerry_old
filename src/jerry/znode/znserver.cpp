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

#include "jerry/znode/znserver.h"

#include <iostream>
#include <zsystem/zsstream.h>

namespace jerry {
namespace znode {

ZNServer::ZNServer(const char *a_ipv4, int a_port)
: ZSServer(ZSServer::spTCP, a_ipv4, a_port),
  matchcodes(new std::map<std::string, znp__api*>())
{
}

ZNServer::ZNServer(const char *a_name)
: ZSServer(ZSServer::spLocal, a_name),
  matchcodes(new std::map<std::string, znp__api*>())
{
}

ZNServer::~ZNServer() {
	delete matchcodes;
}

bool ZNServer::Register(const std::string &a_matchcode, znp__api *a_plugin_api) {
	if(matchcodes->find(a_matchcode) != matchcodes->end())
		return false;
	matchcodes->insert(std::make_pair(a_matchcode, a_plugin_api));
	return true;
}

std::deque<std::string> ZNServer::GetMatchcodeList() const {
	std::deque<std::string> a_back;

	for(std::map<std::string, znp__api*>::iterator iter=matchcodes->begin(); iter!=matchcodes->end(); ++iter)
		a_back.push_back(iter->first);
	return a_back;
}

znp__api* ZNServer::GetPlugin(const std::string &a_matchcode) const {
	std::map<std::string, znp__api*>::iterator iter;

	iter = matchcodes->find(a_matchcode);
	if(iter == matchcodes->end())
		return 0;
	return iter->second;
}

void ZNServer::OnConnect(zs__stream *a_stream_struct) {
	std::deque<std::string> matchcode_list(GetMatchcodeList());
	std::string a_matchcode;
	znp__api *a_plugin_api;

	// "First Longest Match" - Suche
	while(!matchcode_list.empty()) {
		// Alle l�schen, die leer sind
		for(std::deque<std::string>::iterator iter=matchcode_list.begin(); iter!=matchcode_list.end();)
			if(iter->size() == 0)
				iter = matchcode_list.erase(iter);
			else
				++iter;
		if(!matchcode_list.empty()) {
			char a_char;

			while(zs__stream_read(a_stream_struct, &a_char, 1) == 0);
				a_matchcode += a_char;
				// a_matchcode += (a_char = zslib->stream_read(a_stream_struct));
			for(std::deque<std::string>::iterator iter=matchcode_list.begin(); iter!=matchcode_list.end();)
				if(*iter->c_str() != a_char)
					iter = matchcode_list.erase(iter);
				else {
					*iter = std::string(&iter->c_str()[1]);
					++iter;
				}
		}
	}
	a_plugin_api = GetPlugin(a_matchcode);
	if(a_plugin_api) {
		a_plugin_api->response(a_plugin_api, a_stream_struct);
	}
	else {
		std::cerr << "F�r die empfangene Anfrage vom Typ \"" << a_matchcode << "\" ist kein Plugin vorhanden." << std::endl;
		zs__stream_destroy(a_stream_struct);
	}
}

} /* namespace znode */
} /* namespace jerry */
