#include <iostream>
#include <stdio.h>
#include <conio.h>
#include "../jpeg_decoder/jpeg_decode.h"

int main(int argc, char *argv[])
{
	std::string file_path;
	if (argc == 1)
	{
		std::cout << "please enter the file path: ";
		std::cin >> file_path;
	}
	else if (argc == 2)
	{
		file_path = argv[1];
	}

	std::cout << "\ndecode JPEG from " << file_path << std::endl;
	jpeg_pic* j_pic;
	j_pic = new jpeg_pic(file_path.c_str());
	int info = j_pic->pic_info_decode();
	if (j_pic->decode_info(info))
	{
		std::string msg = j_pic->get_msg();
		std::cout << "Error occurs: " << msg << std::endl;
	}
	else
	{
		j_pic->to_rgb();
		j_pic->to_bmp(file_path + ".bmp");
		std::cout << "Output a BMP file to " << file_path + ".bmp" << std::endl;
	}
	std::cout << "press any key to exit...";
	getch();
	return 0;

}