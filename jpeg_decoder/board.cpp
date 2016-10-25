#include "board.h"
#include "MyFrame.h"
#include <cstdlib>

board::board(wxFrame* parent, int h, int w, jpeg_pic* j_p) :
	wxScrolledWindow(parent)
{
	
	flag = 0;
	j_pic = j_p;

	SetVirtualSize(w, h);
	SetScrollRate(1, 1);

	height = width = j_pic->get_mcu_len();

	Connect(wxEVT_PAINT, wxPaintEventHandler(board::OnPaint));

}

void board::OnPaint(wxPaintEvent& event)
{

	wxPaintDC dc(this);
	DoPrepareDC(dc);
	int x = j_pic->get_pic_w();
	int y = j_pic->get_pic_h();

#if 0
	int cl_wid, cl_hig;
	GetClientSize(&cl_wid, &cl_hig);

	wxCoord x_o, y_o;
	dc.GetDeviceOrigin(&x_o, &y_o);
	x_o = -x_o;
	y_o = -y_o;

	int x_limit, y_limit;

	if (x_o + cl_wid > x)
		x_limit = x;
	else
		x_limit = cl_wid;

	if (y_o + cl_hig > y)
		y_limit = y;
	else
		y_limit = cl_hig;

	
	for (int j = y_o; j < y_limit; j++)
	{
		for (int i = x_o; i < x_limit; i++)
		{
			//j_pic->decode_next_mcu();
			dc.SetPen(wxColor(j_pic->get_pic_r(i, j),
				j_pic->get_pic_g(i, j),
				j_pic->get_pic_b(i, j)));
			dc.DrawPoint(i, j);
		}
	}

#endif

	for (int j = 0; j < y; j++)
	{
		for (int i = 0; i < x; i++)
		{
			//j_pic->decode_next_mcu();
			dc.SetPen(wxColor(j_pic->get_pic_r(i, j),
				j_pic->get_pic_g(i, j),
				j_pic->get_pic_b(i, j)));
			dc.DrawPoint(i, j);
		}
	}
	


}

#if 0
void board::DrawMcu(int x, int y )
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			/*
			dc.SetPen(wxColor(j_pic->get_mcu_r(i * height + j),
				j_pic->get_mcu_g(i * height + j),
				j_pic->get_mcu_b(i * height + j)));
			dc.SetBrush(wxColor(j_pic->get_mcu_r(i * height + j),
				j_pic->get_mcu_g(i * height + j),
				j_pic->get_mcu_b(i * height + j)));

            
			dc.DrawPoint(j + y * height, i + x * height);
			*/
			r_buffer[j + y * height + (i + x * height) * j_pic->get_mcu_w_count() * height]
				= j_pic->get_mcu_r(i * height + j);
			g_buffer[j + y * height + (i + x * height) * j_pic->get_mcu_w_count() * height] 
				= j_pic->get_mcu_g(i * height + j);
			b_buffer[j + y * height + (i + x * height) * j_pic->get_mcu_w_count() * height]
				= j_pic->get_mcu_b(i * height + j);

		}
	}
}
#endif