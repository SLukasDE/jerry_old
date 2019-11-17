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

#include "jerry/plugin/web/SessionRegistry.h"
#include <set>
#include <stdio.h>
#include <iostream>
#include <string.h>

namespace jerry {
namespace plugin {
namespace web {

SessionRegistry::SessionRegistry(xmlNode *cur_node)
: base_class(0)
{
	for(_xmlAttr * cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {
		if(cur_attr->name == 0)
			continue;
		if(strcmp((const char*) cur_attr->name, "name") == 0) {
			sessionName = (const char*) xmlGetProp(cur_node, cur_attr->name);
		}
		else if(strcmp((const char*) cur_attr->name, "extends") == 0) {
			str_base_class = (const char*) xmlGetProp(cur_node, cur_attr->name);
		}
		else {
			std::cout << "Waring: In configuration file: there is an unknown attribute \"" << cur_attr->name << "\" in node \"<" << cur_node->name << ">\" specified.\n";
			return;
		}
	}

	for (cur_node = cur_node->children; cur_node; cur_node = cur_node->next) {
		if (cur_node->type != XML_ELEMENT_NODE)
			continue;
		if(cur_node->name == 0)
			continue;
		if(strcmp((const char*) cur_node->name, "constructor") == 0) {
			std::string a_url;
			std::string a_host;
			std::string a_session_prefix;
			bool a_url_set  = false;
			bool a_host_set = false;

			for(_xmlAttr * cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {
				if(cur_attr->name == 0)
					continue;

				if(strcmp((const char*) cur_attr->name, "url") == 0) {
					a_url_set = true;
					a_url = (const char*) xmlGetProp(cur_node, cur_attr->name);
				}
				else if(strcmp((const char*) cur_attr->name, "host") == 0) {
					a_host_set = true;
					a_host = (const char*) xmlGetProp(cur_node, cur_attr->name);
				}
				else if(strcmp((const char*) cur_attr->name, "session_prefix") == 0) {
					a_session_prefix = (const char*) xmlGetProp(cur_node, cur_attr->name);
				}
				else {
					std::cerr << "Error in configuration file: there is an unknown attribute \"" << cur_attr->name << "\" in node \"<" << cur_node->name << ">\" specified." << std::endl;
				}
			}

			if(a_url_set && a_host_set) {
				AddConstructor(a_host, a_url, a_session_prefix == "" ? a_host : a_session_prefix);
			}
			else if(a_url_set) {
				std::cerr << "Error in configuration file: Uncomplete attribute list in node \"<" << cur_node->name << ">\" specified: \"host\" is missing." << std::endl;
			}
			else if(a_host_set) {
				std::cerr << "Error in configuration file: Uncomplete attribute list in node \"<" << cur_node->name << ">\" specified: \"url\" is missing." << std::endl;
			}

			continue;
		}
		if(strcmp((const char*) cur_node->name, "plugin_web_files") == 0) {
			std::string a_directory;
			std::string a_prefix;
			bool a_directory_set  = false;

			for(_xmlAttr * cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {
				if(cur_attr->name == 0)
					continue;

				if(strcmp((const char*) cur_attr->name, "directory") == 0) {
					a_directory_set = true;
					a_directory = (const char*) xmlGetProp(cur_node, cur_attr->name);
				}
				else if(strcmp((const char*) cur_attr->name, "prefix") == 0) {
					a_prefix = (const char*) xmlGetProp(cur_node, cur_attr->name);
				}
				else {
					std::cerr << "Error in configuration file: there is an unknown attribute \"" << cur_attr->name << "\" in node \"<" << cur_node->name << ">\" specified." << std::endl;
				}
			}

			if(a_directory_set) {
				AddDirectory(a_directory, a_prefix);
			}
			else {
				std::cerr << "Error in configuration file: Uncomplete attribute list in node \"<" << cur_node->name << ">\" specified: \"directory\" is missing." << std::endl;
			}

			continue;
		}
		std::cerr << "Error in configuration file: there is an unknown node \"<" << cur_node->name << ">\" specified." << std::endl;
	}
}


SessionRegistry::~SessionRegistry() {
	for(Constructors::iterator a_iter = constructors.begin(); a_iter != constructors.end(); ++a_iter) {
		delete *a_iter;
	}

	for(Directories::iterator a_iter = directories.begin(); a_iter != directories.end(); ++a_iter) {
		delete *a_iter;
	}
}

void SessionRegistry::LinkInheritance(const std::map<std::string, SessionRegistry*> &a_registry) {
	std::map<std::string, SessionRegistry*>::const_iterator a_iter;

	// If we are the default class
	a_iter = a_registry.find("");
	if(a_iter != a_registry.end() && a_iter->second == this) {
		base_class = 0;
		return;
	}

	// lookup the base class
	a_iter = a_registry.find(str_base_class);
	if(a_iter != a_registry.end()) {
		base_class = a_iter->second;
		return;
	}

	base_class = 0;
}

bool SessionRegistry::CheckCycle() const {
	std::set<const SessionRegistry*> linked_classes;

	for(const SessionRegistry *a_iter = this; a_iter != 0; a_iter = a_iter->base_class) {
		if(linked_classes.find(a_iter) != linked_classes.end())
			return true;
		linked_classes.insert(a_iter);
	}
	return false;
}

void SessionRegistry::AddConstructor(const std::string &a_host, const std::string &a_url, const std::string &a_session_prefix) {
	Constructor* constructor = new Constructor;
	constructor->url = a_url;
	constructor->host = a_host;
	constructor->session_prefix = a_session_prefix;
	constructors.push_back(constructor);
}

SessionRegistry::Constructor* SessionRegistry::GetConstructor(const std::string &a_host, const std::string &a_url) const {
	for(Constructors::const_iterator a_iter = constructors.begin(); a_iter != constructors.end(); ++a_iter) {
		if((*a_iter)->url == a_url && (*a_iter)->host == a_host)
			return *a_iter;
	}
	return 0;
}

void SessionRegistry::AddDirectory(const std::string &a_directory, const std::string &a_prefix) {
	Directory* directory = new Directory;
	directory->directory = a_directory;
	directory->prefix = a_prefix;
	directories.push_back(directory);
}


std::string SessionRegistry::GetPath(const std::string &a_url) const {
	for(Directories::const_iterator a_iter = directories.begin(); a_iter != directories.end(); ++a_iter) {
		std::string a_path = (*a_iter)->directory + a_url;
		FILE *a_fd;

		a_fd = fopen(a_path.c_str(), "r");
		if(a_fd) {
			fclose(a_fd);
			return a_path;
		}
	}
	if(base_class) {
		return base_class->GetPath(a_url);
	}
	return "";
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
