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

#include "jerry/plugin/web/cedit.h"
#include "jerry/plugin/web/cplugin.h"

namespace jerry {
namespace plugin {
namespace web {

const znp__scheme* CEdit::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "control", 1, constructors };

	return &scheme;
}

CEdit::CEdit(CObjectData &a_object_data)
: CControl(a_object_data)
{
}

CEdit::~CEdit() {
}

void CEdit::Set(const char *a_name, const znp__value &a_value) {
	if(strcmp(a_name, "text") == 0 && a_value.v_type == zn__vtString) {
		SetText(a_value.v_string);
	}
	else {
		CControl::Set(a_name, a_value);
	}
}

void CEdit::CreateDataFormVar(ZSWriter& a_writer) {
	char a_number_str[33];

	sprintf(a_number_str, "%X", GetData().GetID());
	a_writer << "     C" << a_number_str << "_Stext : null,\n\n";
}

void CEdit::CreateDataFormAssignment(ZSWriter& a_writer) {
	char a_number_str[33];

	sprintf(a_number_str, "%X", GetData().GetID());
//  a_java_script += "  C" + a_str_stream.str() + "_Ptext=document.C" + a_str_stream.str() + ".value\n";
//	a_writer << "  C" << a_number_str << "_Stext=document.getElementsByName(\"C" << a_number_str << "\")[0].value\n";
	a_writer << "          Server.C" << a_number_str << "_Stext=document.getElementById(\"C" << a_number_str << "\").value;\n";
}

void CEdit::CreateDataFormScript(ZSWriter& a_writer) {
	char a_number_str[33];

	sprintf(a_number_str, "%X", GetData().GetID());
//  a_java_script += "  document.writeln(\"<\"+\"INPUT TYPE=\"+'\"'+\"TEXT\"+'\"'+\" VALUE=\"+'\"'+\"C" + a_str_stream.str() + "_Ptext\"+'\"'+\" NAME=\"+'\"'+\"C" + a_str_stream.str() + "_Ptext\"+'\"'+\">\"+\"<\"+\"BR\"+\">\")\n";
	a_writer << "          document.writeln(\"<input type=\"+'\"'+\"hidden\"+'\"'+\" value=\"+'\"'+Server.C" << a_number_str << "_Stext+'\"'+\" name=\"+'\"'+\"C" << a_number_str << "_Stext\"+'\"'+\"><br>\");\n";
}

//void CEdit::CreateJavaScript(ZSWriter& a_writer) {
//}

void CEdit::CreateHtmlBody(ZSWriter& a_writer) {
	char a_number_str[33];

	a_writer << "<form style=\"";
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

	sprintf(a_number_str, "%X", GetData().GetID());
	a_writer << "<input type=\"text\" id=\"C" << a_number_str << "\" value=\"";
//	a_writer << "<input type=\"text\" name=\"C" << a_str_stream1.str() << "\" value=\"";
	for(const char *ptr = text.c_str(); *ptr != 0; ++ptr) {
		if(*ptr == ' ')
			a_writer << "&nbsp;";
		else
			a_writer << *ptr;
	}
	a_writer << "\" style=\"";
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
	a_writer << "position:absolute; top:0px; left:0px; width:";

/*
  a_html_body += "position:absolute; top:";
  sprintf(a_number_str, "%d", top);
  a_html_body += a_number_str;
  a_html_body += "px; left:";
  sprintf(a_number_str, "%d", left);
  a_html_body += a_number_str;
  a_html_body += "px;";
  a_html_body += " width:";
*/
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
	a_writer << " overflow:hidden\">\n</form>\n";
}

void CEdit::SetText(const std::string &a_value) {
	text = a_value;
}

const std::string& CEdit::GetText() const {
	return text;
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
