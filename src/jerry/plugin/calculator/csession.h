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

#ifndef C_JERRY_PLUGIN_CALCULATOR_SESSION_H
#define C_JERRY_PLUGIN_CALCULATOR_SESSION_H

#include "jerry/plugin/calculator/ccustomsession.h"
#include "jerry/plugin/calculator/cbuttononclick.h"

namespace jerry {
namespace plugin {
namespace calculator {

class CButton;
class CButtonOnClick;
class CMyButtonOnClick;
class CEdit;
class CWebPanel;
class CPanel;
class CForm;
class CSession;

class CMyButtonOnClick : public CButtonOnClick
{
public:
	CMyButtonOnClick(CSession &a_session);
	void OnClick(CButton* a_button);

private:
	CSession &session;
};


class CSession : public CCustomSession {
friend class CMyButtonOnClick;
public:
	CSession(const zn__session *a_session);

protected:
	~CSession();
	void SessionInit();

private:
	CForm *form1;
	CButton *button_0;
	CButton *button_1;
	CButton *button_2;
	CButton *button_3;
	CButton *button_4;
	CButton *button_5;
	CButton *button_6;
	CButton *button_7;
	CButton *button_8;
	CButton *button_9;
	CButton *button_add;
	CButton *button_sub;
	CButton *button_mul;
	CButton *button_div;
	CButton *button_heise;
	CButtonOnClick *button_on_click1;
	CEdit *edit1;

	CForm *form2;
	CWebPanel *webpanel1;

//	CPanel *panel1;
};

} /* namespace calculator */
} /* namespace plugin */
} /* namespace jerry */

#endif
