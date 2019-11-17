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

#ifndef C_JERRY_PLUGIN_WEB_BUTTON_H
#define C_JERRY_PLUGIN_WEB_BUTTON_H

#include "jerry/plugin/web/ccontrol.h"
#include <string>

namespace jerry {
namespace plugin {
namespace web {

class CButtonOnClick;

class CButton : public CControl  {
public:
	static const znp__scheme* GetScheme();

	CButton(CObjectData &);
	~CButton();

	void Set(const char *a_name, const znp__value &a_value);

	void CreateHtmlBody(ZSWriter& a_writer);

	void SetCaption(const std::string &a_value);
	const std::string& GetCaption() const;

	void SetOnClick(CButtonOnClick* const &a_value);
	CButtonOnClick* GetOnClick() const;

private:
	std::string caption;
	CButtonOnClick* on_click;
};

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */

#endif
