#include<iostream>
#include<map>
#include<vector>

#include "graph.h"

using namespace std;

/*
void testLL(string path){
	//allocate memory
	RAW_ID_TYPE* light_offset_left = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	RAW_ID_TYPE* light_offset_right = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];

	RAW_ID_TYPE* light_adj_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];
	RAW_ID_TYPE* light_adj_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* task_left = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	RAW_ID_TYPE* task_right = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];

	unsigned long long* output = new unsigned long long[OUT_LEN];



	cout << "*** Construct Light CSR ***" << endl;
	Graph g;
	g.readGraph(path);
	LightCSR csr(g);
	//csr.printCSR();
	cout << "*** Light CSR Construction Finished ***" << endl << endl;

	for(int i = 0; i < csr.offset.size(); i++){
		light_offset_left[i] = csr.offset[i];
		light_offset_right[i] = csr.offset[i];
	}

	for(int i = 0; i < csr.adj.size(); i++){
		light_adj_left[i] = csr.adj[i];
		light_adj_right[i] = csr.adj[i];
	}

	int task_num = 0;
	for(RAW_ID_TYPE i = 0; i < csr.vertex_num; i++){
		int adj_size = light_offset_left[i+1] - light_offset_right[i];
		for(RAW_ID_TYPE j = 0; j < adj_size; j++){
			RAW_ID_TYPE right = light_adj_left[csr.offset[i] + j];
			int adj_size_right = light_offset_left[right+1] - light_offset_right[right];
			if(adj_size_right > 0){
				task_left[task_num] = i;
				task_right[task_num] = right;
				task_num++;
			}
		}
	}

	//print tasks
	cout << "***T asks List ***" << endl;
	for(int i = 0; i < task_num ; i++){
		cout << task_left[i] << " " << task_right[i] << endl;
	}
	cout << "*** Tasks List End ***"<< endl << endl;

	cout << "*** Run Light-Light Merge ***" << endl;
	LLTriCount(
			light_offset_left,light_adj_left,
			light_offset_right,light_adj_right,
			task_left,task_right,
			task_num,
			output
			);
	cout << "TC = " << output[0] << endl;
	cout << "*** Finish Light-Light Merge ***" << endl;

	// release memory
	delete[] light_offset_left;
	delete[] light_offset_right;
	delete[] light_adj_left;
	delete[] light_adj_right;
	delete[] task_left;
	delete[] task_right;
	return;
}

void testHH(string path){
	//allocate memory
	RAW_ID_TYPE* heavy_offset_left = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	RAW_ID_TYPE* heavy_offset_right = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];

	BID_TYPE* heavy_bid_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];
	BID_TYPE* heavy_bid_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	VID_TYPE* heavy_value_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];
	VID_TYPE* heavy_value_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* task_left = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	RAW_ID_TYPE* task_right = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];

	unsigned long long* output = new unsigned long long[2];
	output[0] = output[1] = 0;

	cout << "*** Construct Heavy CSR ***" << endl;
	Graph g;
	g.readGraph(path);
	HeavyCSR csr(g);
	//csr.printCSR();
	cout << "*** Heavy CSR Construction Finished ***" << endl << endl;

	for(int i = 0; i < csr.offset.size(); i++){
		heavy_offset_left[i] = csr.offset[i];
		heavy_offset_right[i] = csr.offset[i];
	}

	for(int i = 0; i < csr.bid.size(); i++){
		heavy_bid_left[i] = csr.bid[i];
		heavy_bid_right[i] = csr.bid[i];
	}

	for(int i = 0; i < csr.bid.size(); i++){
		heavy_value_left[i] = csr.value[i];
		heavy_value_right[i] = csr.value[i];
	}

	int task_num = 0;
	for(RAW_ID_TYPE i = 0; i < csr.vertex_num; i++){
		for(auto item : g.edges[i]){
			int adj_size_right = heavy_offset_left[item+1] - heavy_offset_right[item];
			if(adj_size_right > 0){
				task_left[task_num] = i;
				task_right[task_num] = item;
				task_num++;
			}
		}
	}

	//print tasks
	cout << "*** Tasks List ***" << endl;
	for(int i = 0; i < task_num ; i++){
		cout << task_left[i] << " " << task_right[i] << endl;
	}
	cout << "*** Tasks List End ***"<< endl << endl;

	cout << "*** Run Heavy-Heavy Merge ***" << endl;
	HHTriCount(
			heavy_offset_left,heavy_bid_left,heavy_value_left,
			heavy_offset_right,heavy_bid_right,heavy_value_right,
			task_left,task_right,
			task_num,
			output
			);
	cout << "TC = " << output[0] << endl;
	cout << "*** Finish Heavy-Heavy Merge ***" << endl;

	// release memory
	delete[] heavy_offset_left;
	delete[] heavy_offset_right;
	delete[] heavy_bid_left;
	delete[] heavy_bid_right;
	delete[] heavy_value_left;
	delete[] heavy_value_right;
	delete[] task_left;
	delete[] task_right;
	return;
}

void testHL(string path){
	//allocate memory
	RAW_ID_TYPE* light_offset_left = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	RAW_ID_TYPE* light_adj_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* light_offset_right = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	RAW_ID_TYPE* light_adj_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* heavy_offset_left = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	BID_TYPE* heavy_bid_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];
	VID_TYPE* heavy_value_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* heavy_offset_right = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	BID_TYPE* heavy_bid_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];
	VID_TYPE* heavy_value_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* task_left = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	RAW_ID_TYPE* task_right = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];

	unsigned long long* output = new unsigned long long[2];
	output[0] = output[1] = 0;
	cout << "*** Split Graph ***" << endl;
	Graph g;
	g.readGraph(path);
	Graph heavy,light;
	vector<bool> is_heavy;
	map<RAW_ID_TYPE,RAW_ID_TYPE> id_ori2light;
	map<RAW_ID_TYPE,RAW_ID_TYPE> id_ori2heavy;
	g.splitGraph(light,heavy,is_heavy,id_ori2light,id_ori2heavy,2);
	cout << "*** Split Graph Finished *** " << endl << endl;

	cout << "*** Construct Heavy CSR ***" << endl;
	HeavyCSR hcsr(heavy);
	hcsr.printCSR();
	cout << "*** Heavy CSR Construction Finished ***" << endl << endl;

	cout << "*** Construct Light CSR ***" << endl;
	LightCSR lcsr(light);
	lcsr.printCSR();
	for(auto item : id_ori2light){
		cout << item.first << " " << item.second << endl;
	}
	cout << "*** Light CSR Construction Finished ***" << endl << endl;

	for(int i = 0; i < hcsr.offset.size(); i++){
		heavy_offset_left[i] = hcsr.offset[i];
		heavy_offset_right[i] = hcsr.offset[i];
	}

	for(int i = 0; i < hcsr.bid.size(); i++){
		heavy_bid_left[i] = hcsr.bid[i];
		heavy_bid_right[i] = hcsr.bid[i];
	}

	for(int i = 0; i < hcsr.bid.size(); i++){
		heavy_value_left[i] = hcsr.value[i];
		heavy_value_right[i] = hcsr.value[i];
	}

	for(int i = 0; i < lcsr.offset.size(); i++){
		light_offset_left[i] = lcsr.offset[i];
		light_offset_right[i] = lcsr.offset[i];
	}

	for(int i = 0; i < lcsr.adj.size(); i++){
		light_adj_left[i] = lcsr.adj[i];
		light_adj_right[i] = lcsr.adj[i];
	}

	int task_num = 0;
	for(RAW_ID_TYPE i = 0; i < g.vertex_num; i++){
		RAW_ID_TYPE left = g.degrees[i].id;
		for(auto item : g.edges[left]){
			if((is_heavy[left]&&!is_heavy[item]) || (!is_heavy[left]&&is_heavy[item])){
				if(is_heavy[item]){
					int adj_size_right = heavy_offset_left[id_ori2heavy[item]+1] - heavy_offset_right[id_ori2heavy[item]];
					if(adj_size_right > 0){
						task_left[task_num] = left;
						task_right[task_num] = item;
						task_num++;
					}
				}else{
					int adj_size_right = light_offset_left[id_ori2light[item]+1] - light_offset_right[id_ori2light[item]];
					if(adj_size_right > 0){
						task_left[task_num] = item;
						task_right[task_num] = left;
						task_num++;
					}
				}
			}

		}
	}

	//print tasks
	cout << "*** Tasks List ***" << endl;
	for(int i = 0; i < task_num ; i++){
		cout << task_left[i] << " " << task_right[i] << "->" << id_ori2light[task_left[i]] << " " << id_ori2heavy[task_right[i]] << endl;
		task_left[i] = id_ori2light[task_left[i]];
		task_right[i] = id_ori2heavy[task_right[i]];
	}
	cout << "*** Tasks List End ***"<< endl << endl;

	cout << "*** Run Heavy-Light Merge ***" << endl;
	HLTriCount(
			light_offset_left,light_adj_left,
			heavy_offset_right,heavy_bid_right,heavy_value_right,
			task_left,task_right,
			task_num,
			output
			);
	cout << "TC = " << output[0] << endl;
	cout << "*** Finish Heavy-Light Merge ***" << endl;

	// release memory
	delete[] light_offset_left;
	delete[] light_offset_right;
	delete[] light_adj_left;
	delete[] light_adj_right;
	delete[] heavy_offset_left;
	delete[] heavy_offset_right;
	delete[] heavy_bid_left;
	delete[] heavy_bid_right;
	delete[] heavy_value_left;
	delete[] heavy_value_right;
	delete[] task_left;
	delete[] task_right;
	return;
}

void testGraph(string path){
	Graph g;
	g.readCompressedGraph(path);

	//LightCSR csr(g);
	//csr.printCSR();
	//HeavyCSR hcsr(g);
	//hcsr.printCSR();


	Graph heavy,light;
	vector<bool> is_heavy;
	map<RAW_ID_TYPE,RAW_ID_TYPE> id_ori2light;
	map<RAW_ID_TYPE,RAW_ID_TYPE> id_ori2heavy;
	g.splitGraph(light,heavy,is_heavy,id_ori2light,id_ori2heavy,2);
	std::cout << "Light Mapping" << std::endl;
	for(auto item : id_ori2light){
		std::cout << item.first << " : " << item.second << std::endl;
	}

	std::cout << "Heavy Mapping" << std::endl;
	for(auto item : id_ori2heavy){
		std::cout << item.first << " : " << item.second << std::endl;
	}


}

void testTC(string path, char mode){
	//allocate memory
	RAW_ID_TYPE* light_offset_left = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	RAW_ID_TYPE* light_adj_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* light_offset_right = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	RAW_ID_TYPE* light_adj_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* heavy_offset_left = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	BID_TYPE* heavy_bid_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];
	VID_TYPE* heavy_value_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* heavy_offset_right = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	BID_TYPE* heavy_bid_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];
	VID_TYPE* heavy_value_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* ll_task_left = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	RAW_ID_TYPE* ll_task_right = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];

	RAW_ID_TYPE* hl_task_left = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	RAW_ID_TYPE* hl_task_right = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];

	RAW_ID_TYPE* hh_task_left = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	RAW_ID_TYPE* hh_task_right = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];

	unsigned long long* ll_output = new unsigned long long[OUT_LEN];
	unsigned long long* hl_output = new unsigned long long[OUT_LEN];
	unsigned long long* hh_output = new unsigned long long[OUT_LEN];
	cout << "*** Split Graph ***" << endl;
	Graph g;
	if(mode=='n'){
		g.readGraph(path);
	}else if(mode=='c'){
		g.readCompressedGraph(path);
	}else{
		cout << "no such mode!"<<endl;
		return;
	}

	Graph heavy,light;
	vector<bool> is_heavy;
	map<RAW_ID_TYPE,RAW_ID_TYPE> id_ori2light;
	map<RAW_ID_TYPE,RAW_ID_TYPE> id_ori2heavy;
	int heavy_num = (int)(0.05 * g.vertex_num);
	g.splitGraph(light,heavy,is_heavy,id_ori2light,id_ori2heavy,heavy_num);
	cout << "vertex num of light graph " << light.vertex_num << endl;
	cout << "vertex num of heavy graph " << heavy.vertex_num << endl;
	cout << "*** Split Graph Finished *** " << endl << endl;

	cout << "*** Construct Heavy CSR ***" << endl;
	HeavyCSR hcsr(heavy);
	//hcsr.printCSR();
	cout << "*** Heavy CSR Construction Finished ***" << endl << endl;

	cout << "*** Construct Light CSR ***" << endl;
	LightCSR lcsr(light);
	//lcsr.getAdjGivenVertex(5489070);
	//lcsr.getAdjGivenVertex(4);
	//lcsr.testMergeTwo(26000,4);
	//return;

	//lcsr.printCSR();
	cout << "*** Light CSR Construction Finished ***" << endl << endl;

	for(int i = 0; i < hcsr.offset.size(); i++){
		heavy_offset_left[i] = hcsr.offset[i];
		heavy_offset_right[i] = hcsr.offset[i];
	}

	for(int i = 0; i < hcsr.bid.size(); i++){
		heavy_bid_left[i] = hcsr.bid[i];
		heavy_bid_right[i] = hcsr.bid[i];
	}

	for(int i = 0; i < hcsr.bid.size(); i++){
		heavy_value_left[i] = hcsr.value[i];
		heavy_value_right[i] = hcsr.value[i];
	}

	for(int i = 0; i < lcsr.offset.size(); i++){
		light_offset_left[i] = lcsr.offset[i];
		light_offset_right[i] = lcsr.offset[i];
	}
	//return;


	for(int i = 0; i < lcsr.adj.size(); i++){
		light_adj_left[i] = lcsr.adj[i];
		light_adj_right[i] = lcsr.adj[i];
	}

	cout << "csr loaded" << endl;

	int hh_task_num = 0;
	int hl_task_num = 0;
	int ll_task_num = 0;

	for(RAW_ID_TYPE i = 0; i < g.vertex_num; i++){
		RAW_ID_TYPE left = g.degrees[i].id;
		for(auto item : g.edges[left]){
			if(is_heavy[left]&&is_heavy[item]){
				int adj_size_right = heavy_offset_left[id_ori2heavy[item]+1] - heavy_offset_left[id_ori2heavy[item]];
				if(adj_size_right > 0){
					hh_task_left[hh_task_num] = left;
					hh_task_right[hh_task_num] = item;
					hh_task_num++;
				}
			}
			else if((is_heavy[left]&&!is_heavy[item]) || (!is_heavy[left]&&is_heavy[item])){
				if(is_heavy[item]){
					int adj_size_right = heavy_offset_left[id_ori2heavy[item]+1] - heavy_offset_left[id_ori2heavy[item]];
					if(adj_size_right > 0){
						hl_task_left[hl_task_num] = left;
						hl_task_right[hl_task_num] = item;
						hl_task_num++;
					}
				}else{
					int adj_size_right = light_offset_left[id_ori2light[item]+1] - light_offset_left[id_ori2light[item]];
					if(adj_size_right > 0){
						hl_task_left[hl_task_num] = item;
						hl_task_right[hl_task_num] = left;
						hl_task_num++;
					}
				}
			}else{
				int adj_size_right = light_offset_left[id_ori2light[item]+1] - light_offset_left[id_ori2light[item]];
				if(adj_size_right > 0){
					ll_task_left[ll_task_num] = item;
					ll_task_right[ll_task_num] = left;
					ll_task_num++;
				}

			}

		}
	}

	//print tasks
	cout << "size of LL Task List " << ll_task_num << endl;
	cout << "size of HL Task List " << hl_task_num << endl;
	cout << "size of HH Task List " << hh_task_num << endl;

	cout << "*** Tasks List ***" << endl;
	cout << "LL Task List:" << endl;
	for(int i = 0; i < ll_task_num ; i++){
		//cout << ll_task_left[i] << " " << ll_task_right[i] << " -> ";
		//cout << id_ori2light[ll_task_left[i]] << " " << id_ori2light[ll_task_right[i]] << endl;
		ll_task_left[i] = id_ori2light[ll_task_left[i]];
		ll_task_right[i] = id_ori2light[ll_task_right[i]];
	}
	//cout << endl;

	cout << "HL Task List:" << endl;
	for(int i = 0; i < hl_task_num ; i++){
		//cout << hl_task_left[i] << " " << hl_task_right[i] << " -> ";
		//cout << id_ori2light[hl_task_left[i]] << " " << id_ori2light[hl_task_right[i]] << endl;
		hl_task_left[i] = id_ori2light[hl_task_left[i]];
		hl_task_right[i] = id_ori2heavy[hl_task_right[i]];
	}
	//cout << endl;

	cout << "HH Task List:" << endl;
	for(int i = 0; i < hh_task_num ; i++){
		//cout << hh_task_left[i] << " " << hh_task_right[i] << " -> ";
		//cout << id_ori2light[hh_task_left[i]] << " " << id_ori2light[hh_task_right[i]] << endl;
		hh_task_left[i] = id_ori2heavy[hh_task_left[i]];
		hh_task_right[i] = id_ori2heavy[hh_task_right[i]];
	}
	//cout << endl;

	cout << "*** Tasks List End ***"<< endl << endl;


	cout << "*** Run Light-Light Merge ***" << endl;
	LLTriCount(
			light_offset_left,light_adj_left,
			light_offset_right,light_adj_right,
			ll_task_left,ll_task_right,
			ll_task_num,
			0,
			ll_output
			);
	cout << "LL TC = " << ll_output[0] << endl;
	cout << "*** Finish Light-Light Merge ***" << endl;

	cout << "*** Run Heavy-Light Merge ***" << endl;
	HLTriCount(
			light_offset_left,light_adj_left,
			heavy_offset_right,heavy_bid_right,heavy_value_right,
			hl_task_left,hl_task_right,
			hl_task_num,
			0,
			hl_output
			);
	cout << "HL TC = " << hl_output[0] << endl;
	cout << "*** Finish Heavy-Light Merge ***" << endl;

	cout << "*** Run Heavy-Heavy Merge ***" << endl;
	HHTriCount(
			heavy_offset_left,heavy_bid_left,heavy_value_left,
			heavy_offset_right,heavy_bid_right,heavy_value_right,
			hh_task_left,hh_task_right,
			hh_task_num,
			0,
			hh_output
			);
	cout << "HH TC = " << hh_output[0] << endl;
	cout << "*** Finish Heavy-Heavy Merge ***" << endl;
	cout << "Final TC = " << hh_output[0] + hl_output[0] + ll_output[0] << endl;

	// release memory
	delete[] light_offset_left;
	delete[] light_offset_right;
	delete[] light_adj_left;
	delete[] light_adj_right;
	delete[] heavy_offset_left;
	delete[] heavy_offset_right;
	delete[] heavy_bid_left;
	delete[] heavy_bid_right;
	delete[] heavy_value_left;
	delete[] heavy_value_right;
	delete[] ll_task_left;
	delete[] ll_task_right;
	delete[] hl_task_left;
	delete[] hl_task_right;
	delete[] hh_task_left;
	delete[] hh_task_right;
	return;
}

void testTaskDiv(string path,char mode){
	//allocate memory
	RAW_ID_TYPE* light_offset_left = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	RAW_ID_TYPE* light_adj_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* light_offset_right = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	RAW_ID_TYPE* light_adj_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* heavy_offset_left = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	BID_TYPE* heavy_bid_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];
	VID_TYPE* heavy_value_left = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	RAW_ID_TYPE* heavy_offset_right = new RAW_ID_TYPE[LIGHT_OFFSET_LEN];
	BID_TYPE* heavy_bid_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];
	VID_TYPE* heavy_value_right = new RAW_ID_TYPE[LIGHT_ADJ_LEN];

	//RAW_ID_TYPE* ll_task_left = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	//RAW_ID_TYPE* ll_task_right = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	vector<RAW_ID_TYPE*> ll_task_split_left;
	vector<RAW_ID_TYPE*> ll_task_split_right;
	int ll_section_count = 0;
	int ll_task_idx = 0;

	//RAW_ID_TYPE* hl_task_left = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	//RAW_ID_TYPE* hl_task_right = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	vector<RAW_ID_TYPE*> hl_task_split_left;
	vector<RAW_ID_TYPE*> hl_task_split_right;
	int hl_section_count = 0;
	int hl_task_idx = 0;

	//RAW_ID_TYPE* hh_task_left = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	//RAW_ID_TYPE* hh_task_right = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
	vector<RAW_ID_TYPE*> hh_task_split_left;
	vector<RAW_ID_TYPE*> hh_task_split_right;
	int hh_section_count = 0;
	int hh_task_idx = 0;

	unsigned long long* ll_output = new unsigned long long[OUT_LEN];
	unsigned long long* hl_output = new unsigned long long[OUT_LEN];
	unsigned long long* hh_output = new unsigned long long[OUT_LEN];

	for(int i = 0; i < OUT_LEN ; i++){
		ll_output[i] = hl_output[i] = hh_output[i] = 0;
	}
	cout << "*** Split Graph ***" << endl;
	Graph g;
	if(mode=='n'){
		g.readGraph(path);
	}else if(mode=='c'){
		g.readCompressedGraph(path);
	}else{
		cout << "no such mode!"<<endl;
		return;
	}

	Graph heavy,light;
	vector<bool> is_heavy;
	map<RAW_ID_TYPE,RAW_ID_TYPE> id_ori2light;
	map<RAW_ID_TYPE,RAW_ID_TYPE> id_ori2heavy;
	int heavy_num = (int)(0.05 * g.vertex_num);
	g.splitGraph(light,heavy,is_heavy,id_ori2light,id_ori2heavy,heavy_num);
	cout << "vertex num of light graph " << light.vertex_num << endl;
	cout << "vertex num of heavy graph " << heavy.vertex_num << endl;
	cout << "*** Split Graph Finished *** " << endl << endl;

	cout << "*** Construct Heavy CSR ***" << endl;
	HeavyCSR hcsr(heavy);
	//hcsr.printCSR();
	cout << "*** Heavy CSR Construction Finished ***" << endl << endl;

	cout << "*** Construct Light CSR ***" << endl;
	LightCSR lcsr(light);
	//lcsr.getAdjGivenVertex(5489070);
	//lcsr.getAdjGivenVertex(4);
	//lcsr.testMergeTwo(26000,4);
	//return;

	//lcsr.printCSR();
	cout << "*** Light CSR Construction Finished ***" << endl << endl;

	for(int i = 0; i < hcsr.offset.size(); i++){
		heavy_offset_left[i] = hcsr.offset[i];
		heavy_offset_right[i] = hcsr.offset[i];
	}

	for(int i = 0; i < hcsr.bid.size(); i++){
		heavy_bid_left[i] = hcsr.bid[i];
		heavy_bid_right[i] = hcsr.bid[i];
	}

	for(int i = 0; i < hcsr.bid.size(); i++){
		heavy_value_left[i] = hcsr.value[i];
		heavy_value_right[i] = hcsr.value[i];
	}

	for(int i = 0; i < lcsr.offset.size(); i++){
		light_offset_left[i] = lcsr.offset[i];
		light_offset_right[i] = lcsr.offset[i];
	}
	//return;


	for(int i = 0; i < lcsr.adj.size(); i++){
		light_adj_left[i] = lcsr.adj[i];
		light_adj_right[i] = lcsr.adj[i];
	}

	cout << "csr loaded" << endl;

	int hh_task_num = 0;
	int hl_task_num = 0;
	int ll_task_num = 0;

	for(RAW_ID_TYPE i = 0; i < g.vertex_num; i++){
		RAW_ID_TYPE left = g.degrees[i].id;
		for(auto item : g.edges[left]){
			if(is_heavy[left]&&is_heavy[item]){
				int adj_size_right = heavy_offset_left[id_ori2heavy[item]+1] - heavy_offset_left[id_ori2heavy[item]];
				if(adj_size_right > 0){
					if(hh_section_count==0 || hh_task_idx >= TASK_NUM_UP_BOUND){
						hh_task_split_left.emplace_back();
						hh_task_split_right.emplace_back();
						hh_section_count++;
						hh_task_idx = 0;
						hh_task_split_left[hh_section_count - 1] = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
						hh_task_split_right[hh_section_count - 1] = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
					}
					//hh_task_left[hh_task_num] = left;
					//hh_task_right[hh_task_num] = item;
					hh_task_split_left[hh_section_count - 1][hh_task_idx] = id_ori2heavy[left];
					hh_task_split_right[hh_section_count - 1][hh_task_idx] = id_ori2heavy[item];
					hh_task_idx++;
					hh_task_num++;
				}
			}
			else if((is_heavy[left]&&!is_heavy[item]) || (!is_heavy[left]&&is_heavy[item])){
				if(is_heavy[item]){
					int adj_size_right = heavy_offset_left[id_ori2heavy[item]+1] - heavy_offset_left[id_ori2heavy[item]];
					if(adj_size_right > 0){
						if(hl_section_count==0 || hl_task_idx >= TASK_NUM_UP_BOUND){
							hl_task_split_left.emplace_back();
							hl_task_split_right.emplace_back();
							hl_section_count++;
							hl_task_idx = 0;
							hl_task_split_left[hl_section_count - 1] = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
							hl_task_split_right[hl_section_count - 1] = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
						}
						//hl_task_left[hl_task_num] = left;
						//hl_task_right[hl_task_num] = item;
						hl_task_split_left[hl_section_count - 1][hl_task_idx] = id_ori2light[left];
						hl_task_split_right[hl_section_count - 1][hl_task_idx] = id_ori2heavy[item];
						hl_task_idx++;
						hl_task_num++;
					}
				}else{
					int adj_size_right = light_offset_left[id_ori2light[item]+1] - light_offset_left[id_ori2light[item]];
					if(adj_size_right > 0){
						if(hl_section_count==0 || hl_task_idx >= TASK_NUM_UP_BOUND){
							hl_task_split_left.emplace_back();
							hl_task_split_right.emplace_back();
							hl_section_count++;
							hl_task_idx = 0;
							hl_task_split_left[hl_section_count - 1] = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
							hl_task_split_right[hl_section_count - 1] = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
						}
						//hl_task_left[hl_task_num] = item;
						//hl_task_right[hl_task_num] = left;
						hl_task_split_left[hl_section_count - 1][hl_task_idx] = id_ori2light[item];
						hl_task_split_right[hl_section_count - 1][hl_task_idx] = id_ori2heavy[left];
						hl_task_idx++;
						hl_task_num++;
					}
				}
			}else{
				int adj_size_right = light_offset_left[id_ori2light[item]+1] - light_offset_left[id_ori2light[item]];
				if(adj_size_right > 0){
					if(ll_section_count==0 || ll_task_idx >= TASK_NUM_UP_BOUND){
						ll_task_split_left.emplace_back();
						ll_task_split_right.emplace_back();
						ll_section_count++;
						ll_task_idx = 0;
						ll_task_split_left[ll_section_count - 1] = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
						ll_task_split_right[ll_section_count - 1] = new RAW_ID_TYPE[TASK_NUM_UP_BOUND];
					}
					//ll_task_left[ll_task_num] = item;
					//ll_task_right[ll_task_num] = left;
					ll_task_split_left[ll_section_count - 1][ll_task_idx] = id_ori2light[left];
					ll_task_split_right[ll_section_count - 1][ll_task_idx] = id_ori2light[item];
					ll_task_idx++;
					ll_task_num++;
				}

			}

		}
	}

	//print tasks
	cout << "size of LL Task List " << ll_task_num << endl;
	cout << "size of HL Task List " << hl_task_num << endl;
	cout << "size of HH Task List " << hh_task_num << endl;

	cout << "size of LL Task Section " << ll_section_count << endl;
	cout << "size of HL Task Section " << hl_section_count << endl;
	cout << "size of HH Task Section " << hh_section_count << endl;


	unsigned long long ll_tc = 0, hl_tc = 0, hh_tc = 0;
	cout << "*** Run Light-Light Merge ***" << endl;
	for(int t = 0; t < ll_section_count; t++){
		if(t!=ll_section_count-1){
			LLTriCount(
						light_offset_left,light_adj_left,
						light_offset_right,light_adj_right,
						ll_task_split_left[t],ll_task_split_right[t],
						TASK_NUM_UP_BOUND,
						t,
						ll_output
						);
		}else{
			LLTriCount(
						light_offset_left,light_adj_left,
						light_offset_right,light_adj_right,
						ll_task_split_left[t],ll_task_split_right[t],
						ll_task_idx,
						t,
						ll_output
						);
		}
		ll_tc += ll_output[t];
	}
	cout << "LL TC = " << ll_tc << endl;
	cout << "*** Finish Light-Light Merge ***" << endl;

	cout << "*** Run Heavy-Light Merge ***" << endl;

	for(int t = 0; t < hl_section_count; t++){
		if(t!=hl_section_count-1){
			HLTriCount(
					light_offset_left,light_adj_left,
					heavy_offset_right,heavy_bid_right,heavy_value_right,
					hl_task_split_left[t],hl_task_split_right[t],
					TASK_NUM_UP_BOUND,
					t,
					hl_output
					);
		}else{
			HLTriCount(
					light_offset_left,light_adj_left,
					heavy_offset_right,heavy_bid_right,heavy_value_right,
					hl_task_split_left[t],hl_task_split_right[t],
					hl_task_idx,
					t,
					hl_output
					);
		}
		hl_tc += hl_output[t];
	}

	cout << "HL TC = " << hl_tc << endl;
	cout << "*** Finish Heavy-Light Merge ***" << endl;

	cout << "*** Run Heavy-Heavy Merge ***" << endl;
	for(int t = 0; t < hh_section_count; t++){
		if(t!=hh_section_count-1){
			HHTriCount(
					heavy_offset_left,heavy_bid_left,heavy_value_left,
					heavy_offset_right,heavy_bid_right,heavy_value_right,
					hh_task_split_left[t],hh_task_split_right[t],
					TASK_NUM_UP_BOUND,
					t,
					hh_output
					);
		}else{
			HHTriCount(
					heavy_offset_left,heavy_bid_left,heavy_value_left,
					heavy_offset_right,heavy_bid_right,heavy_value_right,
					hh_task_split_left[t],hh_task_split_right[t],
					hh_task_idx,
					t,
					hh_output
					);
		}
		hh_tc += hh_output[t];
	}
	cout << "HH TC = " << hh_tc << endl;
	cout << "*** Finish Heavy-Heavy Merge ***" << endl;
	cout << "Final TC = " << hh_tc + hl_tc + ll_tc << endl;


	// release memory
	for(auto& item : ll_task_split_left){
		delete item;
	}
	for(auto& item : ll_task_split_right){
		delete item;
	}
	for(auto& item : hl_task_split_left){
		delete item;
	}
	for(auto& item : hl_task_split_right){
		delete item;
	}
	for(auto& item : hh_task_split_left){
		delete item;
	}
	for(auto& item : hh_task_split_right){
		delete item;
	}
	delete[] light_offset_left;
	delete[] light_offset_right;
	delete[] light_adj_left;
	delete[] light_adj_right;
	delete[] heavy_offset_left;
	delete[] heavy_offset_right;
	delete[] heavy_bid_left;
	delete[] heavy_bid_right;
	delete[] heavy_value_left;
	delete[] heavy_value_right;
	return;
}
*/

/*
int main(){

	//std::cout << "begin graph construction" << std::endl;
	//testGraph("../../data/compress_patents.txt");
	//cout << endl;

	//std::cout << "begin testing light-light merge" << std::endl;
	//testLL("../../data/testgraph.txt");

	//std::cout << "begin testing heavy-heavy merge" << std::endl;
	//testHH("../../data/testgraph.txt");

	//std::cout << "begin testing heavy-light merge" << std::endl;
	//testHL("../../data/testgraph.txt");

	//std::cout << "begin testing TC" << std::endl;
	//testTC("../../data/compress_patents.txt",'c');

	std::cout << "begin testing split TC" << std::endl;
	testTaskDiv("../../data/compress_patents.txt",'c');
	return 0;
}
*/










