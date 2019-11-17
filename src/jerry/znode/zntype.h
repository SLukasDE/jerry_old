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

#ifndef ZNTYPE_H
#define ZNTYPE_H

#include <string>
#include <vector>
#include <list>
#include <set>
#include "jerry/znode/zn_api.h"

namespace jerry {
namespace znode {

class ZNSession;
class ZNType_Constructor;

class ZNType {
public:
	static void API_Fill(zn__api &a_api);

	ZNType(ZNSession &a_session, const std::string &a_name);
	~ZNType();

	const std::string& GetName() const { return name; }
	bool IsBad() const { return error; }

	void PluginSessionAdd();
	void PluginSessionDelete(unsigned int i);

private:
	ZNType_Constructor& GetConstructor(unsigned int a_argc, znp__type *a_argv);

	ZNSession &session;
	std::string name;
	bool error;
	ZNType *base;
	unsigned int plugin_count;
	std::list<ZNType_Constructor*> constructors;
	std::set<ZNType*> upper_types;

};

class ZNType_Constructor {
public:
	ZNType_Constructor(unsigned int a_argc, znp__type *a_argv);

	bool IsBad() const { return error; }
	bool IsConstructor(unsigned int a_argc, znp__type *a_argv) const;
	void SetBaseCall(unsigned int a_argc, znp__type *a_argv);

	void PluginSessionAdd(znp__type_source a_availability, const std::list<ZNType_Constructor*> *other_constructors);
	void PluginSessionDelete(unsigned int i);

private:
	std::vector<znp__type_source> availability;
	std::vector<znp__type> args;
	std::vector<std::string> args_pointer;
	std::vector<znp__type> base_call_args;
	std::vector<std::string> base_call_args_pointer;
	bool error;
	bool base_call_set;
};

} /* namespace znode */
} /* namespace jerry */

#endif
