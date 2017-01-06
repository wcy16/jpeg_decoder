#ifndef JPEG_DECODE_H_INCLUDED
#define JPEG_DECODE_H_INCLUDED

#include <string>
#include <deque>
#include "glob_var.h"

class jpeg_pic
{
public:
    jpeg_pic(char* path);

	int pic_info_decode();                 // 0 err & 1 baseline & 2 progressive
	bool decode_info(int i);
	bool decode_info_b();                    // baseline info decode
	bool decode_info_p();                    // progressive info decode

	// baseline decode related function
	void decode_mcu_test();            // test function for mcu decode, basically debug use
	IMG_LEN decode_mcu(int mcu_id, IMG_LEN start_of_mcu);  // decode a mcu
	int get_mcu_len();                      // get pixel count of a mcu's length
	int get_mcu_h_count() { return h_mcu_count; }   // get mcu count of a picture in height
	int get_mcu_w_count() { return w_mcu_count; }   // get mcu count of a picture in width
	int get_pic_h() { return h_size; }       // get pixel count of a picture in height
	int get_pic_w() { return w_size; }       // get pixel count of a picture in width
	void decode_next_mcu();            // decode related, decode next mcu
	unsigned char get_mcu_r(int x);   // get red value of a given position of the mcu in buffer
	unsigned char get_mcu_g(int x);   // get green value of a given position of the mcu in buffer
	unsigned char get_mcu_b(int x);   // get blue value of a given position of the mcu in buffer
	void reset_som();    // reset start of mcu and other parameters related
	void to_rgb();    // adjust sequence and output to rgb

	// get picture's rgb
	unsigned char get_pic_r(int x, int y);
	unsigned char get_pic_g(int x, int y);
	unsigned char get_pic_b(int x, int y);

	std::string get_msg();  // get message from the decoder, mostly error message

	// transfer jpeg file format to other file format
	void to_bmp(std::string file_path);
	

private:
	void errmsg(int i);

    unsigned char *img_buf;
	// unsigned char *data_buf;
    // image info
    IMG_LEN len;        // image size
    IMG_LEN soi = 0;    // start of image
	IMG_LEN dqt[4] = { 0, 0, 0, 0 };    // define quantization table
	int q_table_count = 0;  // indicate the number of dqt
    IMG_LEN sof = 0;    // start of frame
    IMG_LEN dht[100];    // define Huffman table
    int huff_count = 0;   // indicate the number of dht
    IMG_LEN sos = 0;    // start of scan
	IMG_LEN sod = 0;  // start of data
	IMG_LEN sop = 0;  // start of progressive image tag

    color_info c_info[3];
    huff_table table[4];
	quan_table q_table[4];
    int hmax, vmax;

    int mcu_size;
    int h_size, w_size;    // pixel size
    int h_mcu_count, w_mcu_count;
	char mcu_buff[256];    // mcu buff
	int y_diff;
	int cr_diff;
	int cb_diff;
	IMG_LEN som;      // start of mcu

	std::deque<char> binary_buff;  // buff when execute decode_mcu

	unsigned char mcu_r[256];
	unsigned char mcu_g[256];
	unsigned char mcu_b[256];

	unsigned char* r_buffer;
	unsigned char* g_buffer;
	unsigned char* b_buffer;

	std::string msg;

	// progressive decode variables



};



#endif // JPEG_DECODE_H_INCLUDED
