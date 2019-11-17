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

#include "jerry/plugin/web/clabel.h"
#include "jerry/plugin/web/cplugin.h"

namespace jerry {
namespace plugin {
namespace web {

const znp__scheme* CLabel::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "control", 1, constructors };

	return &scheme;
}

CLabel::CLabel(CObjectData &a_object_data)
: CControl(a_object_data)
{
}

CLabel::~CLabel() {
}

void CLabel::CreateDataFormVar(ZSWriter& a_writer) {
	char a_number_str[33];

	sprintf(a_number_str, "%X", GetData().GetID());
	a_writer << "  var C" << a_number_str << "_Ptext\n";
}

void CLabel::CreateHtmlBody(ZSWriter& a_writer) {
	char a_number_str[33];

	a_writer << "<div style=\"";
	a_writer << "position:absolute; top:";
	sprintf(a_number_str, "%d", GetTotalTop());
	a_writer << a_number_str;
	a_writer << "px; left:";
	sprintf(a_number_str, "%d", GetTotalLeft());
	a_writer << a_number_str;
	a_writer << "px;";
	a_writer << " width:";
	sprintf(a_number_str, "%d", GetWidth());
	a_writer << a_number_str;
	a_writer << "px; max-width:";
	a_writer << a_number_str;
	a_writer << "px;";
	a_writer << " height:";
	sprintf(a_number_str, "%d", GetHeight());
	a_writer << a_number_str;
	a_writer << "px; max-height:";
	a_writer << a_number_str;
	a_writer << "px;";
	a_writer << " overflow:hidden\">\n";

	for(const char *ptr = text.c_str(); *ptr != 0; ++ptr) {
		if(*ptr == ' ')
			a_writer << "&nbsp;";
		else
			a_writer << *ptr;
	}

	a_writer << "\n</div>\n";
}

void CLabel::Set(const char *a_name, const znp__value &a_value) {
	if(strcmp(a_name, "text") == 0 && a_value.v_type == zn__vtString)
		SetText(a_value.v_string);
	else
		CControl::Set(a_name, a_value);
}

void CLabel::SetText(const std::string &_value) {
	znp__value a_value;

	text = _value;

	a_value.v_type = zn__vtString;
	a_value.v_string = text.c_str();
	GetData().Set("text", a_value);
}

const std::string& CLabel::GetText() const {
	return text;
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
