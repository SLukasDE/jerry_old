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

#include "jerry/plugin/web/ccustompanel.h"
#include "jerry/plugin/web/cplugin.h"

namespace jerry {
namespace plugin {
namespace web {

const znp__scheme* CCustomPanel::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "control", 1, constructors };

	return &scheme;
}

CCustomPanel::CCustomPanel(CObjectData &a_object_data)
: CControl(a_object_data)
{
}

CCustomPanel::~CCustomPanel() {
	while(!children.empty())
		(*children.begin())->SetParent(0);
}

void CCustomPanel::CreateDataFormVar(ZSWriter& a_writer) {
	for(std::set<CControl*>::iterator iter(children.begin()); iter!=children.end(); ++iter)
		(*iter)->CreateDataFormVar(a_writer);
}

void CCustomPanel::CreateDataFormAssignment(ZSWriter& a_writer) {
	for(std::set<CControl*>::iterator iter(children.begin()); iter!=children.end(); ++iter)
		(*iter)->CreateDataFormAssignment(a_writer);
}

void CCustomPanel::CreateDataFormScript(ZSWriter& a_writer) {
	for(std::set<CControl*>::iterator iter(children.begin()); iter!=children.end(); ++iter)
		(*iter)->CreateDataFormScript(a_writer);
}

void CCustomPanel::CreateJavaScript(ZSWriter& a_writer) {
	for(std::set<CControl*>::iterator iter(children.begin()); iter!=children.end(); ++iter)
		(*iter)->CreateJavaScript(a_writer);
}

void CCustomPanel::CreateHtmlBody(ZSWriter& a_writer) {
	for(std::set<CControl*>::iterator iter(children.begin()); iter!=children.end(); ++iter)
		(*iter)->CreateHtmlBody(a_writer);
}

signed int CCustomPanel::GetChildTotalLeft() const {
	return GetLeft() + ( GetParent() ? GetParent()->GetChildTotalLeft() : 0 );
}

signed int CCustomPanel::GetChildTotalTop() const {
	return GetTop() + ( GetParent() ? GetParent()->GetChildTotalTop() : 0 );
}

void CCustomPanel::RegisterControl(CControl &a_children) {
	children.insert(&a_children);
}

void CCustomPanel::RemoveControl(CControl &a_children) {
	children.erase(&a_children);
}

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */
