#include "kernels.h"
#include <iostream>
extern "C"{
void HHTriCount(
		RAW_ID_TYPE heavy_offset_left[LIGHT_OFFSET_LEN],
		BID_TYPE heavy_bid_left[LIGHT_ADJ_LEN],
		VID_TYPE heavy_value_left[LIGHT_ADJ_LEN],

		RAW_ID_TYPE heavy_offset_right[LIGHT_OFFSET_LEN],
		BID_TYPE heavy_bid_right[LIGHT_ADJ_LEN],
		VID_TYPE heavy_value_right[LIGHT_ADJ_LEN],

		RAW_ID_TYPE task_left[TASK_LEN],
		RAW_ID_TYPE task_right[TASK_LEN],

		int task_num,
		unsigned long long output[OUT_LEN])
{
#pragma HLS INTERFACE m_axi port = heavy_offset_left offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = heavy_bid_left offset = slave bundle = gmem3
#pragma HLS INTERFACE m_axi port = heavy_value_left offset = slave bundle = gmem4
#pragma HLS INTERFACE m_axi port = heavy_offset_right offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = heavy_bid_right offset = slave bundle = gmem5
#pragma HLS INTERFACE m_axi port = heavy_value_right offset = slave bundle = gmem6
#pragma HLS INTERFACE m_axi port = task_left offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = task_right offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = output offset = slave bundle = gmem2

#pragma HLS INTERFACE s_axilite port = heavy_offset_left bundle = control
#pragma HLS INTERFACE s_axilite port = heavy_bid_left bundle = control
#pragma HLS INTERFACE s_axilite port = heavy_value_left bundle = control
#pragma HLS INTERFACE s_axilite port = heavy_offset_right bundle = control
#pragma HLS INTERFACE s_axilite port = heavy_bid_right bundle = control
#pragma HLS INTERFACE s_axilite port = heavy_value_right bundle = control
#pragma HLS INTERFACE s_axilite port = task_left bundle = control
#pragma HLS INTERFACE s_axilite port = task_right bundle = control
#pragma HLS INTERFACE s_axilite port = task_num bundle = control
#pragma HLS INTERFACE s_axilite port = output bundle = control

	static RAW_ID_TYPE task_buffer_left[TASK_NUM_UP_BOUND];
	static RAW_ID_TYPE task_buffer_right[TASK_NUM_UP_BOUND];

	static BID_TYPE bid_buffer_left[ADJ_NUM_UP_BOUND];
	static BID_TYPE bid_buffer_right[ADJ_NUM_UP_BOUND];

	static VID_TYPE value_buffer_left[ADJ_NUM_UP_BOUND];
	static VID_TYPE value_buffer_right[ADJ_NUM_UP_BOUND];

	//std::cout << "hello" << std::endl;

	static unsigned long long table[256];

	int task_num_local = 0;
	RAW_ID_TYPE i,j,li,ri,k,til,tir;

	unsigned long long res = 0;
	unsigned long long res2 = 0;

	//adj_paras
	int adj_size_left,adj_size_right;

	//merge paras
	BID_TYPE bid_left,bid_right;
	VID_TYPE value_left,value_right,value_res;
	bool is_equal;
	bool is_greater;

	//offset paras
	RAW_ID_TYPE id_left,id_right,last_left,last_right;
	RAW_ID_TYPE adj_begin_left,adj_begin_right;
	RAW_ID_TYPE adj_end_left,adj_end_right;

	//initial
	task_num_local = task_num;
	res = 0;
	til = 0;
	tir = 0;

	table[0] = 0;table[1] = 1;table[2] = 1;table[3] = 2;table[4] = 1;table[5] = 2;table[6] = 2;table[7] = 3;

	table[8] = 1;table[9] = 2;table[10] = 2;table[11] = 3;table[12] = 2;table[13] = 3;table[14] = 3;table[15] = 4;

	table[16] = 1;table[17] = 2;table[18] = 2;table[19] = 3;table[20] = 2;table[21] = 3;table[22] = 3;table[23] = 4;

	table[24] = 2;table[25] = 3;table[26] = 3;table[27] = 4;table[28] = 3;table[29] = 4;table[30] = 4;table[31] = 5;

	table[32] = 1;table[33] = 2;table[34] = 2;table[35] = 3;table[36] = 2;table[37] = 3;table[38] = 3;table[39] = 4;

	table[40] = 2;table[41] = 3;table[42] = 3;table[43] = 4;table[44] = 3;table[45] = 4;table[46] = 4;table[47] = 5;

	table[48] = 2;table[49] = 3;table[50] = 3;table[51] = 4;table[52] = 3;table[53] = 4;table[54] = 4;table[55] = 5;

	table[56] = 3;table[57] = 4;table[58] = 4;table[59] = 5;table[60] = 4;table[61] = 5;table[62] = 5;table[63] = 6;

	table[64] = 1;table[65] = 2;table[66] = 2;table[67] = 3;table[68] = 2;table[69] = 3;table[70] = 3;table[71] = 4;

	table[72] = 2;table[73] = 3;table[74] = 3;table[75] = 4;table[76] = 3;table[77] = 4;table[78] = 4;table[79] = 5;

	table[80] = 2;table[81] = 3;table[82] = 3;table[83] = 4;table[84] = 3;table[85] = 4;table[86] = 4;table[87] = 5;

	table[88] = 3;table[89] = 4;table[90] = 4;table[91] = 5;table[92] = 4;table[93] = 5;table[94] = 5;table[95] = 6;

	table[96] = 2;table[97] = 3;table[98] = 3;table[99] = 4;table[100] = 3;table[101] = 4;table[102] = 4;table[103] = 5;

	table[104] = 3;table[105] = 4;table[106] = 4;table[107] = 5;table[108] = 4;table[109] = 5;table[110] = 5;table[111] = 6;

	table[112] = 3;table[113] = 4;table[114] = 4;table[115] = 5;table[116] = 4;table[117] = 5;table[118] = 5;table[119] = 6;

	table[120] = 4;table[121] = 5;table[122] = 5;table[123] = 6;table[124] = 5;table[125] = 6;table[126] = 6;table[127] = 7;

	table[128] = 1;table[129] = 2;table[130] = 2;table[131] = 3;table[132] = 2;table[133] = 3;table[134] = 3;table[135] = 4;

	table[136] = 2;table[137] = 3;table[138] = 3;table[139] = 4;table[140] = 3;table[141] = 4;table[142] = 4;table[143] = 5;

	table[144] = 2;table[145] = 3;table[146] = 3;table[147] = 4;table[148] = 3;table[149] = 4;table[150] = 4;table[151] = 5;

	table[152] = 3;table[153] = 4;table[154] = 4;table[155] = 5;table[156] = 4;table[157] = 5;table[158] = 5;table[159] = 6;

	table[160] = 2;table[161] = 3;table[162] = 3;table[163] = 4;table[164] = 3;table[165] = 4;table[166] = 4;table[167] = 5;

	table[168] = 3;table[169] = 4;table[170] = 4;table[171] = 5;table[172] = 4;table[173] = 5;table[174] = 5;table[175] = 6;

	table[176] = 3;table[177] = 4;table[178] = 4;table[179] = 5;table[180] = 4;table[181] = 5;table[182] = 5;table[183] = 6;

	table[184] = 4;table[185] = 5;table[186] = 5;table[187] = 6;table[188] = 5;table[189] = 6;table[190] = 6;table[191] = 7;

	table[192] = 2;table[193] = 3;table[194] = 3;table[195] = 4;table[196] = 3;table[197] = 4;table[198] = 4;table[199] = 5;

	table[200] = 3;table[201] = 4;table[202] = 4;table[203] = 5;table[204] = 4;table[205] = 5;table[206] = 5;table[207] = 6;

	table[208] = 3;table[209] = 4;table[210] = 4;table[211] = 5;table[212] = 4;table[213] = 5;table[214] = 5;table[215] = 6;

	table[216] = 4;table[217] = 5;table[218] = 5;table[219] = 6;table[220] = 5;table[221] = 6;table[222] = 6;table[223] = 7;

	table[224] = 3;table[225] = 4;table[226] = 4;table[227] = 5;table[228] = 4;table[229] = 5;table[230] = 5;table[231] = 6;

	table[232] = 4;table[233] = 5;table[234] = 5;table[235] = 6;table[236] = 5;table[237] = 6;table[238] = 6;table[239] = 7;

	table[240] = 4;table[241] = 5;table[242] = 5;table[243] = 6;table[244] = 5;table[245] = 6;table[246] = 6;table[247] = 7;

	table[248] = 5;table[249] = 6;table[250] = 6;table[251] = 7;table[252] = 6;table[253] = 7;table[254] = 7;table[255] = 8;

	//std::cout << "task_num = " << task_num_local << std::endl;
	for(k = 0; k < task_num_local ; k++){

		READ_TASK_LEFT: for(i = 0; i < 4096; i++){
			task_buffer_left[i] = task_left[til];
			til++;
		}

		READ_TASK_RIGHT: for(j = 0; j < 4096; j++){
			task_buffer_right[j] = task_right[tir];
			tir++;
		}

		//std::cout << "task loaded" << std::endl;
		//std::cout << "writing to " << out_pos << std::endl;

		MERGE: for(i = 0,last_left=END_FLAG,last_right=END_FLAG; i < 4096; i++){
			id_left = task_buffer_left[i];
			id_right = task_buffer_right[i];

			adj_begin_left = heavy_offset_left[id_left];
			adj_begin_right = heavy_offset_right[id_right];

			adj_end_left = heavy_offset_left[id_left + 1];
			adj_end_right = heavy_offset_right[id_right + 1];

			adj_size_left = adj_end_left - adj_begin_left;
			adj_size_right = adj_end_right - adj_begin_right;

			//std::cout << "dealing with "<< i << "th task" << std::endl;
			//std::cout << id_left << " : " << adj_begin_left << " - " << adj_end_left << std::endl;
			//std::cout << id_right << " : " << adj_begin_right << " - " << adj_end_right << std::endl;

			READ_ADJ_LEFT: for(li = 0; li < adj_size_left && id_left != last_left; li++){
				bid_buffer_left[li] = heavy_bid_left[adj_begin_left + li];
				value_buffer_left[li] = heavy_value_left[adj_begin_left + li];
			}

			READ_ADJ_RIGHT: for(ri = 0; ri < adj_size_right && id_right != last_right; ri++){
				bid_buffer_right[ri] = heavy_bid_right[adj_begin_right + ri];
				value_buffer_right[ri] = heavy_value_right[adj_begin_right + ri];
			}
			//std::cout << "adj read" << std::endl;
			last_left = id_left;
			last_right = id_right;

			DO_MERGE: for(li = 0, ri = 0; li < adj_size_left && ri < adj_size_right;){
				bid_left = bid_buffer_left[li];
				bid_right = bid_buffer_right[ri];
				is_equal = (bid_left==bid_right);
				is_greater = (bid_left > bid_right);
				//is_less = (bid_left < bid_right);

				if(is_equal){
					value_left = value_buffer_left[li];
					value_right = value_buffer_right[ri];
					value_res = value_left & value_right;
					res += table[value_res &0xff] +
							table[(value_res >>8) &0xff] +
							table[(value_res >>16) &0xff] +
							table[(value_res >>24) &0xff] ;
					res2 += 1;
					//std::cout << "res updated : " << res << std::endl;
					li++;
					ri++;
				}
				else if(is_greater){
					ri++;

				}
				else{
					li++;
				}
			}
		}
	}

	output[0] = res;
	output[1] = res2;
	value_res = 8;
	res2 = table[value_res &0xff] +
			table[(value_res >>8) &0xff] +
			table[(value_res >>16) &0xff] +
			table[(value_res >>24) &0xff] ;
	output[2] = res2;
	return;
}

void HHTriCount_CPU(
		RAW_ID_TYPE heavy_offset_left[LIGHT_OFFSET_LEN],
		BID_TYPE heavy_bid_left[LIGHT_ADJ_LEN],
		VID_TYPE heavy_value_left[LIGHT_ADJ_LEN],

		RAW_ID_TYPE heavy_offset_right[LIGHT_OFFSET_LEN],
		BID_TYPE heavy_bid_right[LIGHT_ADJ_LEN],
		VID_TYPE heavy_value_right[LIGHT_ADJ_LEN],

		RAW_ID_TYPE task_left[TASK_LEN],
		RAW_ID_TYPE task_right[TASK_LEN],

		int task_num,
		unsigned long long output[OUT_LEN])
{
	//static RAW_ID_TYPE task_buffer_left[TASK_NUM_UP_BOUND+10000];
	//static RAW_ID_TYPE task_buffer_right[TASK_NUM_UP_BOUND+10000];

	static BID_TYPE bid_buffer_left[ADJ_NUM_UP_BOUND];
	static BID_TYPE bid_buffer_right[ADJ_NUM_UP_BOUND];

	static VID_TYPE value_buffer_left[ADJ_NUM_UP_BOUND];
	static VID_TYPE value_buffer_right[ADJ_NUM_UP_BOUND];

	//std::cout << "hello" << std::endl;

	static unsigned long long table[256];

	int task_num_local = 0;
	RAW_ID_TYPE i,j,li,ri,k;

	unsigned long long res = 0;

	//adj_paras
	int adj_size_left,adj_size_right;

	//merge paras
	BID_TYPE bid_left,bid_right;
	VID_TYPE value_left,value_right,value_res;
	bool is_equal;
	bool is_greater;

	//offset paras
	RAW_ID_TYPE id_left,id_right,last_left,last_right;
	RAW_ID_TYPE adj_begin_left,adj_begin_right;
	RAW_ID_TYPE adj_end_left,adj_end_right;

	//initial
	task_num_local = task_num;
	res = 0;

	table[0] = 0;table[1] = 1;table[2] = 1;table[3] = 2;table[4] = 1;table[5] = 2;table[6] = 2;table[7] = 3;

	table[8] = 1;table[9] = 2;table[10] = 2;table[11] = 3;table[12] = 2;table[13] = 3;table[14] = 3;table[15] = 4;

	table[16] = 1;table[17] = 2;table[18] = 2;table[19] = 3;table[20] = 2;table[21] = 3;table[22] = 3;table[23] = 4;

	table[24] = 2;table[25] = 3;table[26] = 3;table[27] = 4;table[28] = 3;table[29] = 4;table[30] = 4;table[31] = 5;

	table[32] = 1;table[33] = 2;table[34] = 2;table[35] = 3;table[36] = 2;table[37] = 3;table[38] = 3;table[39] = 4;

	table[40] = 2;table[41] = 3;table[42] = 3;table[43] = 4;table[44] = 3;table[45] = 4;table[46] = 4;table[47] = 5;

	table[48] = 2;table[49] = 3;table[50] = 3;table[51] = 4;table[52] = 3;table[53] = 4;table[54] = 4;table[55] = 5;

	table[56] = 3;table[57] = 4;table[58] = 4;table[59] = 5;table[60] = 4;table[61] = 5;table[62] = 5;table[63] = 6;

	table[64] = 1;table[65] = 2;table[66] = 2;table[67] = 3;table[68] = 2;table[69] = 3;table[70] = 3;table[71] = 4;

	table[72] = 2;table[73] = 3;table[74] = 3;table[75] = 4;table[76] = 3;table[77] = 4;table[78] = 4;table[79] = 5;

	table[80] = 2;table[81] = 3;table[82] = 3;table[83] = 4;table[84] = 3;table[85] = 4;table[86] = 4;table[87] = 5;

	table[88] = 3;table[89] = 4;table[90] = 4;table[91] = 5;table[92] = 4;table[93] = 5;table[94] = 5;table[95] = 6;

	table[96] = 2;table[97] = 3;table[98] = 3;table[99] = 4;table[100] = 3;table[101] = 4;table[102] = 4;table[103] = 5;

	table[104] = 3;table[105] = 4;table[106] = 4;table[107] = 5;table[108] = 4;table[109] = 5;table[110] = 5;table[111] = 6;

	table[112] = 3;table[113] = 4;table[114] = 4;table[115] = 5;table[116] = 4;table[117] = 5;table[118] = 5;table[119] = 6;

	table[120] = 4;table[121] = 5;table[122] = 5;table[123] = 6;table[124] = 5;table[125] = 6;table[126] = 6;table[127] = 7;

	table[128] = 1;table[129] = 2;table[130] = 2;table[131] = 3;table[132] = 2;table[133] = 3;table[134] = 3;table[135] = 4;

	table[136] = 2;table[137] = 3;table[138] = 3;table[139] = 4;table[140] = 3;table[141] = 4;table[142] = 4;table[143] = 5;

	table[144] = 2;table[145] = 3;table[146] = 3;table[147] = 4;table[148] = 3;table[149] = 4;table[150] = 4;table[151] = 5;

	table[152] = 3;table[153] = 4;table[154] = 4;table[155] = 5;table[156] = 4;table[157] = 5;table[158] = 5;table[159] = 6;

	table[160] = 2;table[161] = 3;table[162] = 3;table[163] = 4;table[164] = 3;table[165] = 4;table[166] = 4;table[167] = 5;

	table[168] = 3;table[169] = 4;table[170] = 4;table[171] = 5;table[172] = 4;table[173] = 5;table[174] = 5;table[175] = 6;

	table[176] = 3;table[177] = 4;table[178] = 4;table[179] = 5;table[180] = 4;table[181] = 5;table[182] = 5;table[183] = 6;

	table[184] = 4;table[185] = 5;table[186] = 5;table[187] = 6;table[188] = 5;table[189] = 6;table[190] = 6;table[191] = 7;

	table[192] = 2;table[193] = 3;table[194] = 3;table[195] = 4;table[196] = 3;table[197] = 4;table[198] = 4;table[199] = 5;

	table[200] = 3;table[201] = 4;table[202] = 4;table[203] = 5;table[204] = 4;table[205] = 5;table[206] = 5;table[207] = 6;

	table[208] = 3;table[209] = 4;table[210] = 4;table[211] = 5;table[212] = 4;table[213] = 5;table[214] = 5;table[215] = 6;

	table[216] = 4;table[217] = 5;table[218] = 5;table[219] = 6;table[220] = 5;table[221] = 6;table[222] = 6;table[223] = 7;

	table[224] = 3;table[225] = 4;table[226] = 4;table[227] = 5;table[228] = 4;table[229] = 5;table[230] = 5;table[231] = 6;

	table[232] = 4;table[233] = 5;table[234] = 5;table[235] = 6;table[236] = 5;table[237] = 6;table[238] = 6;table[239] = 7;

	table[240] = 4;table[241] = 5;table[242] = 5;table[243] = 6;table[244] = 5;table[245] = 6;table[246] = 6;table[247] = 7;

	table[248] = 5;table[249] = 6;table[250] = 6;table[251] = 7;table[252] = 6;table[253] = 7;table[254] = 7;table[255] = 8;

	//std::cout << "task_num = " << task_num_local << std::endl;
/*
	READ_TASK_LEFT: for(i = 0; i < task_num_local; i++){
		task_buffer_left[i] = task_left[i];
	}

	READ_TASK_RIGHT: for(j = 0; j < task_num_local; j++){
		task_buffer_right[j] = task_right[j];
	}
*/
	//std::cout << "task loaded" << std::endl;
	//std::cout << "writing to " << out_pos << std::endl;

	MERGE: for(i = 0,last_left=END_FLAG,last_right=END_FLAG; i < task_num_local; i++){
		id_left = task_left[i];
		id_right = task_right[i];

		adj_begin_left = heavy_offset_left[id_left];
		adj_begin_right = heavy_offset_right[id_right];

		adj_end_left = heavy_offset_left[id_left + 1];
		adj_end_right = heavy_offset_right[id_right + 1];

		adj_size_left = adj_end_left - adj_begin_left;
		adj_size_right = adj_end_right - adj_begin_right;

		//std::cout << "dealing with "<< i << "th task" << std::endl;
		//std::cout << id_left << " : " << adj_begin_left << " - " << adj_end_left << std::endl;
		//std::cout << id_right << " : " << adj_begin_right << " - " << adj_end_right << std::endl;

		READ_ADJ_LEFT: for(li = 0; li < adj_size_left && id_left != last_left; li++){
			bid_buffer_left[li] = heavy_bid_left[adj_begin_left + li];
			value_buffer_left[li] = heavy_value_left[adj_begin_left + li];
		}

		READ_ADJ_RIGHT: for(ri = 0; ri < adj_size_right && id_right != last_right; ri++){
			bid_buffer_right[ri] = heavy_bid_right[adj_begin_right + ri];
			value_buffer_right[ri] = heavy_value_right[adj_begin_right + ri];
		}
		//std::cout << "adj read" << std::endl;
		last_left = id_left;
		last_right = id_right;

		DO_MERGE: for(li = 0, ri = 0; li < adj_size_left && ri < adj_size_right;){
			bid_left = bid_buffer_left[li];
			bid_right = bid_buffer_right[ri];
			is_equal = (bid_left==bid_right);
			is_greater = (bid_left > bid_right);
			//is_less = (bid_left < bid_right);

			if(is_equal){
				value_left = value_buffer_left[li];
				value_right = value_buffer_right[ri];
				value_res = value_left & value_right;
				res += table[value_res &0xff] +
						table[(value_res >>8) &0xff] +
						table[(value_res >>16) &0xff] +
						table[(value_res >>24) &0xff] ;
				//std::cout << "res updated : " << res << std::endl;
				li++;
				ri++;
			}
			else if(is_greater){
				ri++;

			}
			else{
				li++;
			}
		}
	}
	output[0] = res;
	return;
}
}
