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

#include "jerry/plugin/web/chttprequest.h"
#include "jerry/plugin/web/cobject.h"
#include "jerry/plugin/web/cplugin.h"
#include <iostream>
#include <fcntl.h>
#include <algorithm>

namespace jerry {
namespace plugin {
namespace web {

CHttpRequest::CHttpRequest(zs__stream *a_stream_struct)
:	answer_opened(false),
	session(0),
	registry(0),
	call_object(0),
	call_arguments(0),
	call_argument_count(0)
{
	std::map<std::string, CSession*>::iterator a_iter1;
	std::pair<ZSReader, ZSWriter> a_reader_writer(ZSStream::Create(a_stream_struct));
	reader = a_reader_writer.first;
	writer = a_reader_writer.second;

	ReadHeader();
}

CHttpRequest::~CHttpRequest() {
	for(std::list<std::string*>::iterator a_iter=string_values.begin(); a_iter!=string_values.end(); ++a_iter)
		delete *a_iter;
	if(call_argument_count > 0)
		delete[] call_arguments;
}

void CHttpRequest::OpenAnswer(unsigned int a_http_code, const std::string &a_content_type, const std::string &a_location) {
	if(answer_opened)
		return;

	answer_opened = true;

	switch(a_http_code) {
	case 200:
		writer << "HTTP/1.1 200 OK\r\n";
		break;
	case 302:
		writer << "HTTP/1.1 302 Found\r\n";
		break;
	case 403:
		writer << "HTTP/1.1 403 Access Forbidden\r\n";
		break;
	case 404:
		writer << "HTTP/1.1 404 Not Found\r\n";
		break;
	default:
		writer << "HTTP/1.1 " << a_http_code << " \r\n";
		break;
	}

	if(a_location != "")
		writer << "Location: http://" << a_location << "\r\n";
	writer << "Content-Type: " << a_content_type << "\r\n\r\n";
}

int CHttpRequest::AnswerFile() {
	char c;
	int fd;

	if(GetPath() == "")
		return -2;

	fd = open(GetPath().c_str(), O_RDONLY);
	if(fd == -1)
		return -1;

	OpenAnswer(200, "image/gif", "");
	while(read(fd, &c, 1) == 1)
		GetStreamWriter() << c;
	close(fd);
	return 0;
}

static inline void
ParseHeaderString(std::string::iterator &a_current, std::string::iterator a_end,
                  std::string &a_str)
{
	std::string::iterator a_begin(a_current);

	for(;a_current != a_end && *a_current != ' '; ++a_current) ;
	a_str = std::string(a_begin, a_current);
	if(a_current != a_end)
		++a_current;
}

static inline std::string
ParseHttpArgument(std::string::const_iterator &a_current, std::string::iterator a_end,
                  char stop_char)
{
	std::string a_str;

	for(; a_current != a_end; ++a_current) {
		char a_char(0);

		if(*a_current != '%') {
			a_char = *a_current;
		}
		else {
			++a_current;
			if(a_current == a_end)
				break;
			if(*a_current >= '0' && *a_current <= '9')
				a_char = (a_char << 4) | (0x0f & (*a_current-'0') );
			else if(*a_current >= 'a' && *a_current <= 'f')
				a_char = (a_char << 4) | (0x0f & (*a_current-'a'+10) );
			else if(*a_current >= 'A' && *a_current <= 'F')
				a_char = (a_char << 4) | (0x0f & (*a_current-'A'+10) );
	
			++a_current;
			if(a_current == a_end)
				break;
			if(*a_current >= '0' && *a_current <= '9')
				a_char = (a_char << 4) | (0x0f & (*a_current-'0') );
			else if(*a_current >= 'a' && *a_current <= 'f')
				a_char = (a_char << 4) | (0x0f & (*a_current-'a'+10) );
			else if(*a_current >= 'A' && *a_current <= 'F')
				a_char = (a_char << 4) | (0x0f & (*a_current-'A'+10) );
		}

		if(a_char == stop_char) {
			++a_current;
			break;
		}

		a_str += a_char;
	}

	return a_str;
}

static inline unsigned int
ReadObjectID(std::string::iterator &a_begin, const std::string::const_iterator &a_end) {
	unsigned int  a_back(0);

	for(; a_begin != a_end; ++a_begin) {
		if(*a_begin >= '0' && *a_begin <= '9')
			a_back = (a_back << 4) | ((*a_begin-'0') & 0x0f);
		else if(*a_begin >= 'A' && *a_begin <= 'F')
			a_back = (a_back << 4) | ((*a_begin-'A'+10) & 0x0f);
		else
			break;
	}
	return a_back;
}

void CHttpRequest::ReadHeader() {
	long int a_content_length(0);
	std::string a_str;
	std::string::iterator a_str_iter;
	std::list<CSession*> *a_session_list;
	std::map<unsigned int, znp__value> a_call_arguments;

	// first of all we must read the first line to decode
	// the HTTP-method, -url, and -protocol
	// This line does look like this for example: "GET /favicon.ico HTTP/1.1"
	reader >> a_str;
	a_str.resize(a_str.size()-1);
	a_str_iter = a_str.begin();
	ParseHeaderString(a_str_iter, a_str.end(), http_method);
	ParseHeaderString(a_str_iter, a_str.end(), http_url);
	ParseHeaderString(a_str_iter, a_str.end(), http_protocol);

	// delete URL string beginning with "?"-character
	http_url.erase(std::find(http_url.begin(), http_url.end(), '?'), http_url.end());

	// delete leading "/"-character in URL string if existing
	if(http_url.size() >= 1 && http_url.at(0) == '/')
		http_url.erase(0, 1);

	// delete ending "/"-character in URL string if existing
	if(http_url.size() >= 1 && http_url.at(http_url.size()-1) == '/')
		http_url.erase(http_url.size()-1, 1);

	// now we must read all subsequent lines, especially to dencode
	// the HTTP-content payload
	do {
		reader >> a_str;
		a_str.resize(a_str.size()-1);
//		std::cout << "[WEB]: ReadHeader 2: \"" << a_str << "\"" << std::endl;
		if(a_str.size() >= 6 && a_str.substr(0, 6) == "Host: ")
			host = a_str.substr(6, a_str.size()-6);
		else if(a_str.size() >= 9 && a_str.substr(0, 9) == "Referer: ")
			http_referer = a_str.substr(9, a_str.size()-9);
		else if(a_str.size() >= 14 && a_str.substr(0, 14) == "Content-Type: ")
			http_content_type = a_str.substr(14, a_str.size()-14);
		else if(a_str.size() >= 16 && a_str.substr(0, 16) == "Content-Length: ")
			a_content_length = strtol(a_str.substr(16, a_str.size()-16).c_str(), 0, 0);
	} while(a_str != "");

	for(char a_char; a_content_length>0; --a_content_length) {
		reader >> a_char;
		http_content += a_char;
	}





	// Lookup session
	a_str = GetHost() + "/" + GetUrl();
	a_session_list = LOCK_SESSION_LIST();
	for(std::list<CSession*>::iterator a_iter = a_session_list->begin(); a_iter != a_session_list->end(); ++a_iter) {
		if(a_str.find((*a_iter)->GetBaseURI()) == 0) {
			session = *a_iter;
			break;
		}
	}
	UNLOCK_SESSION_LIST();

	// Lookup Sub-URL
	if(session) {
		a_str = GetHost() + "/" + GetUrl();
		sub_url = a_str.substr(session->GetBaseURI().size());
	}
	else {
		sub_url = GetUrl();
	}
	if(sub_url.size() > 0 && sub_url.at(0) == '/') {
		sub_url = sub_url.substr(1);
	}

	// Lookup registry
	if(session) {
		std::map<std::string, SessionRegistry*>::iterator a_reg_iter;

		a_reg_iter = REGISTRY.find(session->GetSessionName());
		if(a_reg_iter != REGISTRY.end()) {
			registry = a_reg_iter->second;
		}
	}
	if(registry == 0) {
		std::map<std::string, SessionRegistry*>::iterator a_reg_iter;

		a_reg_iter = REGISTRY.find("");
		if(a_reg_iter != REGISTRY.end()) {
			registry = a_reg_iter->second;
		}
	}

	// Lookup path
	if(registry && sub_url != "") {
		path = registry->GetPath("/" + sub_url);
	}

	// Lookup application
	if(session == 0) {
		for(std::map<std::string, SessionRegistry*>::iterator a_iter = REGISTRY.begin();
		    a_iter != REGISTRY.end(); ++a_iter) {
			SessionRegistry::Constructor *a_constr;

			a_constr = a_iter->second->GetConstructor(GetHost(), GetUrl());
			if(a_constr) {
				constructor = std::make_pair(a_iter->first, a_constr);
				break;
			}
		}
	}




/*
	std::cout << "[WEB]: -------------------" << std::endl;
	std::cout << "[WEB]: http_method       = \"" << http_method   << "\"" << std::endl;
	std::cout << "[WEB]: http_url          = \"" << http_url      << "\"" << std::endl;
	std::cout << "[WEB]: host              = \"" << host          << "\"" << std::endl;
	std::cout << "[WEB]: http_protocol     = \"" << http_protocol << "\"" << std::endl;
	std::cout << "[WEB]: http_content_type = \"" << http_content_type << "\"" << std::endl;
	std::cout << "[WEB]: http_content      = \"" << http_content << "\"" << std::endl;
*/
	// Now we must decode the HTTP-content payload to objects and properties
	for(std::string::const_iterator a_str_iter(http_content.begin()); a_str_iter != http_content.end();) {
		std::string a_property;
		std::string a_value;
		std::string::iterator a_iter;

		a_property = ParseHttpArgument(a_str_iter, http_content.end(), '=');
		a_value    = ParseHttpArgument(a_str_iter, http_content.end(), '&');

		if(a_property == "")
			continue;

		// check if we are decoding the argument that tells us
		// the object we must "call"
		if(a_property == "call" ) {
			// for security reasons we must ensure that we
			// we have not got this property twice
			if(call_object || call_function != "") {
				std::cerr << "[WEB Error]: Got the call_object/call_function more than once" << std::endl;
				continue;
			}

			a_iter = a_value.begin();
			if(a_iter == a_value.end())
				continue;

			// If value begins with character 'C', then it's an
			// instance method and we decode the Object-ID first
			if(*a_iter == 'C') {
				if(!session) {
					std::cerr << "[WEB Error]: Call of method without session!" << std::endl;
					continue;
				}
				++a_iter;

				call_object = session->ObjectLookup(ReadObjectID(a_iter, a_value.end()));
				// check if we have got a malformed string or just an invalid object ID
				if(call_object == 0) {
					std::cerr << "[WEB Error]: Call of method without object!" << std::endl;
					continue;
				}

				// go forward if the string is not malformed
				if(a_iter == a_value.end() || *a_iter != '_') {
					call_object = 0;
					continue;
				}
			}
			// if value begins with character 'F' then it is a
			// static function we have to call, so we have skipped
			// the decoding of an Object-ID
			else if(*a_iter == 'F') {
			}
			// Everything else is wrong and we can skip decoding
			// the whole property/value pair
			else
				continue;

			// now we decode the method/static-function name
			++a_iter;
			call_function.assign(a_iter, a_value.end());
			continue;
		}
		// if property begins with character 'A' then it describes an
		// argument for a function call.
		if(*a_property.begin() == 'A') {
			unsigned int a_arg_no;
			znp__value a_argument;
			std::string a_str_value;

			a_iter = a_property.begin();
			++a_iter;
			// Now a number is followed that tells us the argument
			// number. We are using a CHttpProperty-Type so we can
			// use the member object_id to store the number.
			a_arg_no = ReadObjectID(a_iter, a_property.end());

			// skik next decoding if string is malformed
			if(a_iter == a_property.end() || *a_iter != '_')
				continue;
			++a_iter;

			// now check the next character of the property name that signals the
			// the value type. Save the value according to the type subject to being
			// a valid type.
			if(a_iter == a_property.end())
				continue;
			switch(*a_iter) {
			case 'U':
				a_argument.v_type = zn__vtUInt;
				sscanf(a_value.c_str(), "%d", &a_argument.v_uint);
				break;
			case 'I':
				a_argument.v_type = zn__vtSInt;
				sscanf(a_value.c_str(), "%d", &a_argument.v_sint);
				break;
			case 'S':
				a_argument.v_type = zn__vtString;
				a_str_value = a_value;
				break;
			case 'B':
				a_argument.v_type = zn__vtBool;
				if(a_value == "true")
					a_argument.v_bool = true;
				else if(a_value == "false")
					a_argument.v_bool = false;
				else
					a_argument.v_type = zn__vtUnknown;
				break;
			case 'P': {
				unsigned int a_object_id;
				CObjectData *a_object;

				a_argument.v_type = zn__vtPointer;
				sscanf(a_value.c_str(), "%x", &a_object_id);
				if(session) {
					a_object = session->ObjectLookup(a_object_id);
				}
				else {
					std::cerr << "[WEB Error]: Got pointer argument without session!" << std::endl;
					a_object = 0;
				}
				a_argument.v_pointer = a_object ? &a_object->GetObjectBase() : 0;
				break;
			}
			default:
				a_argument.v_type = zn__vtUnknown;
				break;
			}
			if(a_argument.v_type == zn__vtUnknown)
				continue;

			// store this argument
			if(a_call_arguments.find(a_arg_no) != a_call_arguments.end()) {
				std::cerr << "[WEB Error]: Multiple argument no. " << a_arg_no << " !" << std::endl;
				continue;
			}
			if(a_argument.v_type == zn__vtString) {
				std::string *a_str_ptr;

				a_str_ptr = new std::string(a_str_value);
				string_values.push_back(a_str_ptr);
				a_argument.v_string = a_str_ptr->c_str();
			}
			a_call_arguments.insert(std::make_pair(a_arg_no, a_argument));
			continue;
		}
		// if property begins with character 'C' then it describes an
		// object property that has changed.
		if(*a_property.begin() == 'C') {
			http_property a_http_property;
			std::string a_str_value;

			if(!session) {
				std::cerr << "[WEB Error]: Set value of object without session!" << std::endl;
				continue;
			}

			// Set Object ID and go forward within the property string
			a_iter = a_property.begin();
			++a_iter;

			// Now a number is followed that tells which object
			// has a changed argument.
			a_http_property.object = session->ObjectLookup(ReadObjectID(a_iter, a_property.end()));

			// check if we have got a malformed string or just an invalid object ID
			if(!a_http_property.object) {
				std::cerr << "[WEB Error]: Set value of object without instance!" << std::endl;
				continue;
			}

			// skik next decoding if string is malformed
			if(a_iter == a_property.end() || *a_iter != '_')
				continue;
			++a_iter;

			// now check the next character of the property name that signals the
			// the value type. Save the value according to the type subject to being
			// a valid type.
			if(a_iter == a_property.end())
				continue;
			switch(*a_iter) {
			case 'U':
				a_http_property.value.v_type = zn__vtUInt;
				sscanf(a_value.c_str(), "%d", &a_http_property.value.v_uint);
				break;
			case 'I':
				a_http_property.value.v_type = zn__vtSInt;
				sscanf(a_value.c_str(), "%d", &a_http_property.value.v_sint);
				break;
			case 'S':
				a_http_property.value.v_type = zn__vtString;
				a_str_value = a_value;
				break;
			case 'B':
				a_http_property.value.v_type = zn__vtBool;
				if(a_value == "true")
					a_http_property.value.v_bool = true;
				else if(a_value == "false")
					a_http_property.value.v_bool = false;
				else
					a_http_property.value.v_type = zn__vtUnknown;
				break;
			case 'P': {
				CObjectData *a_object;
				unsigned int a_object_id;

				a_http_property.value.v_type = zn__vtPointer;
				sscanf(a_value.c_str(), "%x", &a_object_id);
				if(session) {
					a_object = session->ObjectLookup(a_object_id);
				}
				else {
					std::cerr << "[WEB Error]: Set pointer value without session!" << std::endl;
					a_object = 0;
				}
				a_http_property.value.v_pointer = a_object ? &a_object->GetObjectBase() : 0;
				break;
			}
			default:
				a_http_property.value.v_type = zn__vtUnknown;
				break;
			}
			if(a_http_property.value.v_type == zn__vtUnknown)
				continue;
			++a_iter;

			// last but not least save the remaining string of "property" that IS
			// the property name.
			a_http_property.property.assign(a_iter, a_property.end());

			// store this property/value/type-triple
			if(a_http_property.value.v_type == zn__vtString) {
				std::string *a_str_ptr;

				a_str_ptr = new std::string(a_str_value);
				string_values.push_back(a_str_ptr);
				a_http_property.value.v_string = a_str_ptr->c_str();
			}
			http_properties.push_back(a_http_property);
			continue;
		}
	}

	// here we put the arguments into the "real" array and check if all arguments are present
	call_argument_count = a_call_arguments.size();
	if(call_argument_count == 0 || call_function == "") {
		call_arguments = 0;
	}
	else {
		std::map<unsigned int, znp__value>::iterator a_iter;
		unsigned int i;

		call_arguments = new znp__value[call_argument_count];

		for(i=0; i<call_argument_count; ++i) {
			a_iter = a_call_arguments.find(i);
			if(a_iter == a_call_arguments.end())
				break;
			call_arguments[i] = a_iter->second;
		}
		if(i != call_argument_count) {
			std::cerr << "[WEB Error]: Broken arguments, could not find argument " << (i+1) << " of " << call_argument_count << "!" << std::endl;
			delete[] call_arguments;
			call_arguments = 0;
			call_argument_count = 0;
		}
	}

#if 1
// DEBUG output
// show http_properties
for(std::list<http_property>::iterator a_iter=http_properties.begin(); a_iter != http_properties.end(); ++a_iter) {
	std::cout << "[WEB]: Property Obj(" << a_iter->object->GetID() << ")->" <<  (*a_iter).property << " = ";
	switch((*a_iter).value.v_type) {
	case zn__vtUInt:
		std::cout << "UInt(" << a_iter->value.v_uint << ")" << std::endl;
		break;
	case zn__vtSInt:
		std::cout << "SInt(" << a_iter->value.v_sint << ")" << std::endl;
		break;
	case zn__vtString:
		std::cout << "String(" << a_iter->value.v_string << ")" << std::endl;
		break;
	case zn__vtBool:
		std::cout << "Bool(" << a_iter->value.v_bool << ")" << std::endl;
		break;
	case zn__vtPointer:
		std::cout << "Pointer(" << a_iter->value.v_uint << ")" << std::endl;
		break;
	default:
		std::cout << "Unknown(?)" << std::endl;
		break;
	}
}

if(call_function!="") {
	if(call_object)
		std::cout << "[WEB]: Call Obj(" << call_object->GetID() << ")->" << call_function << "(";
	else
		std::cout << "[WEB]: Call ::" << call_function << "(";
	for(unsigned int i=0; i<call_argument_count; ++i) {
		if(i > 0)
			std::cout << ", ";

		switch(call_arguments[i].v_type) {
		case zn__vtUInt:
			std::cout << "UInt(" << call_arguments[i].v_uint << ")";
			break;
		case zn__vtSInt:
			std::cout << "SInt(" << call_arguments[i].v_sint << ")";
			break;
		case zn__vtString:
			std::cout << "String(" << call_arguments[i].v_string << ")";
			break;
		case zn__vtBool:
			std::cout << "Bool(" << call_arguments[i].v_bool << ")";
			break;
		case zn__vtPointer:
			std::cout << "Pointer(" << call_arguments[i].v_uint << ")";
			break;
		default:
			std::cout << "Unknown(?)";
			break;
		}
	}
	std::cout << ")" << std::endl;
}
#endif
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
