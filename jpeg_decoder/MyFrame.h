#ifndef MYFRAME_H_INCLUDED
#define MYFRAME_H_INCLUDED

#include <wx/wx.h>
#include "board.h"
#include "jpeg_decode.h"

class MyFrame : public wxFrame
{
public:
	MyFrame(const wxString& title);
	void OnQuit();
	jpeg_pic* j_pic;

	board *boardpanel;
};


#endif // MYFRAME_H_INCLUDE