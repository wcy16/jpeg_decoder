#include <iostream>
#include <stdio.h>
#include <conio.h>
#include "main.h"
#include "MyFrame.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
	
	MyFrame *decoder = new MyFrame("JPEG decoder");
	
	decoder->Show(true);

	return true;
}