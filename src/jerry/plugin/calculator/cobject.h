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

#ifndef SLWP_JERRY_PLUGIN_CALCULATOR_OBJECT_H
#define SLWP_JERRY_PLUGIN_CALCULATOR_OBJECT_H

#include <string>

#include "jerry/znode/zn_api.h"

namespace jerry {
namespace plugin {
namespace calculator {

class CCustomSession;
class CObject;

class CObjectData {
friend class CObject;
friend class CCustomSession;
public:
	static CObjectData& New(CCustomSession &a_session);

	CObjectData& SetName(const char *a_name);
	CObjectData& SetDestructor(const char *a_type);

	void Constructor(const char *a_type_name, unsigned int a_argc, const znp__value *a_argv);
	void Destructor (const char *a_type_name) const;
	void Set(const char *a_name, const znp__value &a_value) const;
	void Call(const char *a_name, znp__value *a_back, unsigned int a_argc, const znp__value *a_argv) const;

	static void               api_SchemeRequest(znp__session *a_session, const char *a_type);
	static const znp__scheme* api_SchemeGet    (znp__session *a_session, const char *a_type);

	static const znp__object* api_New(znp__session *, const zn__object*, const char *a_type_name);
	static void api_Constructor(const znp__object *a_object, const char *a_type_name, unsigned int a_argc, const znp__value *a_argv);
	static void api_Delete     (const znp__object *a_object);
	static void api_Destructor (const znp__object *a_object, const char *a_type_name);
	static void api_Set        (const znp__object *a_object, const char *a_name, const znp__value *a_value);
	static void api_Call       (const znp__object *a_object, const char *a_name, znp__value *a_back, unsigned int a_argc, const znp__value *a_argv);

	CCustomSession& GetSession() { return session; }
	const CCustomSession & GetSession() const { return session; }
	CObject* GetObject() { return object; }
	const std::string& GetTypeName() const { return name; }
	const znp__object& GetObjectBase() const { return object_base; }

private:
	CObjectData(CCustomSession &a_session);
	CObjectData(CCustomSession &a_session, const zn__object &a_object, const char *a_object_name);
	~CObjectData();

	CCustomSession &session;
	CObject *object;
	std::string name;
	std::string destructor;
	znp__object object_base;
	bool created_inside;
	bool deleted_inside;
	mutable bool set_inside;
	mutable bool called_inside;

	static std::map<std::string, znp__scheme*> schemes;
};

class CObject {
public:
	static const znp__scheme* GetScheme();

	virtual ~CObject();

	virtual void Call(const char *a_name, znp__value *a_back, unsigned int a_argc, const znp__value *a_argv)  { }
	virtual void Set(const char *a_name, const znp__value &a_value) { };

protected:
	CObject(CObjectData &a_object_data);
	const CObjectData& GetData() const { return data; }

private:
	CObjectData &data;
};

} /* namespace calculator */
} /* namespace plugin */
} /* namespace jerry */

#endif
