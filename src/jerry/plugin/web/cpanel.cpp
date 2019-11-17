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

#include "jerry/plugin/web/cpanel.h"
#include "jerry/plugin/web/cplugin.h"

namespace jerry {
namespace plugin {
namespace web {

const znp__scheme* CPanel::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "custompanel", 1, constructors };

	return &scheme;
}

CPanel::CPanel(CObjectData &a_object_data)
: CCustomPanel(a_object_data)
{
}

CPanel::~CPanel() {
}

void CPanel::RegisterControl(CControl &a_children) {
	CCustomPanel::RegisterControl(a_children);
}

void CPanel::RemoveControl(CControl &a_children) {
	CCustomPanel::RemoveControl(a_children);
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
