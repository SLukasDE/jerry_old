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

#ifndef C_JERRY_PLUGIN_CALCULATOR_CONTROL_H
#define C_JERRY_PLUGIN_CALCULATOR_CONTROL_H

#include "jerry/plugin/calculator/cobject.h"

namespace jerry {
namespace plugin {
namespace calculator {

class CCustomPanel;

class CControl : public CObject  {
public: 
	static const znp__scheme* GetScheme();

	~CControl();

	void Set(const char *a_name, const znp__value &a_value);

	void SetParent(CCustomPanel* const &a_parent);
	CCustomPanel* GetParent() const;

	void SetLeft(const signed int &a_value);
	const signed int& GetLeft() const;

	void SetTop(const signed int &a_value);
	const signed int& GetTop() const;

	void SetHeight(const unsigned int &a_value);
	const unsigned int& GetHeight() const;

	void SetWidth(const unsigned int &a_value);
	const unsigned int& GetWidth() const;

protected:
	CControl(CObjectData &a_object_data);

private:
	CCustomPanel *parent;
	signed int left;
	signed int top;
	unsigned int height;
	unsigned int width;
};

} /* namespace calculator */
} /* namespace plugin */
} /* namespace jerry */

#endif
