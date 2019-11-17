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

#include "jerry/plugin/web/cbutton.h"
#include "jerry/plugin/web/cbuttononclick.h"
#include "jerry/plugin/web/csession.h"
#include "jerry/plugin/web/cplugin.h"

namespace jerry {
namespace plugin {
namespace web {

const znp__scheme* CButton::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "control", 1, constructors };

	return &scheme;
}

CButton::CButton(CObjectData &a_object_data)
: CControl(a_object_data),
  on_click(0)
{
}

CButton::~CButton() {
}

void CButton::Set(const char *a_name, const znp__value &a_value) {
	if(strcmp(a_name, "caption") == 0 && a_value.v_type == zn__vtString)
		SetCaption(a_value.v_string);
	else if(strcmp(a_name, "onclick") == 0 && a_value.v_type == zn__vtPointer) {
		if(a_value.v_pointer && a_value.v_pointer->priv_data)
			SetOnClick((CButtonOnClick*) ((CObjectData*) a_value.v_pointer->priv_data)->GetObject());
		else
			SetOnClick(0);
	}
	else
		CControl::Set(a_name, a_value);
}

void CButton::CreateHtmlBody(ZSWriter& a_writer) {
	char a_number_str[33];

	a_writer << "<button";
	if(on_click != 0) {
		char a_number_str2[33];

		sprintf(a_number_str , "%X", GetData().GetID());
		sprintf(a_number_str2, "%X", on_click->GetData().GetID());
//		a_writer << " onClick=\"slwp_event('C" << a_number_str2 << "','C" << a_number_str << "')\"";
		a_writer << " onClick=\"Server.C" << a_number_str2 << "_MOnClick('" << a_number_str << "')\"";
//		a_writer << " onClick=\"test('C" << a_number_str << "')\"";
	}
	a_writer << " style=\"";
/*
  if(GetBackgroundColor().enable) {
    char red[3];
    char green[3];
    char blue[3];

    sprintf(red,   "%X", GetBackgroundColor().red);
    sprintf(green, "%X", GetBackgroundColor().green);
    sprintf(blue,  "%X", GetBackgroundColor().blue);

    a_html_body += "background-color:#";

    if(red[1] == 0)
      a_html_body += "0";
    a_html_body += red;

    if(green[1] == 0)
      a_html_body += "0";
    a_html_body += green;

    if(blue[1] == 0)
      a_html_body += "0";
    a_html_body += blue;
    a_html_body += "; ";
  }
*/
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
	for(const char *ptr = caption.c_str(); *ptr != 0; ++ptr) {
		if(*ptr == ' ')
			a_writer << "&nbsp;";
		else
			a_writer << *ptr;
	}
	a_writer << "\n</button>\n";
}

void CButton::SetCaption(const std::string &_value) {
	znp__value a_value;

	caption = _value;

	a_value.v_type = zn__vtString;
	a_value.v_string = caption.c_str();
	GetData().Set("caption", a_value);
}

const std::string& CButton::GetCaption() const {
	return caption;
}

void CButton::SetOnClick(CButtonOnClick* const &_value) {
	znp__value a_value;

	// return imediatly if nothing has changed
	if(on_click == _value)
		return;

	on_click = _value;

	a_value.v_type = zn__vtPointer;
	a_value.v_pointer = on_click ? &on_click->GetData().GetObjectBase() : 0;
	GetData().Set("onclick", a_value);
}

CButtonOnClick* CButton::GetOnClick() const {
	return on_click;
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
