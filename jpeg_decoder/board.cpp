#include "board.h"
#include "MyFrame.h"
#include <cstdlib>
#include <wx/dcbuffer.h>

board::board(wxFrame* parent, int h, int w, jpeg_pic* j_p) :
	wxScrolledWindow(parent)
{
	
	flag = 0;
	j_pic = j_p;

	SetVirtualSize(w, h);
	SetScrollRate(1, 1);

	height = width = j_pic->get_mcu_len();
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	paintImg();

	Connect(wxEVT_PAINT, wxPaintEventHandler(board::OnPaint));
	Connect(wxEVT_SCROLLBAR, wxScrollEventHandler(board::OnScroll));
}


void board::OnPaint(wxPaintEvent& event)
{

	wxBufferedPaintDC dc(this);
	dc.SetBackground(*wxTRANSPARENT_BRUSH);
	//wxPaintDC dc(this);
	DoPrepareDC(dc);
	dc.Clear();

	dc.DrawBitmap(*bmap, wxPoint(0, 0));


}

void board::OnScroll(wxScrollEvent & event)
{
	
	//Refresh();
	
}

/*
void board::paintImg()
{
	bmap = new wxBitmap(j_pic->get_pic_w(),j_pic->get_pic_h());
	wxMemoryDC mdc;
	mdc.SelectObject(*bmap);
	int x = j_pic->get_pic_w();
	int y = j_pic->get_pic_h();

	for (int j = 0; j < y; j++)
	{
		for (int i = 0; i < x; i++)
		{
			//j_pic->decode_next_mcu();
			mdc.SetPen(wxColor(j_pic->get_pic_r(i, j),
				j_pic->get_pic_g(i, j),
				j_pic->get_pic_b(i, j)));
			mdc.DrawPoint(i, j);
		}
	}
}
*/

void board::paintImg()
{
	unsigned char *picdata;
	picdata = (unsigned char*)malloc(sizeof(char) * 3 * j_pic->get_pic_h() * j_pic->get_pic_w());
	wxImage img(j_pic->get_pic_w(), j_pic->get_pic_h(), picdata);
	j_pic->get_rgb(picdata);

	bmap = new wxBitmap(img);
}


