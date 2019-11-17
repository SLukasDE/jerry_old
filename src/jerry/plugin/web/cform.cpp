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

#include "jerry/plugin/web/cform.h"
#include "jerry/plugin/web/cplugin.h"
#include "jerry/plugin/web/csession.h"

namespace jerry {
namespace plugin {
namespace web {

const znp__scheme* CForm::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "panel", 1, constructors };

	return &scheme;
}

CForm::CForm(CObjectData &a_object_data)
: CPanel(a_object_data)
{
	GetData().GetSession().forms->insert(this);
}

CForm::~CForm() {
}

void CForm::Set(const char *a_name, const znp__value &a_value) {
	if(strcmp(a_name, "title") == 0 && a_value.v_type == zn__vtString)
		SetTitle(a_value.v_string);
	else
		CPanel::Set(a_name, a_value);
}

void CForm::CreateHtmlBody(ZSWriter& a_writer) {
	if( !GetParent() ) {
		CPanel::CreateHtmlBody(a_writer);
		return;
	}

	char a_number_str[33];

	sprintf(a_number_str, "%X", GetData().GetID());

// FIXME
if(title == "Browser")
	a_writer << "<div id=\"C" << a_number_str << "\" style=\"visibility:visible; z-index:2;position:absolute;top:";
else
	a_writer << "<div id=\"C" << a_number_str << "\" style=\"visibility:visible; z-index:1;position:absolute;top:";

	sprintf(a_number_str, "%d", GetTotalTop());
	a_writer << a_number_str;
	a_writer << "px; left:";
	sprintf(a_number_str, "%d", GetTotalLeft());
	a_writer << a_number_str;
	a_writer << "px;";
	a_writer << " width:";
	sprintf(a_number_str, "%d", GetWidth()+8);
	a_writer << a_number_str;
	a_writer << "px; max-width:";
	a_writer << a_number_str;
	a_writer << "px;";
	a_writer << " height:";
	sprintf(a_number_str, "%d", GetHeight()+34);
	a_writer << a_number_str;
	a_writer << "px; max-height:";
	a_writer << a_number_str;
	a_writer << "px;";
	a_writer << " overflow:hidden\">\n";

	sprintf(a_number_str, "%d", GetWidth()+8);
	a_writer << "<table cellpadding=\"0\" cellspacing=\"0\" border=\"0\" width=\"" << a_number_str << "\">\n";
	sprintf(a_number_str, "%X", GetData().GetID());
	a_writer << "    <tr id=\"C" << a_number_str << "Bar\">\n";
	a_writer << "        <td width=\"4\"><img src=\"http://" << GetData().GetSession().GetBaseURI() << "/p1.gif\" width=\"4\" height=\"30\" alt=\"\" border=\"0\"></td>\n";
	a_writer << "        <td background=\"http://" << GetData().GetSession().GetBaseURI() << "/tbg.gif\">";
	for(const char *ptr = title.c_str(); *ptr != 0; ++ptr) {
		if(*ptr == ' ')
			a_writer << "&nbsp;";
		else
			a_writer << *ptr;
	}
	a_writer << "</td>\n";
	a_writer << "        <td background=\"http://" << GetData().GetSession().GetBaseURI() << "/tbg.gif\" align=\"right\"><a href=\"#\"";
#if 0
	if(on_click.v_pointer != 0) {
		char a_number_str2[33];

		sprintf(a_number_str , "%X", GetID());
		sprintf(a_number_str2, "%X", on_click.v_pointer);
		a_writer << " onClick=\"slwp_event('C" << a_number_str2 << "','C" << a_number_str << "')\"";
	}
#endif
	a_writer << "><img src=\"http://" << GetData().GetSession().GetBaseURI() << "/close.gif\" width=\"24\" height=\"30\" alt=\"\" border=\"0\"></a></td>\n";
	a_writer << "        <td width=\"4\"><img src=\"http://" << GetData().GetSession().GetBaseURI() << "/p2.gif\" width=\"4\" height=\"30\" alt=\"\" border=\"0\"></td>\n";
	a_writer << "    </tr>\n";
	a_writer << "    <tr>\n";
	sprintf(a_number_str, "%d", GetHeight());
	a_writer << "        <td><img src=\"http://" << GetData().GetSession().GetBaseURI() << "/left.gif\" width=\"4\" height=\"" << a_number_str << "\" alt=\"\" border=\"0\"></td>\n";
	a_writer << "        <td colspan=\"2\" align=\"center\" valign=\"top\">\n";

//	a_writer << "            test\n";
	CPanel::CreateHtmlBody(a_writer);

	a_writer << "        </td>\n";
	a_writer << "        <td><img src=\"http://" << GetData().GetSession().GetBaseURI() << "/right.gif\" width=\"4\" height=\"" << a_number_str << "\" alt=\"\" border=\"0\"></td>\n";
	a_writer << "    </tr>\n";
	a_writer << "    <tr>\n";
	a_writer << "        <td><img src=\"http://" << GetData().GetSession().GetBaseURI() << "/cbl.gif\" width=\"4\" height=\"4\" alt=\"\" border=\"0\"></td>\n";
	a_writer << "        <td colspan=\"2\"><img src=\"http://" << GetData().GetSession().GetBaseURI() << "/bot.gif\" width=\"100%\" height=\"4\" alt=\"\" border=\"0\"></td>\n";
	a_writer << "        <td><img src=\"http://" << GetData().GetSession().GetBaseURI() << "/cbr.gif\" width=\"4\" height=\"4\" alt=\"\" border=\"0\"></td>\n";
	a_writer << "    </tr>\n";
	a_writer << "</table>\n";
	a_writer << "</div>\n";


	sprintf(a_number_str, "%X", GetData().GetID());
	a_writer << "<SCRIPT LANGUAGE=\"JavaScript\">\n";
	a_writer << "<!-- Begin\n";
	a_writer << "ZList.Init(document.getElementById(\"C" << a_number_str << "\"));\n";
//	a_writer << "Drag.init(document.getElementById(\"C" << a_number_str << "Bar\"), document.getElementById(\"C" << a_number_str << "\"),100,100,800,300);\n";
	a_writer << "Drag.init(document.getElementById(\"C" << a_number_str << "Bar\"), document.getElementById(\"C" << a_number_str << "\"),null,null,null,null);\n";
	a_writer << "//-->\n";
	a_writer << "</script>\n";








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

signed int CForm::GetChildTotalLeft() const {
	return 4;
}

signed int CForm::GetChildTotalTop() const {
	return 30;
}

void CForm::RegisterControl(CControl &a_children) {
	CPanel::RegisterControl(a_children);
}

void CForm::RemoveControl(CControl &a_children) {
	CPanel::RemoveControl(a_children);
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
