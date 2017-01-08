#include <string>
#include "MyFrame.h"


MyFrame::MyFrame(const wxString& title) :
	wxFrame(NULL, -1, title, wxDefaultPosition, wxSize(500, 500))
{
	// wxScrolledWindow* sw = new wxScrolledWindow(this);
	
	wxFileDialog* openJPEG = new wxFileDialog(this);
	wxString fileName;
	if (openJPEG->ShowModal() == wxID_OK)
	{
		fileName = openJPEG->GetPath();

	}
	if (fileName.IsEmpty() == 0) {

		j_pic = new jpeg_pic(fileName.char_str());
		int info = j_pic->pic_info_decode();
		if (j_pic->decode_info(info))
		{
			std::string msg = j_pic->get_msg();
			wxMessageDialog * dial = new wxMessageDialog(NULL, msg, _T("Error"), wxOK | wxICON_ERROR);
			dial->ShowModal();
			Close();
		}
		else
		{
			j_pic->to_rgb();
			int h = j_pic->get_pic_h();
			int w = j_pic->get_pic_w();
#if 0
			int cl_h, cl_w;
			if (h > 500)
				cl_h = 500;
			else
				cl_h = h;
			if (w > 700)
				cl_w = 700;
			else
				cl_w = w;
			SetSize(cl_w, cl_h);
#endif
			//j_pic->to_bmp("btest.bmp");
			boardpanel = new board(this, h, w, j_pic);
		}
		
	}
}

void MyFrame::OnQuit()
{
	Close();
}