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

#include "jerry/plugin/calculator/cwebpanel.h"
#include "jerry/plugin/calculator/ccustomsession.h"

namespace jerry {
namespace plugin {
namespace calculator {

const znp__scheme* CWebPanel::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "control", 1, constructors };

	return &scheme;
}

CWebPanel::CWebPanel(CObjectData &a_object_data)
: CControl(a_object_data.SetName("webpanel"))
{
	a_object_data.Constructor("webpanel", 0, 0);
}

CWebPanel::~CWebPanel() {
	GetData().Destructor("webpanel");
}

void CWebPanel::Set(const char *a_name, const znp__value &a_value) {
	if(strcmp(a_name, "uri") == 0 && a_value.v_type == zn__vtString)
		SetURI(a_value.v_string);
	else
		CControl::Set(a_name, a_value);
}

void CWebPanel::SetURI(const std::string &_value) {
	znp__value a_value;

	uri = _value;

	a_value.v_type = zn__vtString;
	a_value.v_string = uri.c_str();
	GetData().Set("uri", a_value);
}

const std::string& CWebPanel::GetURI() const {
	return uri;
}

} /* namespace calculator */
} /* namespace plugin */
} /* namespace jerry */
