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

#ifndef ZNOBJECT_H
#define ZNOBJECT_H

#include <string>
#include <vector>

#include "jerry/znode/zn_api.h"

namespace jerry {
namespace znode {

class ZNSession;
class ZNType;

class ZNObject {
public:
	static void API_Fill(zn__api &a_api);
#ifdef API_DEBUG
	static void Plugin_API_Fill(znp__api &a_plugin_api);
#endif

	void PluginSessionAdd();
	void PluginSessionDelete(unsigned int i);

private:
	ZNObject(ZNSession &a_session, znp__object *a_object, const std::string &a_name);
	~ZNObject();

	void Constructor(const std::string &a_name, unsigned int a_argc, const zn__value *a_argv);
	void Destructor(const std::string &a_type);
	void Call(const std::string &a_name, znp__value *a_back, unsigned int a_argc, const zn__value *a_argv);
	void Set(const std::string &a_name, const zn__value *a_value);

	const std::string& GetTypeName() const { return name; }
	const zn__object& GetObjectBase() const { return object_base; }

	ZNSession &session;
	const ZNType &type;

	// It's better to have the linked list of instances as part of the instance itself for performance reasons.
	// This makes it faster to remove the object from the list if it is deleted.
	// Otherwise a expensive lookup inside of ZNSession would be necessary.
	ZNObject *object_prev;
	ZNObject *object_next;

	std::string name;
	zn__object object_base;
	std::vector<const znp__object*> plugin_objects;

	static void ValueMasterToSlave(const zn__value &master, znp__value &slave, unsigned char slave_id);

	static void api_ObjectNew        (const zn__session *a_session, znp__object *a_object, const char *a_name);
	static void api_ObjectConstructor(const znp__object *a_object, const char *a_name, unsigned int a_argc, const zn__value *a_argv);
	static void api_ObjectDelete     (const znp__object *a_object);
	static void api_ObjectDestructor (const znp__object *a_object, const char *a_name);
	static void api_ObjectCall       (const znp__object *a_object, const char *a_name, znp__value *a_back, unsigned int a_argc, const zn__value *a_argv);
	static void api_ObjectSet        (const znp__object *a_object, const char *a_name, const zn__value *a_value);

#ifdef API_DEBUG
	static void api_DEBUG_ObjectNew        (const zn__session *a_session, znp__object *a_object, const char *a_name);
	static void api_DEBUG_ObjectConstructor(const znp__object *a_object, const char *a_name, unsigned int a_argc, const zn__value *a_argv);
	static void api_DEBUG_ObjectDelete     (const znp__object *a_object);
	static void api_DEBUG_ObjectDestructor (const znp__object *a_object, const char *a_name);
	static void api_DEBUG_ObjectCall       (const znp__object *a_object, const char *a_name, znp__value *a_back, unsigned int a_argc, const zn__value *a_argv);
	static void api_DEBUG_ObjectSet        (const znp__object *a_object, const char *a_name, const zn__value *a_value);


	static void               api_plugin_DEBUG_SchemeRequest(znp__session *a_session, const char *a_type);
	static const znp__scheme* api_plugin_DEBUG_SchemeGet    (znp__session *a_session, const char *a_type);

	static const znp__object* api_plugin_DEBUG_object_new(znp__session *a_session, const zn__object *a_object, const char *type);
	static void api_plugin_DEBUG_object_constructor(const znp__object *a_object, const char *type, unsigned int a_argc, const znp__value *a_argv);
	static void api_plugin_DEBUG_object_delete     (const znp__object *a_object);
	static void api_plugin_DEBUG_object_destructor (const znp__object *a_object, const char *type);
	static void api_plugin_DEBUG_object_call       (const znp__object *a_object, const char *method, znp__value *a_back, unsigned int a_argc, const znp__value *a_argv);
	static void api_plugin_DEBUG_object_set        (const znp__object *a_object, const char *property, const znp__value *a_value);
#endif
};

} /* namespace znode */
} /* namespace jerry */

#endif
