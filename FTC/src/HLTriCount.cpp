#include "kernels.h"
#include <iostream>
extern "C"{
void HLTriCount(
		RAW_ID_TYPE light_offset_left[LIGHT_OFFSET_LEN],
		RAW_ID_TYPE light_adj_left[LIGHT_ADJ_LEN],

		RAW_ID_TYPE heavy_offset_right[LIGHT_OFFSET_LEN],
		BID_TYPE heavy_bid_right[LIGHT_ADJ_LEN],
		VID_TYPE heavy_value_right[LIGHT_ADJ_LEN],

		RAW_ID_TYPE task_left[TASK_LEN],
		RAW_ID_TYPE task_right[TASK_LEN],

		int task_num,
		unsigned long long output[OUT_LEN]
		)
{
#pragma HLS INTERFACE m_axi port = light_offset_left offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = light_adj_left offset = slave bundle = gmem1
#pragma HLS INTERFACE m_axi port = heavy_offset_right offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = heavy_bid_right offset = slave bundle = gmem3
#pragma HLS INTERFACE m_axi port = heavy_value_right offset = slave bundle = gmem4
#pragma HLS INTERFACE m_axi port = task_left offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = task_right offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = output offset = slave bundle = gmem2

#pragma HLS INTERFACE s_axilite port = light_offset_left bundle = control
#pragma HLS INTERFACE s_axilite port = light_adj_left bundle = control
#pragma HLS INTERFACE s_axilite port = heavy_offset_right bundle = control
#pragma HLS INTERFACE s_axilite port = heavy_bid_right bundle = control
#pragma HLS INTERFACE s_axilite port = heavy_value_right bundle = control
#pragma HLS INTERFACE s_axilite port = task_left bundle = control
#pragma HLS INTERFACE s_axilite port = task_right bundle = control
#pragma HLS INTERFACE s_axilite port = task_num bundle = control
#pragma HLS INTERFACE s_axilite port = output bundle = control

	static RAW_ID_TYPE task_buffer_left[TASK_NUM_UP_BOUND];
	static RAW_ID_TYPE task_buffer_right[TASK_NUM_UP_BOUND];

	static RAW_ID_TYPE adj_buffer_left[ADJ_NUM_UP_BOUND];
	static RAW_ID_TYPE bid_buffer_left[ADJ_NUM_UP_BOUND];
	static VID_TYPE value_buffer_left[ADJ_NUM_UP_BOUND];

	static BID_TYPE bid_buffer_right[ADJ_NUM_UP_BOUND];

	static VID_TYPE value_buffer_right[ADJ_NUM_UP_BOUND];

	int task_num_local = 0;
	RAW_ID_TYPE i,j,li,ri,k,til,tir;

	unsigned long long res = 0;

	//adj_paras
	int adj_size_left,adj_size_right;

	//merge paras
	bool is_found;
	int left,right,mid;
	bool is_equal;
	bool is_greater;


	//offset paras
	RAW_ID_TYPE id_left,id_right,last_left,last_right;
	RAW_ID_TYPE adj_begin_left,adj_begin_right;
	RAW_ID_TYPE adj_end_left,adj_end_right;

	//initial
	task_num_local = task_num;
	res = 0;
	tir = 0;
	til = 0;

	//std::cout << "hl task_num = " << task_num_local << std::endl;

	//std::cout << "task_num = " << task_num_local << std::endl;
	for(k = 0; k < task_num_local; k++){

		READ_TASK_LEFT: for(i = 0; i < 4096; i++){
			task_buffer_left[i] = task_left[til];
			til++;
		}

		READ_TASK_RIGHT: for(j = 0; j < 4096; j++){
			task_buffer_right[j] = task_right[tir];
			tir++;
		}

		//std::cout << "task_buffer loaded" << std::endl;

		MERGE: for(i = 0,last_left=END_FLAG,last_right=END_FLAG; i < 4096; i++){
			id_left = task_buffer_left[i];
			id_right = task_buffer_right[i];

			adj_begin_left = light_offset_left[id_left];
			adj_begin_right = heavy_offset_right[id_right];

			adj_end_left = light_offset_left[id_left + 1];
			adj_end_right = heavy_offset_right[id_right + 1];

			adj_size_left = adj_end_left - adj_begin_left;
			adj_size_right = adj_end_right - adj_begin_right;

			//std::cout << id_left << " : " << adj_begin_left << " - " << adj_end_left << std::endl;
			//std::cout << id_right << " : " << adj_begin_right << " - " << adj_end_right << std::endl;

			READ_ADJ_LEFT: for(li = 0; li < adj_size_left && id_left!=last_left; li++){
				adj_buffer_left[li] = light_adj_left[adj_begin_left + li];
			}

			READ_BV_RIGHT: for(ri = 0; ri < adj_size_right && id_right!=last_right; ri++){
				bid_buffer_right[ri] = heavy_bid_right[adj_begin_right + ri];
				value_buffer_right[ri] = heavy_value_right[adj_begin_right + ri];
			}

			READ_BV_LEFT: for(li = 0; li < adj_size_left && id_left!=last_left; li++){
				bid_buffer_left[li] = adj_buffer_left[li] / 32;
				value_buffer_left[li] = (1 << (32 - adj_buffer_left[li]%32 - 1));
			}

			last_left = id_left;
			last_right = id_right;

			//std::cout << "size " <<adj_size_left<<" "<<adj_size_right << std::endl;
			DO_BINARY_SEARCH_OUTTER:for(li = 0; li < adj_size_left; li++){
				DO_BINARY_SEARCH_INNER:for(left = 0, right = adj_size_right - 1; left <= right ; ){
					mid = left + ((right - left) >> 1);
					//std::cout << bid_buffer_left[li] << std::endl;
					//std::cout << bid_buffer_right[mid] << std::endl;
					is_equal = (bid_buffer_left[li] == bid_buffer_right[mid]);
					is_greater = (bid_buffer_left[li] > bid_buffer_right[mid]);
					//is_less = (bid_buffer_left[li] < bid_buffer_right[mid]);
					if(is_equal){
						//std::cout << "got it !" << std::endl;
						is_found = ((value_buffer_left[li]&value_buffer_right[mid]) == value_buffer_left[li]);
						res += is_found;
						left = right+1;
					}else if(is_greater){
						left = mid + 1;
					}else{
						right = mid - 1;
					}
				}
			}
		}
	}


	output[0] = res;
	return;
}

void HLTriCount_CPU(
		RAW_ID_TYPE light_offset_left[LIGHT_OFFSET_LEN],
		RAW_ID_TYPE light_adj_left[LIGHT_ADJ_LEN],

		RAW_ID_TYPE heavy_offset_right[LIGHT_OFFSET_LEN],
		BID_TYPE heavy_bid_right[LIGHT_ADJ_LEN],
		VID_TYPE heavy_value_right[LIGHT_ADJ_LEN],

		RAW_ID_TYPE task_left[TASK_LEN],
		RAW_ID_TYPE task_right[TASK_LEN],

		int task_num,
		unsigned long long output[OUT_LEN]
		)
{
	//static RAW_ID_TYPE task_buffer_left[TASK_NUM_UP_BOUND+10000];
	//static RAW_ID_TYPE task_buffer_right[TASK_NUM_UP_BOUND+10000];

	static RAW_ID_TYPE adj_buffer_left[ADJ_NUM_UP_BOUND];
	static RAW_ID_TYPE bid_buffer_left[ADJ_NUM_UP_BOUND];
	static VID_TYPE value_buffer_left[ADJ_NUM_UP_BOUND];

	static BID_TYPE bid_buffer_right[ADJ_NUM_UP_BOUND];

	static VID_TYPE value_buffer_right[ADJ_NUM_UP_BOUND];

	int task_num_local = 0;
	RAW_ID_TYPE i,j,li,ri,k;

	unsigned long long res = 0;

	//adj_paras
	int adj_size_left,adj_size_right;

	//merge paras
	bool is_found;
	int left,right,mid;
	bool is_equal;
	bool is_greater;


	//offset paras
	RAW_ID_TYPE id_left,id_right,last_left,last_right;
	RAW_ID_TYPE adj_begin_left,adj_begin_right;
	RAW_ID_TYPE adj_end_left,adj_end_right;

	//initial
	task_num_local = task_num;
	res = 0;

	//std::cout << "hl task_num = " << task_num_local << std::endl;

	//std::cout << "task_num = " << task_num_local << std::endl;
	/*
	READ_TASK_LEFT: for(i = 0; i < task_num_local; i++){
		task_buffer_left[i] = task_left[i];
	}

	READ_TASK_RIGHT: for(j = 0; j < task_num_local; j++){
		task_buffer_right[j] = task_right[j];
	}
	*/

	//std::cout << "task_buffer loaded" << std::endl;

	MERGE: for(i = 0,last_left=END_FLAG,last_right=END_FLAG; i < task_num_local; i++){
		id_left = task_left[i];
		id_right = task_right[i];

		adj_begin_left = light_offset_left[id_left];
		adj_begin_right = heavy_offset_right[id_right];

		adj_end_left = light_offset_left[id_left + 1];
		adj_end_right = heavy_offset_right[id_right + 1];

		adj_size_left = adj_end_left - adj_begin_left;
		adj_size_right = adj_end_right - adj_begin_right;

		//std::cout << id_left << " : " << adj_begin_left << " - " << adj_end_left << std::endl;
		//std::cout << id_right << " : " << adj_begin_right << " - " << adj_end_right << std::endl;

		READ_ADJ_LEFT: for(li = 0; li < adj_size_left && id_left!=last_left; li++){
			adj_buffer_left[li] = light_adj_left[adj_begin_left + li];
		}

		READ_BV_RIGHT: for(ri = 0; ri < adj_size_right && id_right!=last_right; ri++){
			bid_buffer_right[ri] = heavy_bid_right[adj_begin_right + ri];
			value_buffer_right[ri] = heavy_value_right[adj_begin_right + ri];
		}

		READ_BV_LEFT: for(li = 0; li < adj_size_left && id_left!=last_left; li++){
			bid_buffer_left[li] = adj_buffer_left[li] / 32;
			value_buffer_left[li] = (1 << (32 - adj_buffer_left[li]%32 - 1));
		}

		last_left = id_left;
		last_right = id_right;

		//std::cout << "size " <<adj_size_left<<" "<<adj_size_right << std::endl;
		DO_BINARY_SEARCH_OUTTER:for(li = 0; li < adj_size_left; li++){
			DO_BINARY_SEARCH_INNER:for(left = 0, right = adj_size_right - 1; left <= right ; ){
				mid = left + ((right - left) >> 1);
				//std::cout << bid_buffer_left[li] << std::endl;
				//std::cout << bid_buffer_right[mid] << std::endl;
				is_equal = (bid_buffer_left[li] == bid_buffer_right[mid]);
				is_greater = (bid_buffer_left[li] > bid_buffer_right[mid]);
				//is_less = (bid_buffer_left[li] < bid_buffer_right[mid]);
				if(is_equal){
					//std::cout << "got it !" << std::endl;
					is_found = ((value_buffer_left[li]&value_buffer_right[mid]) == value_buffer_left[li]);
					res += is_found;
					left = right+1;
				}else if(is_greater){
					left = mid + 1;
				}else{
					right = mid - 1;
				}
			}
		}
	}


	output[0] = res;
	return;
}
}
