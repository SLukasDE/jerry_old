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

#ifndef SESSIONREGISTRY_H_
#define SESSIONREGISTRY_H_

#include <string>
#include <map>
#include <list>
#include <libxml/tree.h>

namespace jerry {
namespace plugin {
namespace web {

class SessionRegistry {
public:
	struct Constructor {
		std::string url;
		std::string host;
		std::string session_prefix;
	};
	typedef std::list<Constructor*> Constructors;

	struct Directory {
		std::string directory;
		std::string prefix;
	};
	typedef std::list<Directory*> Directories;

	SessionRegistry(xmlNode *cur_node);
	~SessionRegistry();

	const std::string& getSessionName() const { return sessionName; }


	void LinkInheritance(const std::map<std::string, SessionRegistry*> &a_registry);
	bool CheckCycle() const;

	void AddConstructor(const std::string &a_host, const std::string &a_url, const std::string &a_session_prefix);
	Constructor* GetConstructor(const std::string &a_host, const std::string &a_url) const;

	void AddDirectory(const std::string &a_directory, const std::string &a_prefix);
	std::string GetPath(const std::string &a_url) const;

private:
	std::string sessionName;
	std::string str_base_class;
	SessionRegistry* base_class;

	Constructors constructors;
	Directories directories;
};

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */

#endif /* SESSIONREGISTRY_H_ */
