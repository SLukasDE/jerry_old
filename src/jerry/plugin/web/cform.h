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

#ifndef C_JERRY_PLUGIN_WEB_FORM_H
#define C_JERRY_PLUGIN_WEB_FORM_H

#include "jerry/plugin/web/cpanel.h"
#include <string>

/**
  *@author Sven Lukas
  */

namespace jerry {
namespace plugin {
namespace web {

class CForm : public CPanel {
public:
	static const znp__scheme* GetScheme();

	CForm(CObjectData &);
	~CForm();

	void Set(const char *a_name, const znp__value &a_value);

//	void CreateDataFormVar(ZSWriter& a_writer);
//	void CreateDataFormAssignment(ZSWriter& a_writer);
//	void CreateDataFormScript(ZSWriter& a_writer);
//	void CreateJavaScript(ZSWriter& a_writer);
	void CreateHtmlBody(ZSWriter& a_writer);

	void SetTitle(const std::string &a_value);
	const std::string& GetTitle() const;

protected:
	signed int GetChildTotalLeft() const;
	signed int GetChildTotalTop() const;

	void RegisterControl(CControl &a_children);
	void RemoveControl(CControl &a_children);

private:
	std::string title;
};

} /* namespace web */
} /* namespace plugin */
} /* namespace jerry */

#endif
