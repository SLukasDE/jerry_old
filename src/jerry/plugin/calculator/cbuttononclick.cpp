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

#include "jerry/plugin/calculator/cbuttononclick.h"
#include "jerry/plugin/calculator/cbutton.h"
#include "jerry/plugin/calculator/ccustomsession.h"

namespace jerry {
namespace plugin {
namespace calculator {

const znp__scheme* CButtonOnClick::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "object", 1, constructors };

	return &scheme;
}

CButtonOnClick::CButtonOnClick(CObjectData &a_object_data)
: CObject(a_object_data.SetName("buttononclick"))
{
	a_object_data.Constructor("buttononclick", 0, 0);
}

CButtonOnClick::~CButtonOnClick() {
	GetData().Destructor("buttononclick");
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

} /* namespace calculator */
} /* namespace plugin */
} /* namespace jerry */
