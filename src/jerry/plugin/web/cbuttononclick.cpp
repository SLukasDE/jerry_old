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

#include "jerry/plugin/web/cbuttononclick.h"
#include "jerry/plugin/web/cbutton.h" // damit InsertProperty auch vtPointer als ValueType verwendet
#include "jerry/plugin/web/cplugin.h"
#include <iostream>

namespace jerry {
namespace plugin {
namespace web {

const znp__scheme* CButtonOnClick::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "object", 1, constructors };

	return &scheme;
}

CButtonOnClick::CButtonOnClick(CObjectData &a_object_data)
: CObject(a_object_data)
{
}

CButtonOnClick::~CButtonOnClick() {
}

void CButtonOnClick::CreateDataFormVar(ZSWriter& a_writer) {
	char a_number_str[33];

/*
	sprintf(a_number_str, "%X", GetID());
	a_writer << "  if(call==\"C" << a_number_str << "\"){\n";
	a_writer << "    document.writeln(\"<\"+\"input type=\"+'\"'+\"hidden\"+'\"'+\" value=\"+'\"'+sender+'\"'+\" name=\"+'\"'+call+\"_Pobject\"+'\"'+\">\"+\"<\"+\"br\"+\">\")\n";
	a_writer << "    document.writeln(\"<\"+\"input type=\"+'\"'+\"hidden\"+'\"'+\" value=\"+'\"'+call+'\"'+\" name=\"+'\"'+\"call\"+'\"'+\">\"+\"<\"+\"br\"+\">\")\n";
	a_writer << "  }\n";
*/

	sprintf(a_number_str, "%X", GetData().GetID());
	a_writer << "     C" << a_number_str << "_MOnClick : function(o)\n";
	a_writer << "     {\n";
	a_writer << "          Server.Start();\n";
//	a_writer << "          document.writeln(\"<\"+\"input type=\"+'\"'+\"hidden\"+'\"'+\" value=\"+'\"'+C3_MOnClick+'\"'+\" name=\"+'\"'+\"Method\"+'\"'+\">\"+\"<\"+\"br\"+\">\")\n";
	a_writer << "          document.writeln(\"<input type='text' value='C" << a_number_str << "_OnClick' name='call'><br>\");\n";
//	a_writer << "          document.writeln(\"<\"+\"input type=\"+'\"'+\"hidden\"+'\"'+\" value=\"+'\"'+o.id+'\"'+\" name=\"+'\"'+\"P1\"+'\"'+\">\"+\"<\"+\"br\"+\">\")\n";
	a_writer << "          document.writeln(\"<input type='text' value='\"+o+\"' name='A0_P'><br>\");\n";
	a_writer << "          Server.End();\n";
	a_writer << "     },\n";
	a_writer << "\n";
}

void CButtonOnClick::Call(const char *a_name, znp__value *a_back, unsigned int a_argc, const znp__value *a_argv) {
	if(strcmp(a_name, "OnClick") == 0
	&& a_argc == 1
	&& a_argv[0].v_type == zn__vtPointer) {
		if(a_argv[0].v_pointer && a_argv[0].v_pointer->priv_data)
			OnClick((CButton*) ((CObjectData*) a_argv[0].v_pointer->priv_data)->GetObject());
		else
			OnClick(0);
	}
	else
		CObject::Call(a_name, a_back, a_argc, a_argv);
}

void CButtonOnClick::OnClick(CButton* a_button) {
	znp__value a_tmp;

	a_tmp.v_type = zn__vtPointer;
	a_tmp.v_pointer = a_button ? &a_button->GetData().GetObjectBase() : 0;
	GetData().Call("OnClick", 0, 1, &a_tmp);
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
