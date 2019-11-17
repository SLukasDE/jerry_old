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

#include "jerry/plugin/calculator/ccontrol.h"
#include "jerry/plugin/calculator/ccustomsession.h"
#include "jerry/plugin/calculator/ccustompanel.h"

namespace jerry {
namespace plugin {
namespace calculator {

const znp__scheme* CControl::GetScheme() {
	static znp__constructor_scheme constructors[] = { { 0, 0, zn__tsOneWay, 0, 0 } };
	static znp__scheme scheme = { "object", 1, constructors };

	return &scheme;
}

CControl::CControl(CObjectData &a_object_data)
: CObject(a_object_data),
  parent(0),
  left(0),
  top(0),
  height(0),
  width(0)
{
	a_object_data.Constructor("control", 0, 0);
}

CControl::~CControl() {
	GetData().Destructor("control");

	if(parent)
		parent->RemoveControl(*this);
}

void CControl::Set(const char *a_name, const znp__value &a_value) {
	if(strcmp(a_name, "parent") == 0 && a_value.v_type == zn__vtPointer) {
		if(a_value.v_pointer && a_value.v_pointer->priv_data)
			SetParent((CCustomPanel*) ((CObjectData*) a_value.v_pointer->priv_data)->GetObject());
		else
			SetParent(0);
	}
	else if(strcmp(a_name, "left") == 0 && a_value.v_type == zn__vtSInt)
		SetLeft(a_value.v_sint);
	else if(strcmp(a_name, "top") == 0 && a_value.v_type == zn__vtSInt)
		SetTop(a_value.v_sint);
	else if(strcmp(a_name, "height") == 0 && a_value.v_type == zn__vtUInt)
		SetHeight(a_value.v_uint);
	else if(strcmp(a_name, "width") == 0 && a_value.v_type == zn__vtUInt)
		SetWidth(a_value.v_uint);
	else
		CObject::Set(a_name, a_value);
}

void CControl::SetParent(CCustomPanel* const &a_parent) {
	CCustomPanel *old_parent;
	znp__value a_value;

	// return imediatly if nothing has changed
	if(parent == a_parent)
		return;

	old_parent = parent;
	if(old_parent)
		old_parent->RemoveControl(*this);

	parent = a_parent;
	if(parent) {
		a_value.v_pointer = &parent->GetData().GetObjectBase();
		a_parent->RegisterControl(*this);
	}
	else {
		a_value.v_pointer = 0;
	}

	a_value.v_type = zn__vtPointer;
	GetData().Set("parent", a_value);
}

CCustomPanel* CControl::GetParent() const {
	return parent;
}

void CControl::SetLeft(const signed int &_value) {
	znp__value a_value;

	left = _value;

	a_value.v_type = zn__vtSInt;
	a_value.v_sint = left;
	GetData().Set("left", a_value);
}

const signed int& CControl::GetLeft() const {
	return left;
}

void CControl::SetTop(const signed int &_value) {
	znp__value a_value;

	top = _value;

	a_value.v_type = zn__vtSInt;
	a_value.v_sint = top;
	GetData().Set("top", a_value);
}

const signed int& CControl::GetTop() const {
	return top;
}

void CControl::SetHeight(const unsigned int &_value) {
	znp__value a_value;

	height = _value;

	a_value.v_type = zn__vtUInt;
	a_value.v_uint = height;
	GetData().Set("height", a_value);
}

const unsigned int& CControl::GetHeight() const {
	return height;
}

void CControl::SetWidth(const unsigned int &_value) {
	znp__value a_value;

	width = _value;

	a_value.v_type = zn__vtUInt;
	a_value.v_uint = width;
	GetData().Set("width", a_value);
}

const unsigned int& CControl::GetWidth() const {
	return width;
}

} /* namespace calculator */
} /* namespace plugin */
} /* namespace jerry */
