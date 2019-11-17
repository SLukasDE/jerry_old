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

#ifndef C_JERRY_PLUGIN_WEB_OBJECT_H
#define C_JERRY_PLUGIN_WEB_OBJECT_H

#include <string>
#include <zsystem/zsstream.h>

#include "jerry/znode/zn_api.h"

namespace jerry {
namespace plugin {
namespace web {

class CSession;
class CObject;

class CObjectData {
friend class CObject;
public:
//	void Constructor(const char *a_type_name, unsigned int a_argc, const znp__value *a_argv);
	void Destructor (const char *a_type_name) const;
	void Set(const char *a_name, const znp__value &_value) const;
	void Call(const char *a_name, znp__value *a_back, unsigned int _argc, const znp__value *_argv) const;

	static void               api_SchemeRequest(znp__session *a_session, const char *a_type);
	static const znp__scheme* api_SchemeGet    (znp__session *a_session, const char *a_type);

	static const znp__object* api_New(znp__session *, const zn__object *a_object, const char *a_type_name);
	static void api_Constructor(const znp__object *a_object, const char *a_type_name, unsigned int a_argc, const znp__value *a_argv);
	static void api_Delete     (const znp__object *a_object);
	static void api_Destructor (const znp__object *a_object, const char *a_type_name);
	static void api_Set        (const znp__object *a_object, const char *a_name, const znp__value *a_value);
	static void api_Call       (const znp__object *a_object, const char *a_name, znp__value *a_back, unsigned int a_argc, const znp__value *a_argv);

	CSession& GetSession() { return session; }
	const CSession& GetSession() const { return session; }
	CObject* GetObject() { return object; }
	const std::string & GetTypeName() const { return name; }
	const znp__object& GetObjectBase() const { return object_base; }
	unsigned int GetID() const { return id; }

private:
	CObjectData(CSession &a_session, const zn__object &a_object, const char *a_object_name);
	~CObjectData();

	CSession &session;
	CObject *object;
	std::string name;
	znp__object object_base;
	unsigned int id;
	bool deleted_inside;
	mutable bool set_inside;
	mutable bool called_inside;
};

class CObject {
public:
	static const znp__scheme* GetScheme();

	virtual ~CObject();

	virtual void Call(const char *a_name, znp__value *a_back, unsigned int a_argc, const znp__value *a_argv) { }
	virtual void Set(const char *a_name, const znp__value &a_value) { }

	void Unmark() { mark = false; }
	bool IsMarked() const { return mark; }
	virtual void Mark() { mark = true; }

	virtual void CreateDataFormVar(ZSWriter& a_writer) { }
	virtual void CreateDataFormAssignment(ZSWriter& a_writer) { }
	virtual void CreateDataFormScript(ZSWriter& a_writer) { }
	virtual void CreateJavaScript(ZSWriter& a_writer) { }
	virtual void CreateHtmlBody(ZSWriter& a_writer) { }

	virtual bool IsWindow() const { return false; }
	const CObjectData& GetData() const { return data; }

/*
	static void api_ImplementationChallange(znp__session *a_plugin_session, const char *a_type_name);
	static void api_ImplementationRequest  (znp__session *a_plugin_session, const char *a_type_name);
	static void api_ImplementationResponse (znp__session *a_plugin_session, const char *a_type_name, zn__implementation *a_implementation);
*/

protected:
	CObject(CObjectData &a_object_data);
	void ServerSet(const char *a_name, const znp__value &a_value) { }

private:
	CObjectData &data;
	bool mark;
};

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */

#endif
