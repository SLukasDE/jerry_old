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

#include "jerry/plugin/calculator/csession.h"
#include "jerry/plugin/calculator/cbutton.h"
#include "jerry/plugin/calculator/cedit.h"
#include "jerry/plugin/calculator/cwebpanel.h"
#include "jerry/plugin/calculator/cpanel.h"
#include <iostream>
#include <string.h>

namespace jerry {
namespace plugin {
namespace calculator {

znp__session* CCustomSession::api_Create(znp__api *a_plugin_api, const zn__session *_session, const char *a_session_name) {
	CCustomSession *a_session;

	if(strcmp(a_session_name, "calc1") != 0) {
		std::cout << "The Calculator-Plugin is only designed to answer only for applications with the name \"calc1\", but there is an application called with name \"" << a_session_name << "\", so we return a NULL-Pointer for znp_session." << std::endl;
		std::cout << "This is a valid return value and a good test-case for the web-presenter" << std::endl;
		return 0;
	}

	a_session = new CSession(_session);
	return (znp__session*) &a_session->plugin_session;
}


CMyButtonOnClick::CMyButtonOnClick(CSession &a_session)
: CButtonOnClick(CObjectData::New(a_session)),
  session(a_session)
{
}

void CMyButtonOnClick::OnClick(CButton* a_button) {
	if(a_button == session.button_heise) {
		std::cout << "[Calc]: Heise-Button" << std::endl;
		if(session.webpanel1)
			session.webpanel1->SetURI("http://www.heise.de");
	}
	else if(a_button == session.button_1) {
		std::cout << "[Calc]: 1-Button" << std::endl;
		if(session.webpanel1) {
			if(session.webpanel1->GetParent())
				delete session.webpanel1->GetParent();
			delete session.webpanel1;
		}
		session.webpanel1 = 0;
	}
	else
		std::cout << "[Calc]: irgendein Button: " << std::endl;
}

CSession::CSession(const zn__session *a_session)
: CCustomSession(a_session)
{
}

CSession::~CSession() {
}


void CSession::SessionInit() {
	GetScreen().SetTitle("Test-App");

	form1 = new CForm(CObjectData::New(*this));
	form1->SetLeft(5);
	form1->SetTop(5);
	form1->SetHeight(225);
	form1->SetWidth(165);
	form1->SetParent(&GetScreen());
	form1->SetTitle("Calculator");

	button_on_click1 = new CMyButtonOnClick(*this);

	edit1 = new CEdit(CObjectData::New(*this));
	edit1->SetLeft(5);
	edit1->SetTop(0);
	edit1->SetHeight(20);
	edit1->SetWidth(155);
	edit1->SetText("Edit1");
	edit1->SetParent(form1);

	button_7 = new CButton(CObjectData::New(*this));
	button_7->SetLeft(5);
	button_7->SetTop(30);
	button_7->SetHeight(30);
	button_7->SetWidth(30);
	button_7->SetCaption("7");
	button_7->SetParent(form1);
	button_7->SetOnClick(button_on_click1);

	button_8 = new CButton(CObjectData::New(*this));
	button_8->SetLeft(45);
	button_8->SetTop(30);
	button_8->SetHeight(30);
	button_8->SetWidth(30);
	button_8->SetCaption("8");
	button_8->SetParent(form1);
	button_8->SetOnClick(button_on_click1);

	button_9 = new CButton(CObjectData::New(*this));
	button_9->SetLeft(85);
	button_9->SetTop(30);
	button_9->SetHeight(30);
	button_9->SetWidth(30);
	button_9->SetCaption("9");
	button_9->SetParent(form1);
	button_9->SetOnClick(button_on_click1);

	button_4 = new CButton(CObjectData::New(*this));
	button_4->SetLeft(5);
	button_4->SetTop(70);
	button_4->SetHeight(30);
	button_4->SetWidth(30);
	button_4->SetCaption("4");
	button_4->SetParent(form1);
	button_4->SetOnClick(button_on_click1);

	button_5 = new CButton(CObjectData::New(*this));
	button_5->SetLeft(45);
	button_5->SetTop(70);
	button_5->SetHeight(30);
	button_5->SetWidth(30);
	button_5->SetCaption("5");
	button_5->SetParent(form1);
	button_5->SetOnClick(button_on_click1);

	button_6 = new CButton(CObjectData::New(*this));
	button_6->SetLeft(85);
	button_6->SetTop(70);
	button_6->SetHeight(30);
	button_6->SetWidth(30);
	button_6->SetCaption("6");
	button_6->SetParent(form1);
	button_6->SetOnClick(button_on_click1);

	button_1 = new CButton(CObjectData::New(*this));
	button_1->SetLeft(5);
	button_1->SetTop(110);
	button_1->SetHeight(30);
	button_1->SetWidth(30);
	button_1->SetCaption("1");
	button_1->SetParent(form1);
	button_1->SetOnClick(button_on_click1);

	button_2 = new CButton(CObjectData::New(*this));
	button_2->SetLeft(45);
	button_2->SetTop(110);
	button_2->SetHeight(30);
	button_2->SetWidth(30);
	button_2->SetCaption("2");
	button_2->SetParent(form1);
	button_2->SetOnClick(button_on_click1);

	button_3 = new CButton(CObjectData::New(*this));
	button_3->SetLeft(85);
	button_3->SetTop(110);
	button_3->SetHeight(30);
	button_3->SetWidth(30);
	button_3->SetCaption("3");
	button_3->SetParent(form1);
	button_3->SetOnClick(button_on_click1);

	button_0 = new CButton(CObjectData::New(*this));
	button_0->SetLeft(45);
	button_0->SetTop(150);
	button_0->SetHeight(30);
	button_0->SetWidth(30);
	button_0->SetCaption("0");
	button_0->SetParent(form1);
	button_0->SetOnClick(button_on_click1);

	button_add = new CButton(CObjectData::New(*this));
	button_add->SetLeft(130);
	button_add->SetTop(30);
	button_add->SetHeight(30);
	button_add->SetWidth(30);
	button_add->SetCaption("+");
	button_add->SetParent(form1);
	button_add->SetOnClick(button_on_click1);

	button_sub = new CButton(CObjectData::New(*this));
	button_sub->SetLeft(130);
	button_sub->SetTop(70);
	button_sub->SetHeight(30);
	button_sub->SetWidth(30);
	button_sub->SetCaption("-");
	button_sub->SetParent(form1);
	button_sub->SetOnClick(button_on_click1);

	button_mul = new CButton(CObjectData::New(*this));
	button_mul->SetLeft(130);
	button_mul->SetTop(110);
	button_mul->SetHeight(30);
	button_mul->SetWidth(30);
	button_mul->SetCaption("*");
	button_mul->SetParent(form1);
	button_mul->SetOnClick(button_on_click1);

	button_div = new CButton(CObjectData::New(*this));
	button_div->SetLeft(130);
	button_div->SetTop(150);
	button_div->SetHeight(30);
	button_div->SetWidth(30);
	button_div->SetCaption("/");
	button_div->SetParent(form1);
	//  button_div->SetOnClick(button_on_click1);
	button_div->SetOnClick(0);

	button_heise = new CButton(CObjectData::New(*this));
	button_heise->SetLeft(5);
	button_heise->SetTop(190);
	button_heise->SetHeight(30);
	button_heise->SetWidth(155);
	button_heise->SetCaption("Heise Newsticker 2");
	button_heise->SetParent(form1);
	button_heise->SetOnClick(button_on_click1);



/*
	button_test = new CButton(*this, 0);
	button_test->SetLeft(0);
	button_test->SetTop(0);
	button_test->SetHeight(30);
	button_test->SetWidth(300);
	button_test->SetCaption("test");
	button_test->SetParent(form1);
	button_test->SetOnClick(button_on_click1);

	panel1 = new CPanel(*this, 0);
	panel1->SetLeft(5);
	panel1->SetTop(5);
	panel1->SetHeight(480);
	panel1->SetWidth(640);
	panel1->SetParent(&GetScreen());
*/

	form2 = new CForm(CObjectData::New(*this));
	form2->SetLeft(175);
	form2->SetTop(5);
	form2->SetHeight(500);
	form2->SetWidth(800);
	form2->SetParent(&GetScreen());
	form2->SetTitle("Browser");

	webpanel1 = new CWebPanel(CObjectData::New(*this));
	webpanel1->SetLeft(0);
	webpanel1->SetTop(0);
	webpanel1->SetHeight(500);
	webpanel1->SetWidth(800);
	webpanel1->SetParent(form2);
	webpanel1->SetURI("http://wischmop-default");
}

} /* namespace calculator */
} /* namespace plugin */
} /* namespace jerry */
