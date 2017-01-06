#ifndef GLOB_VAR_H_INCLUDED
#define GLOB_VAR_H_INCLUDED

const char SOI[2] = {char(0xff), char(0xd8)};
const char DQT[2] = {char(0xff), char(0xdb)};
const char DHT[2] = {char(0xff), char(0xc4)};

typedef int IMG_LEN;

const int zig_zag_table[64] = {
	0, 1, 8, 16, 9, 2, 3, 10,
	17, 24, 32, 25, 18, 11, 4, 5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13, 6, 7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

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

struct mcu_buf
{
	int val[64];
};

typedef long LONG;
typedef unsigned long DWORD;
typedef unsigned short WORD;

typedef struct {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BMPFILEHEADER;

typedef struct {
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BMPINFOHEADER;

#endif // GLOB_VAR_H_INCLUDED
