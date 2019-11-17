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

#include "jerry/plugin/calculator/cform.h"
#include "jerry/plugin/calculator/ccustomsession.h"

namespace jerry {
namespace plugin {
namespace calculator {

const znp__scheme* CForm::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "panel", 1, constructors };

	return &scheme;
}

CForm::CForm(CObjectData &a_object_data)
: CPanel(a_object_data.SetName("form"))
{
	a_object_data.Constructor("form", 0, 0);
}

CForm::~CForm() {
	GetData().Destructor("form");
}

void CForm::Set(const char *a_name, const znp__value &a_value) {
	if(strcmp(a_name, "title") == 0 && a_value.v_type == zn__vtString)
		SetTitle(a_value.v_string);
	else
		CPanel::Set(a_name, a_value);
}

void CForm::SetTitle(const std::string &_value) {
	znp__value a_value;

	title = _value;

	a_value.v_type = zn__vtString;
	a_value.v_string = title.c_str();
	GetData().Set("title", a_value);
}

const std::string& CForm::GetTitle() const {
	return title;
}

void CForm::RegisterControl(CControl &a_children) {
	CPanel::RegisterControl(a_children);
}

void CForm::RemoveControl(CControl &a_children) {
	CPanel::RemoveControl(a_children);
}

} /* namespace calculator */
} /* namespace plugin */
} /* namespace jerry */
