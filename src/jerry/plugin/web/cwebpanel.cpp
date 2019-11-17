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

#include "jerry/plugin/web/cwebpanel.h"
#include "jerry/plugin/web/cplugin.h"

namespace jerry {
namespace plugin {
namespace web {

const znp__scheme* CWebPanel::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "control", 1, constructors };

	return &scheme;
}

CWebPanel::CWebPanel(CObjectData &a_object_data)
: CControl(a_object_data)
{
}

CWebPanel::~CWebPanel() {
}

void CWebPanel::CreateDataFormVar(ZSWriter& a_writer) {
	char a_number_str[33];

	sprintf(a_number_str, "%X", GetData().GetID());
	a_writer << "     C" << a_number_str << "_Suri : null,\n\n";
}

void CWebPanel::CreateDataFormAssignment(ZSWriter& a_writer) {
	char a_number_str[33];

	sprintf(a_number_str, "%X", GetData().GetID());
	a_writer << "          Server.C" << a_number_str << "_Suri=document.getElementById(\"C" << a_number_str << "\").src;\n";
}

void CWebPanel::CreateDataFormScript(ZSWriter& a_writer) {
	char a_number_str[33];

	sprintf(a_number_str, "%X", GetData().GetID());
	a_writer << "          if(typeof(C" << a_number_str << "_Suri)!=\"undefined\")\n";
	a_writer << "              document.writeln(\"<input type='text' value='Server.C" << a_number_str << "_Suri' name='C" << a_number_str << "_Suri'><br>\");\n";
}

void CWebPanel::CreateHtmlBody(ZSWriter& a_writer) {
	char a_number_str[33];
	
	sprintf(a_number_str, "%X", GetData().GetID());
	a_writer << "<iframe id=\"C" << a_number_str << "\" src=\"" << uri << "\"";
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
	a_writer << " overflow:hidden\">\n<p>Ihr Browser kann leider keine eingebetteten Frames anzeigen!\n</iframe>\n";
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

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
