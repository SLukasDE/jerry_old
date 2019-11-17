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

#include "jerry/znode/zntype.h"

#include <iostream>

#include "jerry/znode/znode.h"
#include "jerry/znode/znsession.h"

namespace jerry {
namespace znode {

void ZNType::API_Fill(zn__api &a_api) {
}

ZNType::ZNType(ZNSession &a_session, const std::string &a_name)
: session(a_session),
  name(a_name),
  error(false),
  base(0),
  plugin_count(0)
{
	session.RegisterType(*this);
}

ZNType::~ZNType() {
	session.RemoveType(*this);

	for(std::set<ZNType*>::iterator a_iter = upper_types.begin(); a_iter != upper_types.end(); ++a_iter) {
		(*a_iter)->base = 0;
	}

	if(base)
		base->upper_types.erase(this);
}

void ZNType::PluginSessionAdd() {
	const znp__scheme *a_scheme;
	std::string a_base_str;

	// First we send the request to the new plugin to get ready for the next step.
	session.GetPluginSessions()[plugin_count]->plugin_api->scheme_request(session.GetPluginSessions()[plugin_count], name.c_str());
	// Next we get the scheme of the new plugin.
	a_scheme = session.GetPluginSessions()[plugin_count]->plugin_api->scheme_get(session.GetPluginSessions()[plugin_count], name.c_str());

	// just add default values for the new plugin if it does not response with a scheme
	if(!a_scheme) {
		for(std::list<ZNType_Constructor*>::iterator a_iter = constructors.begin(); a_iter != constructors.end(); ++a_iter)
			(*a_iter)->PluginSessionAdd(zn__tsNotAvaliable, 0);
		++plugin_count;
		return;
	}



	// ### ####################### ###
	// ### Checking for Base-Class ###
	// ### ####################### ###

	// set a_base_str to the base name in the new scheme.
	if(a_scheme
	&& a_scheme->base)
		a_base_str = a_scheme->base;

	// check if new scheme has correct base class.
	if(plugin_count != 0) {
		if( (base && a_base_str != base->GetName())
		|| (!base && a_base_str != "")) {
			error = true;
			std::cerr << "[Server Error]: Plugin " << session.GetPluginSessions()[plugin_count] << " has a wrong base_class \"" << a_base_str << "\"." << std::endl;
		}
	}



	// ### ######################### ###
	// ### Checking for Constructors ###
	// ### ######################### ###

	if(a_scheme) {
		for(unsigned int j=0; j<a_scheme->constructor_count; ++j) {
			ZNType_Constructor *a_constructor;

			a_constructor = &GetConstructor(a_scheme->constructors[j].argc, a_scheme->constructors[j].argv);
			a_constructor->PluginSessionAdd(a_scheme->constructors[j].availability, &constructors);
			a_constructor->SetBaseCall(a_scheme->constructors[j].base_call_argc, a_scheme->constructors[j].base_call_argv);
			if(a_constructor->IsBad())
				error = true;
		}
	}



	// ### ###################### ###
	// ### Getting for Base-Class ###
	// ### ###################### ###

	if(a_base_str != "" && plugin_count == 0) {
		base = &session.GetType(a_base_str);
		base->upper_types.insert(this);
	}

	++plugin_count;
}

void ZNType::PluginSessionDelete(unsigned int i) {
	for(std::list<ZNType_Constructor*>::iterator a_iter = constructors.begin(); a_iter != constructors.end(); ++a_iter) {
		(*a_iter)->PluginSessionDelete(i);
	}
}

ZNType_Constructor& ZNType::GetConstructor(unsigned int a_argc, znp__type *a_argv) {
	ZNType_Constructor *a_constructor;

	for(std::list<ZNType_Constructor*>::iterator a_iter = constructors.begin(); a_iter != constructors.end(); ++a_iter) {
		if((*a_iter)->IsConstructor(a_argc, a_argv))
			return **a_iter;
	}
	a_constructor = new ZNType_Constructor(a_argc, a_argv);
	constructors.push_back(a_constructor);
	return *a_constructor;
}







ZNType_Constructor::ZNType_Constructor(unsigned int a_argc, znp__type *a_argv)
: args(a_argc),
  args_pointer(a_argc, ""),
  base_call_args(a_argc),
  base_call_args_pointer(a_argc, ""),
  error(false),
  base_call_set(false)
{
	for(unsigned int i=0; i<a_argc; ++i) {
		args[i].v_type = a_argv[i].v_type;
		if(a_argv[i].v_type == zn__vtPointer) {
			args_pointer[i] = a_argv[i].v_pointer;
			args[i].v_pointer = args_pointer[i].c_str();
		}
	}
}

bool ZNType_Constructor::IsConstructor(unsigned int a_argc, znp__type *a_argv) const {
	if(a_argc != args.size())
		return false;
	for(unsigned int i=0; i<a_argc; ++i) {
		if( (args[i].v_type != a_argv[i].v_type)
		 || (a_argv[i].v_type == zn__vtPointer
		  && args_pointer[i] != a_argv[i].v_pointer) )
			return false;
	}
	return true;
}

void ZNType_Constructor::SetBaseCall(unsigned int a_argc, znp__type *a_argv) {
	if(!base_call_set) {
		for(unsigned int i=0; i<a_argc; ++i) {
			base_call_args[i].v_type = a_argv[i].v_type;
			if(a_argv[i].v_type == zn__vtPointer) {
				base_call_args_pointer[i] = a_argv[i].v_pointer;
				base_call_args[i].v_pointer = base_call_args_pointer[i].c_str();
			}
		}
		base_call_set = true;
		return;
	}

	if(a_argc != base_call_args.size()) {
		error = true;
		std::cerr << "[Server Error]: Cannot set base call of constructor with different signature." << std::endl;
		return;
	}
	for(unsigned int i=0; i<a_argc; ++i) {
		if( (args[i].v_type != a_argv[i].v_type)
		 || (a_argv[i].v_type == zn__vtPointer
		  && args_pointer[i] != a_argv[i].v_pointer) ) {
			error = true;
			std::cerr << "[Server Error]: Cannot set base call of constructor with different signature." << std::endl;
			return;
		}
	}
}

void ZNType_Constructor::PluginSessionAdd(znp__type_source a_availability, const std::list<ZNType_Constructor*> *other_constructors) {
	if(a_availability == zn__tsCharged) {
		for(unsigned int i=0; i<availability.size(); ++i) {
			if(availability[i] == zn__tsCharged) {
				error = true;
				std::cerr << "[Server Error]: Cannot set the constructor avaliability to \"Charged\" because there is already another plugin with same availability." << std::endl;
				a_availability = zn__tsNotAvaliable;
				break;
			}
		}
	}

	if(a_availability == zn__tsCharged && other_constructors) {
		for(std::list<ZNType_Constructor*>::const_iterator a_iter=other_constructors->begin(); a_iter != other_constructors->end(); ++a_iter) {
			for(unsigned int i=0; i < (*a_iter)->availability.size() && i < availability.size(); ++i) {
				if((*a_iter)->availability[i] == zn__tsCharged) {
					error = true;
					std::cerr << "[Server Error]: Cannot set the constructor avaliability to \"Charged\" because there is already another plugin with another constructor with having the same availability." << std::endl;
					a_availability = zn__tsNotAvaliable;
					break;
				}
			}
		}
	}

	availability.push_back(a_availability);
}

void ZNType_Constructor::PluginSessionDelete(unsigned int i) {
	if(i >= availability.size()) {
		std::cerr << "[Server Warning]: Cannot delete a non-existing constructor plugin with index " << i << "." << std::endl;
		return;
	}
	availability[i] = zn__tsNotAvaliable;
}

} /* namespace znode */
} /* namespace jerry */
