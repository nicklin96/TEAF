typedef unsigned int RAW_ID_TYPE;
typedef unsigned int BID_TYPE;
typedef unsigned int VID_TYPE;

#ifndef LIGHT_OFFSET_LEN
#define LIGHT_OFFSET_LEN 50000000
#endif

#ifndef LIGHT_ADJ_LEN
#define LIGHT_ADJ_LEN 1500000000
#endif

#ifndef END_FLAG
#define END_FLAG 4294967290
#endif

#ifndef HEAVY_OFFSET_LEN
#define HEAVY_OFFSET_LEN 1500000000
#endif

#ifndef HEAVY_ADJ_LEN
#define HEAVY_ADJ_LEN 250000000
#endif

#ifndef TASK_NUM_UP_BOUND
#define TASK_NUM_UP_BOUND 4096
#endif

#ifndef TASK_LEN
#define TASK_LEN 250000000
#endif

#ifndef ADJ_NUM_UP_BOUND
#define ADJ_NUM_UP_BOUND 25000
#endif

#ifndef OUT_LEN
#define OUT_LEN 64
#endif

#ifndef FLATTEN_NUM
#define FLATTEN_NUM 4
#endif

#ifndef KERNELS_H
#define KERNELS_H
extern "C"{
void HLTriCount(
		RAW_ID_TYPE light_offset_left[LIGHT_OFFSET_LEN],
		RAW_ID_TYPE light_adj_left[LIGHT_ADJ_LEN],

		RAW_ID_TYPE heavy_offset_right[LIGHT_OFFSET_LEN],
		BID_TYPE heavy_bid_right[LIGHT_ADJ_LEN],
		VID_TYPE heavy_value_right[LIGHT_ADJ_LEN],

		RAW_ID_TYPE task_left[TASK_NUM_UP_BOUND],
		RAW_ID_TYPE task_right[TASK_NUM_UP_BOUND],

		int task_num,
		unsigned long long output[OUT_LEN]
		);

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
		unsigned long long output[OUT_LEN]);

void LLTriCount(
		RAW_ID_TYPE light_offset_left[LIGHT_OFFSET_LEN],
		RAW_ID_TYPE light_adj_left[LIGHT_ADJ_LEN],

		RAW_ID_TYPE light_offset_right[LIGHT_OFFSET_LEN],
		RAW_ID_TYPE light_adj_right[LIGHT_ADJ_LEN],

		RAW_ID_TYPE task_left[TASK_NUM_UP_BOUND],
		RAW_ID_TYPE task_right[TASK_NUM_UP_BOUND],

		int task_num,
		unsigned long long output[OUT_LEN]
		);


}
#endif




