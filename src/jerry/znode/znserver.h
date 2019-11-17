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

#ifndef SLZN_SERVER_H
#define SLZN_SERVER_H

#include <map>
#include <deque>
#include <string>
#include <zsystem/zsserver.h>
#include "jerry/znode/zn_api.h"

/**
  *@author Sven Lukas
  */

namespace jerry {
namespace znode {

class ZNServer : public ZSServer  {
public:
	ZNServer(const char *a_ipv4, int a_port);
	ZNServer(const char *a_name);

	bool Register(const std::string &a_matchcode, znp__api *a_presenter);
	std::deque<std::string> GetMatchcodeList() const;
	znp__api* GetPlugin(const std::string &) const;

protected:
	~ZNServer();
	void OnConnect(zs__stream *a_stream_struct);

private:
	std::map<std::string, znp__api*> *matchcodes;
};

} /* namespace znode */
} /* namespace jerry */

#endif
