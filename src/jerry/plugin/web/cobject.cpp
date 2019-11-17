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

#include "jerry/plugin/web/cobject.h"
#include "jerry/plugin/web/cbuttononclick.h"
#include "jerry/plugin/web/ccontrol.h"
#include "jerry/plugin/web/cbutton.h"
#include "jerry/plugin/web/cedit.h"
#include "jerry/plugin/web/clabel.h"
#include "jerry/plugin/web/cwebpanel.h"
#include "jerry/plugin/web/ccustompanel.h"
#include "jerry/plugin/web/cpanel.h"
#include "jerry/plugin/web/cform.h"
#include "jerry/plugin/web/cplugin.h"
#include "jerry/plugin/web/csession.h"

namespace jerry {
namespace plugin {
namespace web {

static void ValueSlaveToMaster(const znp__value &slave, zn__value &master) {
	master.v_type = slave.v_type;
	switch(slave.v_type) {
	case zn__vtUInt:
		master.v_uint = slave.v_uint;
		break;
	case zn__vtSInt:
		master.v_sint = slave.v_sint;
		break;
	case zn__vtString:
		master.v_string = slave.v_string;
		break;
	case zn__vtBool:
		master.v_bool = slave.v_bool;
		break;
	case zn__vtPointer:
		if(slave.v_pointer)
			master.v_pointer = slave.v_pointer->master;
		else
			master.v_pointer = 0;
		break;
	case zn__vtData:
		master.v_blob.data = slave.v_blob.data;
		master.v_blob.size = slave.v_blob.size;
		break;
	default:
		break;
	}
}

/**
 * This Constructor is called if the object is created outside
 */
CObjectData::CObjectData(CSession &a_session, const zn__object &a_object, const char *a_object_name)
: session(a_session),
  object(0),
  name(a_object_name),
  id(++a_session.object_id_producer),
  deleted_inside(true),
  set_inside(true),
  called_inside(true)
{
	object_base.master    = &a_object;
	object_base.session   = &a_session.plugin_session;
	object_base.priv_data = this;
	session.objects.insert(std::make_pair(id, this));
}

CObjectData::~CObjectData() {
	session.objects.erase(id);
	if(deleted_inside)
		SERVER_API->object_delete(&object_base);
}

void CObjectData::Destructor(const char *a_type) const {
	if(deleted_inside)
		SERVER_API->object_destructor(&object_base, a_type);
}

void CObjectData::Set(const char *a_name, const znp__value &_value) const {
	if(set_inside) {
		zn__value a_value;
		ValueSlaveToMaster(_value, a_value);
		SERVER_API->object_set(&object_base, a_name, &a_value);
	}
}

void CObjectData::Call(const char *a_name, znp__value *a_back, unsigned int _argc, const znp__value *_argv) const {
	if(called_inside) {
		if(_argc == 0)
			SERVER_API->object_call(&object_base, a_name, a_back, 0, 0);
		else {
			zn__value *a_argv = new zn__value[_argc];
			for(unsigned int i=0; i<_argc; ++i)
				ValueSlaveToMaster(_argv[i], a_argv[i]);
			SERVER_API->object_call(&object_base, a_name, a_back, _argc, a_argv);
			delete[] a_argv;
		}
	}
}

void CObjectData::api_SchemeRequest(znp__session *a_session, const char *a_type) {
}

const znp__scheme* CObjectData::api_SchemeGet(znp__session *a_session, const char *a_type) {
	if(strcmp(a_type, "object") == 0)
		return CObject::GetScheme();
	if(strcmp(a_type, "control") == 0)
		return CControl::GetScheme();
	if(strcmp(a_type, "buttononclick") == 0)
		return CButtonOnClick::GetScheme();
	if(strcmp(a_type, "button") == 0)
		return CButton::GetScheme();
	if(strcmp(a_type, "edit") == 0)
		return CEdit::GetScheme();
	if(strcmp(a_type, "label") == 0)
		return CLabel::GetScheme();
	if(strcmp(a_type, "webpanel") == 0)
		return CWebPanel::GetScheme();
	if(strcmp(a_type, "custompanel") == 0)
		return CCustomPanel::GetScheme();
	if(strcmp(a_type, "panel") == 0)
		return CPanel::GetScheme();
	if(strcmp(a_type, "form") == 0)
		return CForm::GetScheme();

	std::cout << "[Web] Scheme for unknown type \"" << a_type << "\" requested." << std::endl;
	return 0;
}

const znp__object* CObjectData::api_New(znp__session *a_plugin_session, const zn__object *a_object, const char *a_name) {
	return &(new CObjectData(*GET_SESSION(a_plugin_session), *a_object, a_name))->object_base;
}

void CObjectData::api_Constructor(const znp__object *a_object, const char *a_name, unsigned int a_argc, const znp__value *a_argv) {
	CObjectData *a_data;

	a_data = (CObjectData*) a_object->priv_data;
	if(a_data == 0)
		return;

	if(a_data->name == "button") {
		if(a_data->name == a_name && a_argc == 0) {
			new CButton(*a_data);
			return;
		}
		std::cout << "[Web]: Object \"" << a_data->name << "\" does not have the requested constructor \"" << a_name << "\"." << std::endl;
		return;
	}

	if(a_data->name == "buttononclick") {
		if(a_data->name == a_name && a_argc == 0) {
			new CButtonOnClick(*a_data);
			return;
		}
		std::cout << "[Web]: Object \"" << a_data->name << "\" does not have the requested constructor \"" << a_name << "\"." << std::endl;
		return;
	}

	if(a_data->name == "edit") {
		if(a_data->name == a_name && a_argc == 0) {
			new CEdit(*a_data);
			return;
		}
		std::cout << "[Web]: Object \"" << a_data->name << "\" does not have the requested constructor \"" << a_name << "\"." << std::endl;
		return;
	}

	if(a_data->name == "label") {
		if(a_data->name == a_name && a_argc == 0) {
			new CLabel(*a_data);
			return;
		}
		std::cout << "[Web]: Object \"" << a_data->name << "\" does not have the requested constructor \"" << a_name << "\"." << std::endl;
		return;
	}

	if(a_data->name == "webpanel") {
		if(a_data->name == a_name && a_argc == 0) {
			new CWebPanel(*a_data);
			return;
		}
		std::cout << "[Web]: Object \"" << a_data->name << "\" does not have the requested constructor \"" << a_name << "\"." << std::endl;
		return;
	}

	if(a_data->name == "panel") {
		if(a_data->name == a_name && a_argc == 0) {
			new CPanel(*a_data);
			return;
		}
		std::cout << "[Web]: Object \"" << a_data->name << "\" does not have the requested constructor \"" << a_name << "\"." << std::endl;
		return;
	}

	if(a_data->name == "form") {
		if(a_data->name == a_name && a_argc == 0) {
			new CForm(*a_data);
			return;
		}
		std::cout << "[Web]: Object \"" << a_data->name << "\" does not have the requested constructor \"" << a_name << "\"." << std::endl;
		return;
	}

	std::cout << "[Web]: Object \"" << a_data->name << "\" is not available" << std::endl;
}

void CObjectData::api_Delete(const znp__object *_object) {
	CObjectData *a_object((CObjectData*) _object->priv_data);

	if(!a_object) {
		std::cerr << "[Web]: ERROR: Object should be deleted but object does not exists" << std::endl;
		return;
	}

	a_object->deleted_inside = false;
	// check if there is still an object reference present. This can happen if this object has no destructor "implemented"
	if(a_object->GetObject()) {
		GET_SESSION(_object->session)->forms->erase((CForm*) a_object->GetObject());
		delete a_object->GetObject();
	}
	delete a_object;
}

void CObjectData::api_Destructor(const znp__object *_object, const char *a_type) {
}

void CObjectData::api_Call(const znp__object *_object, const char *a_name, znp__value *a_back, unsigned int a_argc, const znp__value *a_argv) {
	CObjectData *a_object((CObjectData*) _object->priv_data);

	if(!a_object
	|| !a_object->GetObject())
		return;

	a_object->called_inside = false;
	a_object->GetObject()->Call(a_name, a_back, a_argc, a_argv);
}

void CObjectData::api_Set(const znp__object *_object, const char *a_name, const znp__value *a_value) {
	CObjectData *a_object((CObjectData*) _object->priv_data);

	if(!a_object
	|| !a_object->GetObject())
		return;

	a_object->set_inside = false;
	a_object->GetObject()->Set(a_name, *a_value);
}





















const znp__scheme* CObject::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "", 1, constructors };

	return &scheme;
}

CObject::CObject(CObjectData &a_object_data)
: data(a_object_data)
{
	data.object = this;
}

CObject::~CObject() {
	bool call_delete = data.deleted_inside;

	GetData().Destructor("object");
	data.object = 0;
	if(call_delete)
		delete &data;
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
