#include "kernels.h"
#include <iostream>
extern "C"{
void LLTriCount(
		RAW_ID_TYPE light_offset_left[LIGHT_OFFSET_LEN],
		RAW_ID_TYPE light_adj_left[LIGHT_ADJ_LEN],

		RAW_ID_TYPE light_offset_right[LIGHT_OFFSET_LEN],
		RAW_ID_TYPE light_adj_right[LIGHT_ADJ_LEN],

		RAW_ID_TYPE task_left[TASK_LEN],
		RAW_ID_TYPE task_right[TASK_LEN],

		int task_num,
		unsigned long long output[OUT_LEN]
		)
{
#pragma HLS INTERFACE m_axi port = light_offset_left offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = light_adj_left offset = slave bundle = gmem1
#pragma HLS INTERFACE m_axi port = light_offset_right offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = light_adj_right offset = slave bundle = gmem3
#pragma HLS INTERFACE m_axi port = task_left offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = task_right offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = output offset = slave bundle = gmem2

#pragma HLS INTERFACE s_axilite port = light_offset_left bundle = control
#pragma HLS INTERFACE s_axilite port = light_adj_left bundle = control
#pragma HLS INTERFACE s_axilite port = light_offset_right bundle = control
#pragma HLS INTERFACE s_axilite port = light_adj_right bundle = control
#pragma HLS INTERFACE s_axilite port = task_left bundle = control
#pragma HLS INTERFACE s_axilite port = task_right bundle = control
#pragma HLS INTERFACE s_axilite port = task_num bundle = control
#pragma HLS INTERFACE s_axilite port = out_pos bundle = control
#pragma HLS INTERFACE s_axilite port = output bundle = control

	static RAW_ID_TYPE task_buffer_left[TASK_NUM_UP_BOUND];
	static RAW_ID_TYPE task_buffer_right[TASK_NUM_UP_BOUND];

	static RAW_ID_TYPE adj_buffer_left[ADJ_NUM_UP_BOUND];
	static RAW_ID_TYPE adj_buffer_right[ADJ_NUM_UP_BOUND];

	int task_num_local = 0;
	RAW_ID_TYPE i,j,li,ri,k,til,tir;

	unsigned long long res = 0;

	//adj_paras
	RAW_ID_TYPE adj_size_left,adj_size_right;

	//merge paras
	RAW_ID_TYPE value_left,value_right;
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

	//std::cout << "ll task_num = " << task_num_local << std::endl;
	for(k = 0; k < task_num_local ; k++){

		READ_TASK_LEFT: for(i = 0; i < 4096; i++){
			task_buffer_left[i] = task_left[til];
			til++;
		}

		READ_TASK_RIGHT: for(j = 0; j < 4096; j++){
			task_buffer_right[j] = task_right[tir];
			tir++;
		}

		MERGE: for(i = 0,last_left=END_FLAG,last_right=END_FLAG; i < 4096; i++){

			id_left = task_buffer_left[i];
			id_right = task_buffer_right[i];

			//std::cout << "dealing with " << id_left << " & " << id_right << std::endl;

			adj_begin_left = light_offset_left[id_left];
			adj_begin_right = light_offset_right[id_right];

			adj_end_left = light_offset_left[id_left + 1];
			adj_end_right = light_offset_right[id_right + 1];

			adj_size_left = adj_end_left - adj_begin_left;
			adj_size_right = adj_end_right - adj_begin_right;

			READ_ADJ_LEFT: for(li = 0; li < adj_size_left && id_left!=last_left; li++){
				adj_buffer_left[li] = light_adj_left[adj_begin_left + li];
			}

			READ_ADJ_RIGHT: for(ri = 0; ri < adj_size_right && id_right!=last_right; ri++){
				adj_buffer_right[ri] = light_adj_right[adj_begin_right + ri];
			}

			value_left = adj_buffer_left[0];
			value_right = adj_buffer_right[0];
			last_left = id_left;
			last_right = id_right;

			DO_MERGE: for(li = 0, ri = 0; li < adj_size_left && ri < adj_size_right;){
				value_left = adj_buffer_left[li];
				value_right = adj_buffer_right[ri];
				is_equal = (value_left==value_right);
				is_greater = (value_left > value_right);
				//is_less = (value_left < value_right);

				if(is_equal){
					res++;
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
	return;
}

void LLTriCount_CPU(
		RAW_ID_TYPE light_offset_left[LIGHT_OFFSET_LEN],
		RAW_ID_TYPE light_adj_left[LIGHT_ADJ_LEN],

		RAW_ID_TYPE light_offset_right[LIGHT_OFFSET_LEN],
		RAW_ID_TYPE light_adj_right[LIGHT_ADJ_LEN],

		RAW_ID_TYPE task_left[TASK_LEN],
		RAW_ID_TYPE task_right[TASK_LEN],

		int task_num,
		unsigned long long output[OUT_LEN]
		)
{
	//static RAW_ID_TYPE task_buffer_left[TASK_NUM_UP_BOUND+10000];
	//static RAW_ID_TYPE task_buffer_right[TASK_NUM_UP_BOUND+10000];

	static RAW_ID_TYPE adj_buffer_left[ADJ_NUM_UP_BOUND];
	static RAW_ID_TYPE adj_buffer_right[ADJ_NUM_UP_BOUND];

	int task_num_local = 0;
	RAW_ID_TYPE i,j,li,ri,k;

	unsigned long long res = 0;

	//adj_paras
	RAW_ID_TYPE adj_size_left,adj_size_right;

	//merge paras
	RAW_ID_TYPE value_left,value_right;
	bool is_equal;
	bool is_greater;


	//offset paras
	RAW_ID_TYPE id_left,id_right,last_left,last_right;
	RAW_ID_TYPE adj_begin_left,adj_begin_right;
	RAW_ID_TYPE adj_end_left,adj_end_right;

	//initial
	task_num_local = task_num;
	res = 0;

	//std::cout << "ll task_num = " << task_num_local << std::endl;
	/*
	READ_TASK_LEFT: for(i = 0; i < task_num_local; i++){
		task_buffer_left[i] = task_left[i];
	}

	READ_TASK_RIGHT: for(j = 0; j < task_num_local; j++){
		task_buffer_right[j] = task_right[j];
	}
	*/

	MERGE: for(i = 0,last_left=END_FLAG,last_right=END_FLAG; i < task_num_local; i++){

		id_left = task_left[i];
		id_right = task_right[i];

		//std::cout << "dealing with " << id_left << " & " << id_right << std::endl;

		adj_begin_left = light_offset_left[id_left];
		adj_begin_right = light_offset_right[id_right];

		adj_end_left = light_offset_left[id_left + 1];
		adj_end_right = light_offset_right[id_right + 1];

		adj_size_left = adj_end_left - adj_begin_left;
		adj_size_right = adj_end_right - adj_begin_right;

		READ_ADJ_LEFT: for(li = 0; li < adj_size_left && id_left!=last_left; li++){
			adj_buffer_left[li] = light_adj_left[adj_begin_left + li];
		}

		READ_ADJ_RIGHT: for(ri = 0; ri < adj_size_right && id_right!=last_right; ri++){
			adj_buffer_right[ri] = light_adj_right[adj_begin_right + ri];
		}

		value_left = adj_buffer_left[0];
		value_right = adj_buffer_right[0];
		last_left = id_left;
		last_right = id_right;

		DO_MERGE: for(li = 0, ri = 0; li < adj_size_left && ri < adj_size_right;){
			value_left = adj_buffer_left[li];
			value_right = adj_buffer_right[ri];
			is_equal = (value_left==value_right);
			is_greater = (value_left > value_right);
			//is_less = (value_left < value_right);

			if(is_equal){
				res++;
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
