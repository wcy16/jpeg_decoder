#include <iostream>
#include <stdio.h>
#include <conio.h>
#include<wx/wx.h>
#include "MyFrame.h"

class MyApp : public wxApp
{
public:
	virtual bool OnInit();
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
	
	MyFrame *decoder = new MyFrame("JPEG decoder");
	
	decoder->Show(true);

	return true;
}
