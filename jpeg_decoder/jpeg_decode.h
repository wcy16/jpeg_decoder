#ifndef JPEG_DECODE_H_INCLUDED
#define JPEG_DECODE_H_INCLUDED

#include <string>
#include <deque>
#include "glob_var.h"

struct huff_table
{
    int length;
    int prop;
    long int codeword[5000];
    long int cw_leng[5000];
    //char codeword[5000][50];
    unsigned char weight[5000];
};

struct color_info
{
    int id;
    int dc_huff_id;
    int ac_huff_id;
    int h_sample_factor;
    int v_sample_factor;
    int q_table_id;
};

struct quan_table
{
	int val[64];
};

class jpeg_pic
{
public:
    jpeg_pic(char* path);

    bool decode_info();
	void decode_mcu_test();
	IMG_LEN decode_mcu(int mcu_id, IMG_LEN start_of_mcu);
	int get_mcu_len();
	int get_mcu_h_count() { return h_mcu_count; }
	int get_mcu_w_count() { return w_mcu_count; }
	int get_pic_h() { return h_size; }
	int get_pic_w() { return w_size; }
	void decode_next_mcu();
	unsigned char get_mcu_r(int x);
	unsigned char get_mcu_g(int x);
	unsigned char get_mcu_b(int x);
	void reset_som();
	void to_rgb();

	unsigned char get_pic_r(int x, int y);
	unsigned char get_pic_g(int x, int y);
	unsigned char get_pic_b(int x, int y);

	std::string get_msg();

	

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
    IMG_LEN dht[4] = {0, 0, 0, 0};    // define Huffman table
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


};



#endif // JPEG_DECODE_H_INCLUDED
