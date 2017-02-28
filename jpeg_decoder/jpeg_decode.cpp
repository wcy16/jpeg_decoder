/*
 * debug macro include
 *    JPEG_DECODE_DEBUG
 *    DEBUG_1
 *    DEBUG_2
 *    DEBUG_rgb
 *    PROGRESSIVE_DEBUG
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jpeg_decode.h"
#include "inv_dct.h"


//#define PROGRESSIVE_DEBUG

jpeg_pic::jpeg_pic(const char* path)
{
	// init var
	y_diff = 0;
	cr_diff = 0;
	cb_diff = 0;
	memset(dht, 0, sizeof(dht));

	FILE *pf;    // image file

				 // read image file
	pf = fopen(path, "rb");

	fseek(pf, 0, SEEK_END);
	len = ftell(pf);
	img_buf = new unsigned char[len + 1];
	rewind(pf);
	fread(img_buf, 1, len, pf);


	img_buf[len] = 0;
	fclose(pf);

}

int jpeg_pic::pic_info_decode()
{
	for (IMG_LEN i = 0; i < len; i++)
	{
		if (img_buf[i] == 0xff) {
			switch (img_buf[i + 1])
			{
			case 0xc0: return 1; break;
			case 0xc2: return 2; break;
			default:  break;
			}
		}
	}
	return 0;
}

bool jpeg_pic::decode_info(int i)
{
	bool b = 1;
	switch (i)
	{
	case 0: b = 1; break;
	case 1: b = decode_info_b(); break;
	case 2: b = decode_info_p(); break;
	default: b = 1;  break;
	}
	return b;
}

bool jpeg_pic::decode_info_b()
{


	int q_table_temp = 0;
	int huff_temp = 0;
	for (IMG_LEN i = 0; i < len; i++)
	{
		if (img_buf[i] == 0xff) {
			switch (img_buf[i + 1])
			{
			case 0xd8: soi = i + 2; break;
			case 0xdb:
				dqt[q_table_temp] = i + 2;
				q_table_temp++;
				break;
			case 0xc0: sof = i + 2; break;
			case 0xc2: sop = i + 2; break;
			case 0xc4:
				dht[huff_temp] = i + 2;
				huff_temp++;
				break;
			case 0xda: sos = i + 2; break;
				// other cases to be added
			default: break;
			}

		}
	}

#ifdef JPEG_DECODE_DEBUG
	printf("soi found in %d\n", soi);
	printf("%d dht found in", huff_count);
	for (int i = 0; i < huff_count; i++)
		printf(" %d", dht[i]);
	putchar('\n');
	printf("sos found in %d\n", sos);
#endif // JPEG_DECODE_DEBUG

	// consider open a wrong file
	if (soi == 0 || q_table_temp == 0 || sof == 0 || huff_temp == 0 || sos == 0)
	{
		errmsg(1);
		return 1;
	}

	// program do not support progressive jpeg decode
	if (sop != 0)
	{
		errmsg(0);
		return 1;
	}

	// decode start of frame info
	h_size = img_buf[sof + 3] * 256 + img_buf[sof + 4];
	w_size = img_buf[sof + 5] * 256 + img_buf[sof + 6];
	for (int i = 0; i < 3; i++)
	{
		c_info[i].id = img_buf[sof + 8 + i * 3];
		c_info[i].h_sample_factor = img_buf[sof + 9 + i * 3] >> 4;
		c_info[i].v_sample_factor = img_buf[sof + 9 + i * 3] & 0x0f;
		c_info[i].q_table_id = img_buf[sof + 10 + i * 3];
	}
#ifdef JPEG_DECODE_DEBUG
	printf("height: %d  width: %d\n", h_size, w_size);
#endif // JPEG_DECODE_DEBUG

	// decode quantization table
	q_table_count = 0;
	for (int i = 0; i < q_table_temp; i++)
	{
		int dqt_len = int(img_buf[dqt[i]] * 256 + img_buf[dqt[i] + 1]) - 2;
		int p = dqt[i] + 2;
		while (dqt_len > 0)
		{
			int q_info = img_buf[p];
			int q_a = (q_info >> 4) + 1;   // accuracy
			int q_id = q_info && 0x0f;  // table id
			for (int j = 0; j < 64; j++)
			{
				q_table[q_id].val[j] = img_buf[p + 1 + j * q_a]
					+ (q_a - 1)*img_buf[p + 1 + j * q_a + 1];
			}
			dqt_len = dqt_len - 64 * q_a - 1;
			p = p + 64 * q_a + 1;
			q_table_count++;
		}

	}

#ifdef JPEG_DECODE_DEBUG
	puts("quantization table:\n");
	for (int i = 0; i < q_table_count; i++)
	{
		for (int j = 0; j < 8; j++)
		{

			for (int k = 0; k < 8; k++)
				printf("%d ", q_table[i].val[j]);
			printf("\n");
		}
		printf("\n");

	}
#endif // JPEG_DECODE_DEBUG

	// decode Huffman table
	int last_element;
	int last_leng;
	int codeword_count;
	int weight_pos;

	huff_count = 0;
	for (int i = 0; i < huff_temp; i++)
	{
		IMG_LEN p = dht[i];
		int tlen = int(img_buf[p] * 256 + img_buf[p + 1]) - 2;

		p += 2;

		while (tlen > 0)
		{
			table[huff_count].prop = img_buf[p];
			table[huff_count].length = 0;
			last_element = -1;
			last_leng = 1;
			codeword_count = 0;
			weight_pos = 16;
			p += 1;

			for (int j = 0; j < 16; j++)
			{
				if (img_buf[p + j] == 0x00)
					continue;
				for (int k = 0; k < img_buf[p + j]; k++)
				{
					table[huff_count].cw_leng[codeword_count] = j + 1;
					table[huff_count].codeword[codeword_count] =
						(last_element + 1) << (j + 1 - last_leng);
					table[huff_count].weight[codeword_count] = img_buf[p + weight_pos];
					last_element = table[huff_count].codeword[codeword_count];
					last_leng = table[huff_count].cw_leng[codeword_count];
					weight_pos++;
					codeword_count++;
					table[huff_count].length++;
				}
			}
			p += table[huff_count].length + 16;
			tlen = tlen - 17 - table[huff_count].length;
			huff_count++;

		}

	}

#ifdef JPEG_DECODE_DEBUG
	for (int i = 0; i < huff_count; i++)
	{
		printf("Huffman table %d result:\n", i);
		printf("table length: %d   table prop: %x\n",
			table[i].length, table[i].prop);
		printf("cw_l\tcw\tweight\n");
		char buf[50];
		for (int j = 0; j < table[i].length; j++)
		{
			itoa(table[i].codeword[j], buf, 2);
			printf("  %d\t%s\t%x\n", table[i].cw_leng[j],
				buf, table[i].weight[j]);
		}
	}
#endif // JPEG_DECODE_DEBUG

	// decode start of scan info
	int sos_len = img_buf[sos] * 256 + img_buf[sos + 1];
	sod = sos + sos_len;
	som = sod;

	int c_info_len = 0;
	for (int i = 0; i < sos_len - 6; i += 2)
	{
		c_info[c_info_len].id = img_buf[sos + 3 + i];
		c_info[c_info_len].dc_huff_id = img_buf[sos + 4 + i] >> 4;
		c_info[c_info_len].ac_huff_id = img_buf[sos + 4 + i] & 0x0f;
		c_info_len++;
	}
#ifdef JPEG_DECODE_DEBUG
	printf("start of scan len %d\n", sos_len);
	for (int i = 0; i < 3; i++)
		printf("id: %d  dc: %d  ac: %d h: %d  v: %d  q_table: %d\n",
			c_info[i].id,
			c_info[i].dc_huff_id,
			c_info[i].ac_huff_id,
			c_info[i].h_sample_factor,
			c_info[i].v_sample_factor,
			c_info[i].q_table_id);
#endif // JPEG_DECODE_DEBUG

	// calculate MCU size
	hmax = c_info[0].h_sample_factor;
	vmax = c_info[0].v_sample_factor;
	if (hmax == 2 && vmax == 2) mcu_size = 2;
	else mcu_size = 1;
	if (h_size % (mcu_size * 8) == 0) h_mcu_count = h_size / (mcu_size * 8);
	else h_mcu_count = h_size / (mcu_size * 8) + 1;
	if (w_size % (mcu_size * 8) == 0) w_mcu_count = w_size / (mcu_size * 8);
	else w_mcu_count = w_size / (mcu_size * 8) + 1;

#ifdef DEBUG_2
	printf("mcusize = %d\n", mcu_size);
	printf("hmcu = %d\nwmcu = %d\n", h_mcu_count, w_mcu_count);
	printf("hsize = %d\nwsize = %d\n", h_size, w_size);

#endif // DEBUG_2

	return 0;
}

IMG_LEN jpeg_pic::decode_mcu(int mcu_id, IMG_LEN start_of_mcu)
{
	double y_data[256];
	double cr_data[64];
	double cb_data[64];

	long int cw_l = 0;    // codeword length
	int cw = 0;      // codeword
	int match = 0;
	int huff_id;
	int weight;
	int val;        // value of decode


					// decode Y
	int y_count = hmax * vmax;
#ifdef JPEG_DECODE_DEBUG
	printf("\ny_count = %d\n", y_count);
#endif // JPEG_DECODE_DEBUG
	for (int i = 0; i < y_count; i++)
	{
		// Huffman decode
		// dc decode
		cw = 0;
		cw_l = 0;
		match = 0;
		val = 0;
		// search Huffman table
		huff_id = c_info[0].dc_huff_id * 2;
		while (match == 0)
		{
			if (binary_buff.size() == 0)
			{
				int temp = img_buf[start_of_mcu];
				start_of_mcu++;
				if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
#ifdef JPEG_DECODE_DEBUG
				if (i == 0)
					printf("add %d\nstart of mcu %d\n", temp, start_of_mcu);
#endif // JPEG_DECODE_DEBUG
				binary_buff.push_back(temp >> 7);
				binary_buff.push_back((temp & 0x40) >> 6);
				binary_buff.push_back((temp & 0x20) >> 5);
				binary_buff.push_back((temp & 0x10) >> 4);
				binary_buff.push_back((temp & 0x08) >> 3);
				binary_buff.push_back((temp & 0x04) >> 2);
				binary_buff.push_back((temp & 0x02) >> 1);
				binary_buff.push_back(temp & 0x01);
			}
#ifdef JPEG_DECODE_DEBUG
			if (i == 0)
				printf("binary_buff.front = %d\n", binary_buff.front());
#endif // JPEG_DECODE_DEBUG
			cw = cw * 2 + int(binary_buff.front());
			binary_buff.pop_front();
			cw_l++;
#ifdef JPEG_DECODE_DEBUG
			if (i == 0)
				printf("cw_l = %d\ncw = %d\n", cw_l, cw);
#endif // JPEG_DECODE_DEBUG
			for (int j = 0; j < table[huff_id].length; j++)
			{
#ifdef JPEG_DECODE_DEBUG
				if (i == 0)
					printf("table[%d].cw_leng[%d] = %d\n", huff_id, j, table[huff_id].cw_leng[j]);
#endif // JPEG_DECODE_DEBUG


				if (table[huff_id].cw_leng[j] < cw_l) continue;
				if (table[huff_id].cw_leng[j] > cw_l) break;
				if (table[huff_id].codeword[j] == cw)
				{
					match = 1;
					weight = table[huff_id].weight[j];
#ifdef JPEG_DECODE_DEBUG
					if (i == 0)
						printf("weight: %d\n", weight);
#endif // JPEG_DECODE_DEBUG
					break;
				}
			}
		}
		for (int j = 0; j < weight; j++)
		{
			if (binary_buff.size() == 0)
			{
				int temp = img_buf[start_of_mcu];
				start_of_mcu++;
				if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
#ifdef JPEG_DECODE_DEBUG
				if (i == 0)
					printf("add %d\nstart of mcu %d\n", temp, start_of_mcu);
#endif // JPEG_DECODE_DEBUG

				binary_buff.push_back(temp >> 7);
				binary_buff.push_back((temp & 0x40) >> 6);
				binary_buff.push_back((temp & 0x20) >> 5);
				binary_buff.push_back((temp & 0x10) >> 4);
				binary_buff.push_back((temp & 0x08) >> 3);
				binary_buff.push_back((temp & 0x04) >> 2);
				binary_buff.push_back((temp & 0x02) >> 1);
				binary_buff.push_back(temp & 0x01);
			}
#ifdef JPEG_DECODE_DEBUG
			if (i == 0)
				printf("binary_buff.front = %d\n", binary_buff.front());
#endif // JPEG_DECODE_DEBUG
			val = val * 2 + int(binary_buff.front());
			binary_buff.pop_front();
		}
		if (val < (1 << (weight - 1))) val = val + 1 - (1 << weight);
		y_data[i * 64] = val;
#ifdef JPEG_DECODE_DEBUG
		printf("weight = %d val = %d\n", weight, val);
		printf("y_data[%d*64] = %f\n", i, y_data[i * 64]);
#endif // JPEG_DECODE_DEBUG
		// ac decode
		for (int k = 1; k < 64; k++)
		{
			cw = 0;
			cw_l = 0;
			match = 0;
			val = 0;
			// search Huffman table
			huff_id = c_info[0].ac_huff_id * 2 + 1;
			while (match == 0)
			{
				if (binary_buff.size() == 0)
				{
					int temp = img_buf[start_of_mcu];
					start_of_mcu++;
					if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
					binary_buff.push_back(temp >> 7);
					binary_buff.push_back((temp & 0x40) >> 6);
					binary_buff.push_back((temp & 0x20) >> 5);
					binary_buff.push_back((temp & 0x10) >> 4);
					binary_buff.push_back((temp & 0x08) >> 3);
					binary_buff.push_back((temp & 0x04) >> 2);
					binary_buff.push_back((temp & 0x02) >> 1);
					binary_buff.push_back(temp & 0x01);
				}
				cw = cw * 2 + int(binary_buff.front());
				binary_buff.pop_front();
				cw_l++;
				for (int j = 0; j < table[huff_id].length; j++)
				{
					if (table[huff_id].cw_leng[j] < cw_l) continue;
					if (table[huff_id].cw_leng[j] > cw_l) break;
					if (table[huff_id].codeword[j] == cw)
					{
						match = 1;
						weight = table[huff_id].weight[j];
						break;
					}
				}
			}
			if (weight == 0)
			{

				for (; k < 64; k++)
					y_data[i * 64 + k] = 0.0f;
				break;
			}
			// high 4 digits
			for (int j = 0; j < (weight >> 4); j++)
			{
				y_data[i * 64 + k] = 0;
				k++;
			}

			// low 4 digits

			for (int j = 0; j < (weight & 0x0f); j++)
			{
				if (binary_buff.size() == 0)
				{
					int temp = img_buf[start_of_mcu];
					start_of_mcu++;
					if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
					binary_buff.push_back(temp >> 7);
					binary_buff.push_back((temp & 0x40) >> 6);
					binary_buff.push_back((temp & 0x20) >> 5);
					binary_buff.push_back((temp & 0x10) >> 4);
					binary_buff.push_back((temp & 0x08) >> 3);
					binary_buff.push_back((temp & 0x04) >> 2);
					binary_buff.push_back((temp & 0x02) >> 1);
					binary_buff.push_back(temp & 0x01);
				}
				val = val * 2 + int(binary_buff.front());
				binary_buff.pop_front();
			}
			if (val < (1 << ((weight & 0x0f) - 1))) val = val + 1 - (1 << (weight & 0x0f));
			y_data[i * 64 + k] = val;
#ifdef JPEG_DECODE_DEBUG
			printf("k = %d\n", k);
#endif // JPEG_DECODE_DEBUG

		}
	}
#ifdef JPEG_DECODE_DEBUG
	printf("start of mcu in:%d\n", start_of_mcu);
#endif // JPEG_DECODE_DEBUG

	// decode cr
	// dc decode
	cw = 0;
	cw_l = 0;
	match = 0;
	val = 0;
	// search Huffman table
	huff_id = c_info[1].dc_huff_id * 2;
	while (match == 0)
	{
		if (binary_buff.size() == 0)
		{
			int temp = img_buf[start_of_mcu];
			start_of_mcu++;
			if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
			binary_buff.push_back(temp >> 7);
			binary_buff.push_back((temp & 0x40) >> 6);
			binary_buff.push_back((temp & 0x20) >> 5);
			binary_buff.push_back((temp & 0x10) >> 4);
			binary_buff.push_back((temp & 0x08) >> 3);
			binary_buff.push_back((temp & 0x04) >> 2);
			binary_buff.push_back((temp & 0x02) >> 1);
			binary_buff.push_back(temp & 0x01);
		}
		cw = cw * 2 + int(binary_buff.front());
		binary_buff.pop_front();
		cw_l++;
		for (int j = 0; j < table[huff_id].length; j++)
		{
			if (table[huff_id].cw_leng[j] < cw_l) continue;
			if (table[huff_id].cw_leng[j] > cw_l) break;
			if (table[huff_id].codeword[j] == cw)
			{
				match = 1;
				weight = table[huff_id].weight[j];
				break;
			}
		}
	}
	for (int j = 0; j < weight; j++)
	{
		if (binary_buff.size() == 0)
		{
			int temp = img_buf[start_of_mcu];
			start_of_mcu++;
			if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
			binary_buff.push_back(temp >> 7);
			binary_buff.push_back((temp & 0x40) >> 6);
			binary_buff.push_back((temp & 0x20) >> 5);
			binary_buff.push_back((temp & 0x10) >> 4);
			binary_buff.push_back((temp & 0x08) >> 3);
			binary_buff.push_back((temp & 0x04) >> 2);
			binary_buff.push_back((temp & 0x02) >> 1);
			binary_buff.push_back(temp & 0x01);
		}
		val = val * 2 + int(binary_buff.front());
		binary_buff.pop_front();
	}
	if (val < (1 << (weight - 1))) val = val + 1 - (1 << weight);
	cr_data[0] = val;
	// ac decode
	for (int k = 1; k < 64; k++)
	{
		cw = 0;
		cw_l = 0;
		match = 0;
		val = 0;
		// search Huffman table
		huff_id = c_info[1].ac_huff_id * 2 + 1;
		while (match == 0)
		{
			if (binary_buff.size() == 0)
			{
				int temp = img_buf[start_of_mcu];
				start_of_mcu++;
				if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
				binary_buff.push_back(temp >> 7);
				binary_buff.push_back((temp & 0x40) >> 6);
				binary_buff.push_back((temp & 0x20) >> 5);
				binary_buff.push_back((temp & 0x10) >> 4);
				binary_buff.push_back((temp & 0x08) >> 3);
				binary_buff.push_back((temp & 0x04) >> 2);
				binary_buff.push_back((temp & 0x02) >> 1);
				binary_buff.push_back(temp & 0x01);
			}
			cw = cw * 2 + int(binary_buff.front());
			binary_buff.pop_front();
			cw_l++;
			for (int j = 0; j < table[huff_id].length; j++)
			{
				if (table[huff_id].cw_leng[j] < cw_l) continue;
				if (table[huff_id].cw_leng[j] > cw_l) break;
				if (table[huff_id].codeword[j] == cw)
				{
					match = 1;
					weight = table[huff_id].weight[j];
					break;
				}
			}
		}
		if (weight == 0)
		{
			for (; k < 64; k++)
				cr_data[k] = 0;
			break;
		}
		// high 4 digits
		for (int j = 0; j < (weight >> 4); j++)
		{
			cr_data[k] = 0;
			k++;
		}
		// low 4 digits

		for (int j = 0; j < (weight & 0x0f); j++)
		{
			if (binary_buff.size() == 0)
			{
				int temp = img_buf[start_of_mcu];
				start_of_mcu++;
				if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
				binary_buff.push_back(temp >> 7);
				binary_buff.push_back((temp & 0x40) >> 6);
				binary_buff.push_back((temp & 0x20) >> 5);
				binary_buff.push_back((temp & 0x10) >> 4);
				binary_buff.push_back((temp & 0x08) >> 3);
				binary_buff.push_back((temp & 0x04) >> 2);
				binary_buff.push_back((temp & 0x02) >> 1);
				binary_buff.push_back(temp & 0x01);
			}
			val = val * 2 + int(binary_buff.front());
			binary_buff.pop_front();
		}
		if (val < (1 << ((weight & 0x0f) - 1))) val = val + 1 - (1 << (weight & 0x0f));
		cr_data[k] = val;

	}
#ifdef JPEG_DECODE_DEBUG
	//for(int i = 0; i < binary_buff.size();i++)
	printf("%d ", binary_buff.size());
#endif // JPEG_DECODE_DEBUG

	// decode cr
	// dc decode
	cw = 0;
	cw_l = 0;
	match = 0;
	val = 0;
	// search Huffman table
#ifdef JPEG_DECODE_DEBUG
	printf("\n\n\n");
#endif // JPEG_DECODE_DEBUG
	huff_id = c_info[2].dc_huff_id * 2;
	while (match == 0)
	{
		if (binary_buff.size() == 0)
		{
			int temp = img_buf[start_of_mcu];
			start_of_mcu++;
			if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
#ifdef JPEG_DECODE_DEBUG
			printf("\nadd %d\n", temp);
#endif // JPEG_DECODE_DEBUG
			binary_buff.push_back(temp >> 7);
			binary_buff.push_back((temp & 0x40) >> 6);
			binary_buff.push_back((temp & 0x20) >> 5);
			binary_buff.push_back((temp & 0x10) >> 4);
			binary_buff.push_back((temp & 0x08) >> 3);
			binary_buff.push_back((temp & 0x04) >> 2);
			binary_buff.push_back((temp & 0x02) >> 1);
			binary_buff.push_back(temp & 0x01);
		}
#ifdef JPEG_DECODE_DEBUG
		printf("binary_buff.front = %d\n", binary_buff.front());
#endif // JPEG_DECODE_DEBUG
		cw = cw * 2 + int(binary_buff.front());
		binary_buff.pop_front();
		cw_l++;
#ifdef JPEG_DECODE_DEBUG
		printf("cw_l = %d\ncw = %d\n", cw_l, cw);
#endif // JPEG_DECODE_DEBUG
		for (int j = 0; j < table[huff_id].length; j++)
		{
#ifdef JPEG_DECODE_DEBUG
			printf("table[%d].cw_leng[%d] = %d\n", huff_id, j, table[huff_id].cw_leng[j]);
#endif // JPEG_DECODE_DEBUG

			if (table[huff_id].cw_leng[j] < cw_l) continue;
			if (table[huff_id].cw_leng[j] > cw_l) break;
			if (table[huff_id].codeword[j] == cw)
			{
				match = 1;
				weight = table[huff_id].weight[j];
#ifdef JPEG_DECODE_DEBUG
				printf("weight: %d\n", weight);
#endif // JPEG_DECODE_DEBUG
				break;
			}
		}
	}
#ifdef JPEG_DECODE_DEBUG
	printf("\n\nweight = %d\n\n", weight);
#endif // JPEG_DECODE_DEBUG
	for (int j = 0; j < weight; j++)
	{
		if (binary_buff.size() == 0)
		{
			int temp = img_buf[start_of_mcu];
			start_of_mcu++;
			if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
			binary_buff.push_back(temp >> 7);
			binary_buff.push_back((temp & 0x40) >> 6);
			binary_buff.push_back((temp & 0x20) >> 5);
			binary_buff.push_back((temp & 0x10) >> 4);
			binary_buff.push_back((temp & 0x08) >> 3);
			binary_buff.push_back((temp & 0x04) >> 2);
			binary_buff.push_back((temp & 0x02) >> 1);
			binary_buff.push_back(temp & 0x01);
		}
		val = val * 2 + int(binary_buff.front());
		binary_buff.pop_front();
	}
	if (val < (1 << (weight - 1))) val = val + 1 - (1 << weight);
	cb_data[0] = val;
	// ac decode
	for (int k = 1; k < 64; k++)
	{
		cw = 0;
		cw_l = 0;
		match = 0;
		val = 0;
		// search Huffman table
		huff_id = c_info[2].ac_huff_id * 2 + 1;
		while (match == 0)
		{
			if (binary_buff.size() == 0)
			{
				int temp = img_buf[start_of_mcu];
				start_of_mcu++;
				if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
				binary_buff.push_back(temp >> 7);
				binary_buff.push_back((temp & 0x40) >> 6);
				binary_buff.push_back((temp & 0x20) >> 5);
				binary_buff.push_back((temp & 0x10) >> 4);
				binary_buff.push_back((temp & 0x08) >> 3);
				binary_buff.push_back((temp & 0x04) >> 2);
				binary_buff.push_back((temp & 0x02) >> 1);
				binary_buff.push_back(temp & 0x01);
			}
			cw = cw * 2 + int(binary_buff.front());
			binary_buff.pop_front();
			cw_l++;
			for (int j = 0; j < table[huff_id].length; j++)
			{
				if (table[huff_id].cw_leng[j] < cw_l) continue;
				if (table[huff_id].cw_leng[j] > cw_l) break;
				if (table[huff_id].codeword[j] == cw)
				{
					match = 1;
					weight = table[huff_id].weight[j];
					break;
				}
			}
		}
		if (weight == 0)
		{
			for (; k < 64; k++)
				cb_data[k] = 0;
			break;
		}
		// high 4 digits
		for (int j = 0; j < (weight >> 4); j++)
		{
			cb_data[k] = 0;
			k++;
		}
		// low 4 digits

		for (int j = 0; j < (weight & 0x0f); j++)
		{
			if (binary_buff.size() == 0)
			{
				int temp = img_buf[start_of_mcu];
				start_of_mcu++;
				if (temp == 0xff) { temp = 0xff; start_of_mcu++; }
				binary_buff.push_back(temp >> 7);
				binary_buff.push_back((temp & 0x40) >> 6);
				binary_buff.push_back((temp & 0x20) >> 5);
				binary_buff.push_back((temp & 0x10) >> 4);
				binary_buff.push_back((temp & 0x08) >> 3);
				binary_buff.push_back((temp & 0x04) >> 2);
				binary_buff.push_back((temp & 0x02) >> 1);
				binary_buff.push_back(temp & 0x01);
			}
			val = val * 2 + int(binary_buff.front());
			binary_buff.pop_front();
		}
		if (val < (1 << ((weight & 0x0f) - 1))) val = val + 1 - (1 << (weight & 0x0f));
		cb_data[k] = val;

	}
#ifdef JPEG_DECODE_DEBUG
	putchar('\n\n');
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[i * 8 + j]));
		}
		printf("\n");
	}
#endif // JPEG_DECODE_DEBUG

#ifdef DEBUG_1
	printf("decode huffman\n");
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 0 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 1 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 2 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 3 + i * 8 + j]));
		}
		printf("\n");
	}
#endif // DEBUG_1


	// inverse diff
	y_data[0] += y_diff;
	cr_data[0] += cr_diff;
	cb_data[0] += cb_diff;
	y_diff = int(y_data[0]);
	for (int i = 0; i < y_count - 1; i++)
	{
		y_data[64 + 64 * i] += y_data[64 * i];
		y_diff = int(y_data[64 + 64 * i]);
	}
	cr_diff = int(cr_data[0]);
	cb_diff = int(cb_data[0]);
#ifdef DEBUG_1

	printf("decode diff\n");
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 0 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 1 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 2 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 3 + i * 8 + j]));
		}
		printf("\n");
	}
#endif // DEBUG_1
	// inverse quantization
	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < y_count; j++)
		{
			y_data[j * 64 + i] *= q_table[c_info[0].q_table_id].val[i];
		}
		cr_data[i] *= q_table[c_info[1].q_table_id].val[i];
		cb_data[i] *= q_table[c_info[2].q_table_id].val[i];
	}
#ifdef JPEG_DECODE_DEBUG
	puts("\n\n after inverse quantization\n");
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[i * 8 + j]));
		}
		printf("\n");
	}
	putchar('\n');
#endif // JPEG_DECODE_DEBUG

	// inverse zigzag
	double inv_y[256];
	double inv_cr[64];
	double inv_cb[64];

	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < y_count; j++)
		{
			inv_y[j * 64 + zig_zag_table[i]] = y_data[j * 64 + i];
		}
		inv_cr[zig_zag_table[i]] = cr_data[i];
		inv_cb[zig_zag_table[i]] = cb_data[i];
	}
#ifdef DEBUG_1
	printf("decode zigzag\n");
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(inv_y[64 * 0 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(inv_y[64 * 1 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(inv_y[64 * 2 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(inv_y[64 * 3 + i * 8 + j]));
		}
		printf("\n");
	}
#endif // DEBUG_1
#ifdef JPEG_DECODE_DEBUG
	puts("\n\n after inverse zigzag\n");
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[i * 8 + j]));
		}
		printf("\n");
	}
	putchar('\n');
#endif // JPEG_DECODE_DEBUG

	// inverse dct
	for (int i = 0; i < y_count; i++)
	{
		inv_dct64(inv_y + i * 64, y_data + i * 64);
	}
	inv_dct64(inv_cr, cr_data);
	inv_dct64(inv_cb, cb_data);

	for (int i = 0; i < 64 * y_count; i++) {
		if (y_data[i] < -128) y_data[i] = -128;
		else if (y_data[i] > 127) y_data[i] = 127;
	}
	for (int i = 0; i < 64; i++) {
		if (cr_data[i] < -128) cr_data[i] = -128;
		else if (cr_data[i] > 127) cr_data[i] = 127;
	}
	for (int i = 0; i < 64; i++) {
		if (cb_data[i] < -128) cb_data[i] = -128;
		else if (cb_data[i] > 127) cb_data[i] = 127;
	}

#ifdef DEBUG_1
	printf("decode idct\n");
	printf("y\n");
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 0 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 1 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 2 + i * 8 + j]));
		}
		printf("   ");
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", int(y_data[64 * 3 + i * 8 + j]));
		}
		printf("\n");
	}

	printf("cr\n");
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			//printf("%d ", int(cb_data[64 * 0 + i *8 +j]));
			printf("%d ", int(cr_data[64 * 0 + i * 8 + j]));
		}
		printf("\n");
	}
	printf("cb\n");
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			//printf("%d ", int(cb_data[64 * 0 + i *8 +j]));
			printf("%d ", int(cb_data[64 * 0 + i * 8 + j]));
		}
		printf("\n");
	}
#endif // DEBUG_1

	// ycrcb to rgb
	double r_temp[256];
	double g_temp[256];
	double b_temp[256];

	for (int i = 0; i < 8 * hmax; i++)
	{
		for (int j = 0; j < 8 * vmax; j++)
		{

			int mat_sel = i / 8 * 2 + j / 8;
			int h_v = i % 8;
			int w_v = j % 8;
			r_temp[i * 8 * hmax + j] = y_data[h_v * 8 + w_v + mat_sel * 64]
				+ 1.402 * (cb_data[(i / hmax) * 8 + j / hmax]) + 128 + 0.5;
			g_temp[i * 8 * hmax + j] = y_data[h_v * 8 + w_v + mat_sel * 64]
				- 0.34414 * (cr_data[(i / hmax) * 8 + j / hmax])
				- 0.71414 * (cb_data[(i / hmax) * 8 + j / hmax]) + 128 + 0.5;
			b_temp[i * 8 * hmax + j] = y_data[h_v * 8 + w_v + mat_sel * 64]
				+ 1.772 * (cr_data[(i / hmax) * 8 + j / hmax]) + 128 + 0.5;
		}
	}

	for (int i = 0; i < 64 * hmax * vmax; i++)
	{
		if (r_temp[i] > 255) mcu_r[i] = 255;
		else if (r_temp[i] < 0) mcu_r[i] = 0;
		else mcu_r[i] = char(r_temp[i]);
		if (g_temp[i] > 255) mcu_g[i] = 255;
		else if (g_temp[i] < 0) mcu_g[i] = 0;
		else mcu_g[i] = char(g_temp[i]);
		if (b_temp[i] > 255) mcu_b[i] = 255;
		else if (b_temp[i] < 0) mcu_b[i] = 0;
		else mcu_b[i] = char(b_temp[i]);
	}
#ifdef DEBUG_rgb
	printf("decode color\n");
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			printf("%03d ", mcu_r[i * 16 + j]);
		}
		printf("\n");
	}
#endif // DEBUG_rgb
#ifdef JPEG_DECODE_DEBUG
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%.1f ", y_data[i * 8 + j]);
		}
		printf("\n");
	}
	putchar('\n');
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%.1f ", cr_data[i * 8 + j]);
		}
		printf("\n");
	}
	putchar('\n');
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%.1f ", cb_data[i * 8 + j]);
		}
		printf("\n");
	}
	// rgb
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%.1f ", r_temp[i * 8 + j]);
		}
		printf("\n");
	}
	putchar('\n');
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%.1f ", g_temp[i * 8 + j]);
		}
		printf("\n");
	}
	putchar('\n');
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%.1f ", b_temp[i * 8 + j]);
		}
		printf("\n");
	}
	// char rgb
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", mcu_r[i * 8 + j]);
		}
		printf("\n");
	}
	putchar('\n');
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", mcu_g[i * 8 + j]);
		}
		printf("\n");
	}
	putchar('\n');
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			printf("%d ", mcu_b[i * 8 + j]);
		}
		printf("\n");
	}
#endif // JPEG_DECODE_DEBUG

	return start_of_mcu;

}

int jpeg_pic::get_mcu_len()
{
	return hmax * 8;
}

unsigned char jpeg_pic::get_mcu_r(int x)
{
	return mcu_r[x];
}
unsigned char jpeg_pic::get_mcu_g(int x)
{
	return mcu_g[x];
}
unsigned char jpeg_pic::get_mcu_b(int x)
{
	return mcu_b[x];
}

void jpeg_pic::decode_mcu_test()
{
	decode_mcu(0, sod);
}

void jpeg_pic::decode_next_mcu()
{
	som = decode_mcu(0, som);
}

void jpeg_pic::reset_som() 
{ 
	som = sod; 
	binary_buff.clear(); 
	y_diff = 0;
	cr_diff = 0;
	cb_diff = 0;
}

void jpeg_pic::to_rgb()
{
	r_buffer = (unsigned char *)malloc(h_mcu_count * w_mcu_count * mcu_size * mcu_size * 64 * sizeof(char));
	g_buffer = (unsigned char *)malloc(h_mcu_count * w_mcu_count * mcu_size * mcu_size * 64 * sizeof(char));
	b_buffer = (unsigned char *)malloc(h_mcu_count * w_mcu_count * mcu_size * mcu_size * 64 * sizeof(char));

	for (int x = 0; x < h_mcu_count; x++)
	{
		for (int y = 0; y < w_mcu_count; y++)
		{
			this->decode_next_mcu();
			
			for (int i = 0; i < hmax * 8; i++)
			{
				for (int j = 0; j < hmax * 8; j++)
				{
					r_buffer[j + y * hmax * 8 + (i + x * hmax * 8) * w_mcu_count * hmax * 8]
						= mcu_r[i * hmax * 8 + j];
					g_buffer[j + y * hmax * 8 + (i + x * hmax * 8) * w_mcu_count * hmax * 8]
						= mcu_g[i * hmax * 8 + j];
					b_buffer[j + y * hmax * 8 + (i + x * hmax * 8) * w_mcu_count * hmax * 8]
						= mcu_b[i * hmax * 8 + j];

				}
			}
		}
	}
}

unsigned char jpeg_pic::get_pic_r(int x, int y)
{
	return r_buffer[x + y * w_mcu_count * get_mcu_len()];
}
unsigned char jpeg_pic::get_pic_g(int x, int y)
{
	return g_buffer[x + y * w_mcu_count * get_mcu_len()];
}
unsigned char jpeg_pic::get_pic_b(int x, int y)
{
	return b_buffer[x + y * w_mcu_count * get_mcu_len()];
}

void jpeg_pic::errmsg(int i)
{	
	switch (i)
	{
	case 0: msg = "Not a baseline JPEG file!";
		break;
	case 1: msg = "Not a JPEG file!";
		break;
	case 2: msg = "Progressive JPEG decoder incompleted!";
		break;
	case 9: msg = "Unable to write in bmp file!";
		break;
	default: msg = "Error!";
		break;
	}
	return;
}

std::string jpeg_pic::get_msg()
{
	return msg;
}

bool jpeg_pic::decode_info_p()
{
	// progressive decoder not complete
#ifdef PROGRESSIVE_DEBUG
	IMG_LEN img_p;   // indicate the position of image scan
	int end_of_file;

	img_p = 0;
	end_of_file = 1;

	while (end_of_file)
	{
		
	}
#else
	errmsg(2);
	return 1;



#endif
}

void jpeg_pic::to_bmp(std::string file_path)
{
	unsigned long int data_size_per_line = ((w_size * 24 + 31) >> 5) << 2;
	unsigned long int file_size = h_size * data_size_per_line;
	unsigned long int color_size = h_size * w_size;
	int skip_bits = data_size_per_line - w_size * 3;
	unsigned char bits_fill[4] = { 0, 0, 0, 0 };
	int w_len = w_mcu_count * mcu_size * 8;
	BMPFILEHEADER bfh;
	BMPINFOHEADER bih;
	// bmp file header
	bfh.bfType = 0x4d42;
	bfh.bfSize = file_size + 54;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = 54;
	// bmp info header
	bih.biSize = sizeof(BMPINFOHEADER);
	bih.biWidth = w_size;
	//bih.biHeight = -h_size;
	bih.biHeight = h_size;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = 0;
	bih.biSizeImage = file_size;
	bih.biXPelsPerMeter = 3780;
	bih.biYPelsPerMeter = 3780;
	bih.biClrUsed = 0;  
	bih.biClrImportant = 0;

	FILE *fp = fopen(file_path.c_str(), "wb");
	if (!fp)
	{
		errmsg(9);
		return;
	}

	fwrite(&bfh.bfType, sizeof(bfh.bfType), 1, fp);
	fwrite(&bfh.bfSize, sizeof(bfh.bfSize), 1, fp);
	fwrite(&bfh.bfReserved1, sizeof(bfh.bfReserved1), 1, fp);
	fwrite(&bfh.bfReserved2, sizeof(bfh.bfReserved2), 1, fp);
	fwrite(&bfh.bfOffBits, sizeof(bfh.bfOffBits), 1, fp);

	fwrite(&bih.biSize, sizeof(bih.biSize), 1, fp);
	fwrite(&bih.biWidth, sizeof(bih.biWidth), 1, fp);
	fwrite(&bih.biHeight, sizeof(bih.biHeight), 1, fp);
	fwrite(&bih.biPlanes, sizeof(bih.biPlanes), 1, fp);
	fwrite(&bih.biBitCount, sizeof(bih.biBitCount), 1, fp);
	fwrite(&bih.biCompression, sizeof(bih.biCompression), 1, fp);
	fwrite(&bih.biSizeImage, sizeof(bih.biSizeImage), 1, fp);
	fwrite(&bih.biXPelsPerMeter, sizeof(bih.biXPelsPerMeter), 1, fp);
	fwrite(&bih.biYPelsPerMeter, sizeof(bih.biYPelsPerMeter), 1, fp);
	fwrite(&bih.biClrUsed, sizeof(bih.biClrUsed), 1, fp);
	fwrite(&bih.biClrImportant, sizeof(bih.biClrImportant), 1, fp);
	
	/*
	fwrite(b_buffer, color_size, 1, fp);
	fwrite(g_buffer, color_size, 1, fp);
	fwrite(r_buffer, color_size, 1, fp);
	*/
	//for (int i = 0; i < h_size; i++)	
	for (int i = h_size - 1; i >= 0; i--)
	{
		for (int j = 0; j < w_size; j++) 
		{
			fwrite(b_buffer + i * w_len + j, sizeof(char), 1, fp);
			fwrite(g_buffer + i * w_len + j, sizeof(char), 1, fp);
			fwrite(r_buffer + i * w_len + j, sizeof(char), 1, fp);
		}
		//fwrite(bits_fill, sizeof(char), skip_bits, fp);
		for (int k = 0; k < skip_bits; k++)
			fwrite(bits_fill, sizeof(char), 1, fp);
	}
	fclose(fp);
}

void jpeg_pic::get_rgb(unsigned char * bitdata)
{
	

	int k = 0;
	for (int j = 0; j < h_size; j++)
	{
		for (int i = 0; i < w_size; i++)
		{
			bitdata[k] = r_buffer[i + j * w_mcu_count * get_mcu_len()];
			bitdata[k + 1] = g_buffer[i + j * w_mcu_count * get_mcu_len()];
			bitdata[k + 2] = b_buffer[i + j * w_mcu_count * get_mcu_len()];
			
			k += 3;
		}
	}
	
}
