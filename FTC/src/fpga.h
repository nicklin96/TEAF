
#ifndef F_MATCH_FPGA_H
#define F_MATCH_FPGA_H

#include "../lib/xcl2.cpp"
#include "../lib/xcl2.hpp"
#include <vector>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <unordered_map>
#include <sys/time.h>
#include "graph.h"
#include <string>
#include "LLTriCount.cpp"
#include "HHTriCount.cpp"
#include "HLTriCount.cpp"

class Host {
public:

    vector<cl::Context> context_list;
    vector<cl::Device> device_list;
    vector<cl::Program> program_list;
    vector<cl::CommandQueue> q_list;
    vector<cl::Event*> hh_events;
    vector<cl::Event*> hl_events;
    vector<cl::Event*> ll_events;
    int outputSize;
    vector<vector<unsigned long long>> *deviceResults;

    Host() {
        deviceResults = new vector<vector<unsigned long long>>();
    }

    void init(string hh_binary_file,string hl_binary_file,string ll_binary_file){
    	vector<string> bf_path = {hh_binary_file,hl_binary_file,ll_binary_file};
    	cl_int err;
    	for(int i = 0; i < 3 ; i++){
    		cout << "initial " << bf_path[i] << std::endl;
    		auto devices = xcl::get_xil_devices();
    		auto tmp_device = devices[i];
    		cl::Context context(tmp_device, NULL, NULL, NULL, &err);

    		auto fileBuf = xcl::read_binary_file(bf_path[i]);
    		cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    		std::swap(devices[0],devices[i]);
    		devices.resize(1);
    		cl::Program program(context, devices, bins, NULL, &err);
    		cl::CommandQueue q(context, tmp_device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
    		context_list.push_back(context);
    		device_list.push_back(tmp_device);
    		program_list.push_back(program);
    		q_list.push_back(q);
    		hl_events.emplace_back();
    		hh_events.emplace_back();
    		ll_events.emplace_back();
    	}
    	std::cout << "size of context " << context_list.size() << std::endl;
    	std::cout << "size of device " << device_list.size() << std::endl;
    	std::cout << "size of program " << program_list.size() << std::endl;
    	std::cout << "size of q " << q_list.size() << std::endl;
    	std::cout << "size of events " << ll_events.size() << " " << hl_events.size() << " " << hh_events.size() << std::endl;


    }

    ~Host(){
		for(auto e : hh_events){
			delete e;
		}
		for(auto e : hl_events){
			delete e;
		}
		for(auto e : ll_events){
			delete e;
		}
    }
};

class FPGATC{
public:
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> ll_task_split_left;
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> ll_task_split_right;
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> ll_task_split_cpu_left;
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> ll_task_split_cpu_right;
	int ll_section_count = 0;
	int ll_task_idx = 0;

	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> hl_task_split_left;
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> hl_task_split_right;
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> hl_task_split_cpu_left;
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> hl_task_split_cpu_right;
	int hl_section_count = 0;
	int hl_task_idx = 0;

	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> hh_task_split_left;
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> hh_task_split_right;
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> hh_task_split_cpu_left;
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE>> hh_task_split_cpu_right;
	int hh_section_count = 0;
	int hh_task_idx = 0;

	int hh_task_num = 0;
	int hl_task_num = 0;
	int ll_task_num = 0;

	vector<unsigned long long> ll_output;
	vector<unsigned long long> hl_output;
	vector<unsigned long long> hh_output;
	vector<unsigned long long> ll_output_cpu;
	vector<unsigned long long> hl_output_cpu;
	vector<unsigned long long> hh_output_cpu;
	vector<vector<unsigned long long,aligned_allocator<unsigned long long>>> ll_output_multi;
	vector<vector<unsigned long long,aligned_allocator<unsigned long long>>> hl_output_multi;
	vector<vector<unsigned long long,aligned_allocator<unsigned long long>>> hh_output_multi;

	vector<bool> is_heavy;
	map<RAW_ID_TYPE,RAW_ID_TYPE> id_ori2light;
	map<RAW_ID_TYPE,RAW_ID_TYPE> id_ori2heavy;

	LightCSR* lcsr;
	HeavyCSR* hcsr;

	int hh_kernel_num = 8;
	int hl_kernel_num = 8;
	int ll_kernel_num = 8;

	FPGATC(string path,char mode){
		std::cout << "*** Split Graph ***" << std::endl;
		std::cout << path << std::endl;
		Graph g,light,heavy;
		if(mode=='n'){
			g.readGraph(path);
		}else if(mode=='c'){
			g.readCompressedGraph(path);
		}else{
			cout << "no such mode!"<<endl;
			return;
		}
		int heavy_num = getHeavyNum(g);
		g.splitGraph(light,heavy,is_heavy,id_ori2light,id_ori2heavy,heavy_num);
		std::cout << "vertex num of light graph " << light.vertex_num << std::endl;
		std::cout << "vertex num of heavy graph " << heavy.vertex_num << std::endl;
		std::cout << "*** Split Graph Finished *** " << std::endl << std::endl;

		std::cout << "*** Split Graph ***" << std::endl;

		cout << "*** Construct Heavy CSR ***" << endl;
		heavy.printDetail();
		hcsr = new HeavyCSR(heavy);
		//hcsr.printCSR();
		cout << "*** Heavy CSR Construction Finished ***" << endl << endl;

		cout << "*** Construct Light CSR ***" << endl;
		light.printDetail();
		lcsr = new LightCSR(light);
		cout << "*** Light CSR Construction Finished ***" << endl << endl;

		for(int k = 0; k < OUT_LEN; k++){
			ll_output.push_back(0);
			hl_output.push_back(0);
			hh_output.push_back(0);
			ll_output_cpu.push_back(0);
			hl_output_cpu.push_back(0);
			hh_output_cpu.push_back(0);
		}

		for(int kk = 0; kk < OUT_LEN; kk++){
			hh_output_multi.emplace_back();
			hl_output_multi.emplace_back();
			ll_output_multi.emplace_back();
				for(int k = 0; k < OUT_LEN; k++){
					ll_output_multi[kk].push_back(0);
					hl_output_multi[kk].push_back(0);
					hh_output_multi[kk].push_back(0);
				}
		}


		cout << "*** Task Split ***" << std::endl;
		hh_task_idx = 0;
		hl_task_idx = 0;
		ll_task_idx = 0;
		for(RAW_ID_TYPE i = 0; i < g.vertex_num; i++){
			RAW_ID_TYPE left = g.degrees[i].id;
			for(auto item : g.edges[left]){
				//std::cout << "dealing with " << left << " " << item << std::endl;
				if(is_heavy[left]&&is_heavy[item]){
					//std::cout << "hh" << std::endl;
					//std::cout << "dealing with " << left << " " << item << std::endl;
					int adj_size_right = hcsr->offset[id_ori2heavy[item]+1] - hcsr->offset[id_ori2heavy[item]];
					if(adj_size_right > 0){
						//hh_task_left[hh_task_num] = left;
						//hh_task_right[hh_task_num] = item;
						hh_task_split_left.push_back(id_ori2heavy[left]);
						hh_task_split_right.push_back(id_ori2heavy[item]);
						hh_task_idx++;
						//hh_task_num++;
					}
				}
				else if((is_heavy[left]&&!is_heavy[item]) || (!is_heavy[left]&&is_heavy[item])){
					//std::cout << "hl" << std::endl;
					if(is_heavy[item]){
						int adj_size_right = hcsr->offset[id_ori2heavy[item]+1] - hcsr->offset[id_ori2heavy[item]];
						if(adj_size_right > 0){
							//hl_task_left[hl_task_num] = left;
							//hl_task_right[hl_task_num] = item;
							hl_task_split_left.push_back(id_ori2light[left]);
							hl_task_split_right.push_back(id_ori2heavy[item]);
							hl_task_idx++;
							//hl_task_num++;
						}
					}else{
						//std::cout << "ll" << std::endl;
						int adj_size_right = lcsr->offset[id_ori2light[item]+1] - lcsr->offset[id_ori2light[item]];
						if(adj_size_right > 0){
							//hl_task_left[hl_task_num] = item;
							//hl_task_right[hl_task_num] = left;
							hl_task_split_left.push_back(id_ori2light[item]);
							hl_task_split_right.push_back(id_ori2heavy[left]);
							hl_task_idx++;
							//hl_task_num++;
						}
					}
				}else{
					int adj_size_right = lcsr->offset[id_ori2light[item]+1] - lcsr->offset[id_ori2light[item]];
					if(adj_size_right > 0){

						ll_task_split_left.push_back(id_ori2light[left]);
						ll_task_split_right.push_back(id_ori2light[item]);
						ll_task_idx++;
						//ll_task_num++;
					}

				}

			}
		}

		hh_task_num = hh_task_idx / TASK_NUM_UP_BOUND;
		ll_task_num = ll_task_idx / TASK_NUM_UP_BOUND;
		hl_task_num = hl_task_idx / TASK_NUM_UP_BOUND;

		hh_task_num = 0.6 * hh_task_num;
		hl_task_num = 0.6 * hl_task_num;
		ll_task_num = 0.6 * ll_task_num;

		hh_task_num -= hh_task_num % hh_kernel_num;
		hl_task_num -= hl_task_num % hl_kernel_num;
		ll_task_num -= ll_task_num % ll_kernel_num;

		int ll_cpu_count = ll_task_idx - ll_task_num * TASK_NUM_UP_BOUND;
		int hl_cpu_count = hl_task_idx - hl_task_num * TASK_NUM_UP_BOUND;
		int hh_cpu_count = hh_task_idx - hh_task_num * TASK_NUM_UP_BOUND;

		int ll_fpga_count = ll_task_num * TASK_NUM_UP_BOUND;
		int hl_fpga_count = hl_task_num * TASK_NUM_UP_BOUND;
		int hh_fpga_count = hh_task_num * TASK_NUM_UP_BOUND;


		for(int i = 0 ; i < hh_cpu_count; i++){
			hh_task_split_cpu_left.push_back(hh_task_split_left[i]);
			hh_task_split_cpu_right.push_back(hh_task_split_right[i]);
		}
		for(int i = 0 ; i < hl_cpu_count; i++){
			hl_task_split_cpu_left.push_back(hl_task_split_left[i]);
			hl_task_split_cpu_right.push_back(hl_task_split_right[i]);
		}
		for(int i = 0 ; i < ll_cpu_count; i++){
			ll_task_split_cpu_left.push_back(ll_task_split_left[i]);
			ll_task_split_cpu_right.push_back(ll_task_split_right[i]);
		}
		for(int i = 0 ; i < hh_fpga_count; i++){
			hh_task_split_left[i] = hh_task_split_left[hh_cpu_count + i];
			hh_task_split_right[i] = hh_task_split_right[hh_cpu_count + i];
		}
		for(int i = 0 ; i < hl_fpga_count; i++){
			hl_task_split_left[i] = hl_task_split_left[hl_cpu_count + i];
			hl_task_split_right[i] = hl_task_split_right[hl_cpu_count + i];
		}
		for(int i = 0 ; i < ll_fpga_count; i++){
			ll_task_split_left[i] = ll_task_split_left[ll_cpu_count + i];
			ll_task_split_right[i] = ll_task_split_right[ll_cpu_count + i];
		}

		hh_task_split_left.resize(hh_task_num * TASK_NUM_UP_BOUND);
		hl_task_split_left.resize(hl_task_num * TASK_NUM_UP_BOUND);
		ll_task_split_left.resize(ll_task_num * TASK_NUM_UP_BOUND);

		hh_task_split_right.resize(hh_task_num * TASK_NUM_UP_BOUND);
		hl_task_split_right.resize(hl_task_num * TASK_NUM_UP_BOUND);
		ll_task_split_right.resize(ll_task_num * TASK_NUM_UP_BOUND);

		//print tasks
		std::cout << "size of LL Task List " << ll_task_idx << std::endl;
		std::cout << "size of HL Task List " << hl_task_idx << std::endl;
		std::cout << "size of HH Task List " << hh_task_idx << std::endl;


		std::cout << "size of LL Task CPU " << ll_cpu_count << std::endl;
		std::cout << "size of HL Task CPU " << hl_cpu_count << std::endl;
		std::cout << "size of HH Task CPU " << hh_cpu_count << std::endl;

		std::cout << "size of LL Task FPGA " << ll_task_split_left.size() << " " << ll_task_split_right.size() << std::endl;
		std::cout << "size of HL Task FPGA " << hl_task_split_left.size() << " " << hl_task_split_right.size() << std::endl;
		std::cout << "size of HH Task FPGA " << hh_task_split_left.size() << " " << hh_task_split_right.size() << std::endl;

		std::cout << "size of LL Task Num " << ll_task_num << std::endl;
		std::cout << "size of HL Task Num " << hl_task_num << std::endl;
		std::cout << "size of HH Task Num " << hh_task_num << std::endl;

	}

	~FPGATC(){
		delete hcsr;
		delete lcsr;
	}

	uint64_t get_duration_ns(const cl::Event *event) {
	    cl_int err;
	    uint64_t nstimestart, nstimeend;
	    OCL_CHECK(err,
	              err = event->getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_START, &nstimestart));
	    OCL_CHECK(err,
	              err = event->getProfilingInfo<uint64_t>(CL_PROFILING_COMMAND_END, &nstimeend));
	    return (nstimeend - nstimestart) ;
	}

	int getHeavyNum(Graph& g){
		int heavy_num;
		unsigned long long degreeCount = 0;
		unsigned long long threshold = g.degreeSum * 0.25;
		for(heavy_num = 0; heavy_num < g.vertex_num;heavy_num++){
			degreeCount += g.edges[g.degrees[heavy_num].id].size();
			if(degreeCount > threshold){
				break;
			}
		}
		return heavy_num - 1;

	}

	void runOnCPU(){
		struct timeval start{}, end{};
		double time_cost = 0;
		unsigned long long ll_tc = 0, hl_tc = 0, hh_tc = 0;
		cout << "*** Run Light-Light Merge ***" << endl;
		gettimeofday(&start,nullptr);
		LLTriCount(
					lcsr->offset.data(),lcsr->adj.data(),
					lcsr->offset.data(),lcsr->adj.data(),
					ll_task_split_left.data(),ll_task_split_right.data(),
					ll_task_num,
					ll_output.data()
					);
		LLTriCount_CPU(
					lcsr->offset.data(),lcsr->adj.data(),
					lcsr->offset.data(),lcsr->adj.data(),
					ll_task_split_cpu_left.data(),ll_task_split_cpu_right.data(),
					ll_task_split_cpu_left.size(),
					ll_output_cpu.data()
					);
		gettimeofday(&end,nullptr);
		time_cost +=(double) (end.tv_sec - start.tv_sec) * 1000 +
		                          (end.tv_usec - start.tv_usec) / 1000.0;
		ll_tc += ll_output[0] + ll_output_cpu[0];

		cout << "LL TC = " << ll_tc << endl;
		cout << "FPGA LL TC = " << ll_output[0] << endl;
		cout << "CPU LL TC = " << ll_output_cpu[0] << endl;
		cout << "*** Finish Light-Light Merge ***" << endl;

		cout << "*** Run Heavy-Light Merge ***" << endl;
		gettimeofday(&start,nullptr);
		HLTriCount(
					lcsr->offset.data(),lcsr->adj.data(),
					hcsr->offset.data(),hcsr->bid.data(),hcsr->value.data(),
					hl_task_split_left.data(),hl_task_split_right.data(),
					hl_task_num,
					hl_output.data()
					);
		cout << "on CPU begin" << endl;
		HLTriCount_CPU(
					lcsr->offset.data(),lcsr->adj.data(),
					hcsr->offset.data(),hcsr->bid.data(),hcsr->value.data(),
					hl_task_split_cpu_left.data(),hl_task_split_cpu_right.data(),
					hl_task_split_cpu_left.size(),
					hl_output_cpu.data()
					);
		gettimeofday(&end,nullptr);
		time_cost +=(double) (end.tv_sec - start.tv_sec) * 1000 +
				                          (end.tv_usec - start.tv_usec) / 1000.0;
		hl_tc += hl_output[0] + hl_output_cpu[0];
		cout << "HL TC = " << hl_tc << endl;
		cout << "FPGA HL TC = " << hl_output[0] << endl;
		cout << "CPU HL TC = " << hl_output_cpu[0] << endl;
		cout << "*** Finish Heavy-Light Merge ***" << endl;

		cout << "*** Run Heavy-Heavy Merge ***" << endl;
		gettimeofday(&start,nullptr);
		HHTriCount(
					hcsr->offset.data(),hcsr->bid.data(),hcsr->value.data(),
					hcsr->offset.data(),hcsr->bid.data(),hcsr->value.data(),
					hh_task_split_left.data(),hh_task_split_right.data(),
					hh_task_num,
					hh_output.data()
					);
		HHTriCount_CPU(
					hcsr->offset.data(),hcsr->bid.data(),hcsr->value.data(),
					hcsr->offset.data(),hcsr->bid.data(),hcsr->value.data(),
					hh_task_split_cpu_left.data(),hh_task_split_cpu_right.data(),
					hh_task_split_cpu_left.size(),
					hh_output_cpu.data()
					);
		gettimeofday(&end,nullptr);
		time_cost +=(double) (end.tv_sec - start.tv_sec) * 1000 +
										  (end.tv_usec - start.tv_usec) / 1000.0;
		hh_tc += hh_output[0] + hh_output_cpu[0];
		cout << "HH TC = " << hh_tc << endl;
		cout << "FPGA HH TC = " << hh_output[0] << endl;
		cout << "CPU HH TC = " << hh_output_cpu[0] << endl;
		cout << hh_output[1] << " " << hh_output[2] << endl;
		cout << "*** Finish Heavy-Heavy Merge ***" << endl;
		cout << "Final TC = " << hh_tc + hl_tc + ll_tc << endl;
		cout << "Time cost = " << time_cost << endl;

	}




	void runOnFPGA(Host* host){

		//vector<string> kernel_name = {"HHTriCount","HLTriCount","LLTriCount"};
		vector<cl::Kernel> hh_kernels(hh_kernel_num);
		vector<cl::Kernel> hl_kernels(hl_kernel_num);
		vector<cl::Kernel> ll_kernels(ll_kernel_num);
		cl_int hh_err;
		cl_int hl_err;
		cl_int ll_err;

		for(int i = 0 ; i < hh_kernel_num ; i++){
			OCL_CHECK(hh_err,hh_kernels[i] = cl::Kernel(host->program_list[0], "HHTriCount", &hh_err));
		}
		for(int i = 0 ; i < hl_kernel_num ; i++){
			OCL_CHECK(hl_err,hl_kernels[i] = cl::Kernel(host->program_list[1], "HLTriCount", &hl_err));
		}
		for(int i = 0 ; i < ll_kernel_num ; i++){
			OCL_CHECK(ll_err,ll_kernels[i] = cl::Kernel(host->program_list[2], "LLTriCount", &ll_err));
		}




		//cl::Kernel hh_kernel(host->program_list[0], "HHTriCount", &hh_err);
		//cl::Kernel hl_kernel(host->program_list[1], "HLTriCount", &hl_err);
		//cl::Kernel ll_kernel(host->program_list[2], "LLTriCount", &ll_err);

		cl_mem_flags read = CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY;
		cl_mem_flags write = CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE;

		std::cout << " size of hcsr " << hcsr->offset.size() << " " << hcsr->offset.size() << " " << hcsr->value.size() << std::endl;
		std::cout << " size of lcsr " << lcsr->offset.size() << " " << lcsr->adj.size() << std::endl;
		std::cout << " size of output" << hh_output.size() << " " << hl_output.size() << " " << ll_output.size() << std::endl;

		std::cout << "*** init hh buffers ***" << std::endl;

		cl::Buffer *hh_offset_left = new cl::Buffer(host->context_list[0], read, sizeof(RAW_ID_TYPE) * hcsr->offset.size(),
		                                       hcsr->offset.data(),
		                                       &hh_err);
		cl::Buffer *hh_offset_right = new cl::Buffer(host->context_list[0], read, sizeof(RAW_ID_TYPE) * hcsr->offset.size(),
											   hcsr->offset.data(),
											   &hh_err);
		cl::Buffer *hh_bid_left = new cl::Buffer(host->context_list[0], read, sizeof(BID_TYPE) * hcsr->bid.size(),
											   hcsr->bid.data(),
											   &hh_err);
		cl::Buffer *hh_bid_right = new cl::Buffer(host->context_list[0], read, sizeof(BID_TYPE) * hcsr->bid.size(),
											   hcsr->bid.data(),
											   &hh_err);
		cl::Buffer *hh_value_left = new cl::Buffer(host->context_list[0], read, sizeof(VID_TYPE) * hcsr->value.size(),
											   hcsr->value.data(),
											   &hh_err);
		cl::Buffer *hh_value_right = new cl::Buffer(host->context_list[0], read, sizeof(VID_TYPE) * hcsr->value.size(),
											   hcsr->value.data(),
											   &hh_err);
		vector<cl::Buffer*> hh_task_left(hh_kernel_num);
		vector<cl::Buffer*> hh_task_right(hh_kernel_num);
		vector<cl::Buffer*> hh_out(hh_kernel_num);
		int hh_chunk_size = hh_task_num / hh_kernel_num;
		//int hh_remain_size = hh_task_num % hh_kernel_num;
		vector<int> hh_isize_list;
		//int hh_isize = 0;
		//int hh_offset = 0;
		std::cout << "hh chunk size = " << hh_chunk_size << std::endl;
		for(int i = 0; i < hh_kernel_num; i++){
			hh_isize_list.push_back(hh_chunk_size);

			hh_task_left[i] = new cl::Buffer(host->context_list[0], read, sizeof(RAW_ID_TYPE) * hh_chunk_size * TASK_NUM_UP_BOUND,
											   hh_task_split_left.data() + i * hh_chunk_size * TASK_NUM_UP_BOUND,
											   &hh_err);
			hh_task_right[i] = new cl::Buffer(host->context_list[0], read, sizeof(RAW_ID_TYPE) * hh_chunk_size * TASK_NUM_UP_BOUND,
											   hh_task_split_right.data()+ i * hh_chunk_size * TASK_NUM_UP_BOUND,
											   &hh_err);
			hh_out[i] = new cl::Buffer(host->context_list[0], write, sizeof(unsigned long long) * hh_output_multi[i].size(),
											   hh_output_multi[i].data(),
												//hh_output.data(),
											   &hh_err);
		}
//		cl::Buffer *hh_task_left = new cl::Buffer(host->context_list[0], read, sizeof(RAW_ID_TYPE) * hh_task_split_left.size(),
//											   hh_task_split_left.data(),
//											   &hh_err);
//		cl::Buffer *hh_task_right = new cl::Buffer(host->context_list[0], read, sizeof(RAW_ID_TYPE) * hh_task_split_right.size(),
//											   hh_task_split_right.data(),
//											   &hh_err);
//		cl::Buffer *hh_out = new cl::Buffer(host->context_list[0], write, sizeof(unsigned long long) * hh_output.size(),
//											   hh_output.data(),
//											   &hh_err);




		std::cout << "*** init hl buffers ***" << std::endl;

		cl::Buffer *hl_offset_left = new cl::Buffer(host->context_list[1], read, sizeof(RAW_ID_TYPE) * lcsr->offset.size(),
											   lcsr->offset.data(),
											   &hl_err);
		cl::Buffer *hl_offset_right = new cl::Buffer(host->context_list[1], read, sizeof(RAW_ID_TYPE) * hcsr->offset.size(),
											   hcsr->offset.data(),
											   &hl_err);
		cl::Buffer *hl_adj_left = new cl::Buffer(host->context_list[1], read, sizeof(BID_TYPE) * lcsr->adj.size(),
											   lcsr->adj.data(),
											   &hl_err);
		cl::Buffer *hl_bid_right = new cl::Buffer(host->context_list[1], read, sizeof(BID_TYPE) * hcsr->bid.size(),
											   hcsr->bid.data(),
											   &hl_err);
		cl::Buffer *hl_value_right = new cl::Buffer(host->context_list[1], read, sizeof(VID_TYPE) * hcsr->value.size(),
											   hcsr->value.data(),
											   &hl_err);

		vector<cl::Buffer*> hl_task_left(hl_kernel_num);
		vector<cl::Buffer*> hl_task_right(hl_kernel_num);
		vector<cl::Buffer*> hl_out(hl_kernel_num);
		int hl_chunk_size = hl_task_num / hl_kernel_num;
		//int hl_remain_size = hl_task_num % hl_kernel_num;
		vector<int> hl_isize_list;
		//int hl_isize = 0;
		//int hl_offset = 0;
		std::cout << "hl chunk size = " << hl_chunk_size << std::endl;
		for(int i = 0; i < hl_kernel_num; i++){
			hl_isize_list.push_back(hl_chunk_size);

			hl_task_left[i] = new cl::Buffer(host->context_list[1], read, sizeof(RAW_ID_TYPE) * hl_chunk_size * TASK_NUM_UP_BOUND,
											   hl_task_split_left.data() + i * hl_chunk_size * TASK_NUM_UP_BOUND,
											   &hl_err);
			hl_task_right[i] = new cl::Buffer(host->context_list[1], read, sizeof(RAW_ID_TYPE) * hl_chunk_size * TASK_NUM_UP_BOUND,
											   hl_task_split_right.data()+ i * hl_chunk_size * TASK_NUM_UP_BOUND,
											   &hl_err);
			hl_out[i] = new cl::Buffer(host->context_list[1], write, sizeof(unsigned long long) * hl_output_multi[i].size(),
											   hl_output_multi[i].data(),
												//hl_output.data(),
											   &hl_err);
		}

//		cl::Buffer *hl_task_left = new cl::Buffer(host->context_list[1], read, sizeof(RAW_ID_TYPE) * hl_task_split_left.size(),
//											   hl_task_split_left.data(),
//											   &hl_err);
//		cl::Buffer *hl_task_right = new cl::Buffer(host->context_list[1], read, sizeof(RAW_ID_TYPE) * hl_task_split_right.size(),
//											   hl_task_split_right.data(),
//											   &hl_err);
//
//		cl::Buffer *hl_out = new cl::Buffer(host->context_list[1], write, sizeof(unsigned long long) * hl_output.size(),
//											   hl_output.data(),
//											   &hl_err);


		std::cout << "*** init ll buffers ***" << std::endl;

		cl::Buffer *ll_offset_left = new cl::Buffer(host->context_list[2], read, sizeof(RAW_ID_TYPE) * lcsr->offset.size(),
											   lcsr->offset.data(),
											   &ll_err);
		cl::Buffer *ll_offset_right = new cl::Buffer(host->context_list[2], read, sizeof(RAW_ID_TYPE) * lcsr->offset.size(),
											   lcsr->offset.data(),
											   &ll_err);
		cl::Buffer *ll_adj_left = new cl::Buffer(host->context_list[2], read, sizeof(RAW_ID_TYPE) * lcsr->adj.size(),
											   lcsr->adj.data(),
											   &ll_err);
		cl::Buffer *ll_adj_right = new cl::Buffer(host->context_list[2], read, sizeof(RAW_ID_TYPE) * lcsr->adj.size(),
											   lcsr->adj.data(),
											   &ll_err);
		vector<cl::Buffer*> ll_task_left(ll_kernel_num);
		vector<cl::Buffer*> ll_task_right(ll_kernel_num);
		vector<cl::Buffer*> ll_out(ll_kernel_num);
		int ll_chunk_size = ll_task_num / ll_kernel_num;
		//int ll_remain_size = ll_task_num % ll_kernel_num;
		vector<int> ll_isize_list;
		//int ll_isize = 0;
		//int ll_offset = 0;
		std::cout << "ll chunk size = " << ll_chunk_size << std::endl;
		for(int i = 0; i < ll_kernel_num; i++){

			ll_isize_list.push_back(ll_chunk_size);

			ll_task_left[i] = new cl::Buffer(host->context_list[2], read, sizeof(RAW_ID_TYPE) * ll_chunk_size * TASK_NUM_UP_BOUND,
											   ll_task_split_left.data() + i * ll_chunk_size * TASK_NUM_UP_BOUND,
											   &ll_err);
			ll_task_right[i] = new cl::Buffer(host->context_list[2], read, sizeof(RAW_ID_TYPE) * ll_chunk_size * TASK_NUM_UP_BOUND,
											   ll_task_split_right.data()+ i * ll_chunk_size * TASK_NUM_UP_BOUND,
											   &ll_err);
			ll_out[i] = new cl::Buffer(host->context_list[2], write, sizeof(unsigned long long) * ll_output_multi[i].size(),
											   ll_output_multi[i].data(),
												//ll_output.data(),
											   &ll_err);
		}

//		cl::Buffer *ll_task_left = new cl::Buffer(host->context_list[2], read, sizeof(RAW_ID_TYPE) * ll_task_split_left.size(),
//											   ll_task_split_left.data(),
//											   &ll_err);
//		cl::Buffer *ll_task_right = new cl::Buffer(host->context_list[2], read, sizeof(RAW_ID_TYPE) * ll_task_split_right.size(),
//											   ll_task_split_right.data(),
//											   &ll_err);
//
//		cl::Buffer *ll_out = new cl::Buffer(host->context_list[2], write, sizeof(unsigned long long) * ll_output.size(),
//											   ll_output.data(),
//											   &ll_err);


		int hh_arg_count = 0, hl_arg_count = 0, ll_arg_count = 0;

		std::cout << "*** init hh args ***" << std::endl;
		for(int i = 0; i < hh_kernel_num; i++){
			hh_arg_count = 0;
			OCL_CHECK(hh_err, hh_err = hh_kernels[i].setArg(hh_arg_count++, *hh_offset_left));
			OCL_CHECK(hh_err, hh_err = hh_kernels[i].setArg(hh_arg_count++, *hh_bid_left));
			OCL_CHECK(hh_err, hh_err = hh_kernels[i].setArg(hh_arg_count++, *hh_value_left));
			OCL_CHECK(hh_err, hh_err = hh_kernels[i].setArg(hh_arg_count++, *hh_offset_right));
			OCL_CHECK(hh_err, hh_err = hh_kernels[i].setArg(hh_arg_count++, *hh_bid_right));
			OCL_CHECK(hh_err, hh_err = hh_kernels[i].setArg(hh_arg_count++, *hh_value_right));
			OCL_CHECK(hh_err, hh_err = hh_kernels[i].setArg(hh_arg_count++, *hh_task_left[i]));
			OCL_CHECK(hh_err, hh_err = hh_kernels[i].setArg(hh_arg_count++, *hh_task_right[i]));
			OCL_CHECK(hh_err, hh_err = hh_kernels[i].setArg(hh_arg_count++, hh_isize_list[i]));
			OCL_CHECK(hh_err, hh_err = hh_kernels[i].setArg(hh_arg_count++, *hh_out[i]));
		}
/*
		OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count++, *hh_offset_left));
		OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count++, *hh_bid_left));
		OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count++, *hh_value_left));
		OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count++, *hh_offset_right));
		OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count++, *hh_bid_right));
		OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count++, *hh_value_right));
		OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count++, *hh_task_left));
		OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count++, *hh_task_right));
		OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count++, hh_task_num));
		OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count++, *hh_out));
*/
		std::cout << "*** init hl args ***" << std::endl;
		for(int i = 0; i < hl_kernel_num; i++){
			hl_arg_count = 0;
			OCL_CHECK(hl_err, hl_err = hl_kernels[i].setArg(hl_arg_count++, *hl_offset_left));
			OCL_CHECK(hl_err, hl_err = hl_kernels[i].setArg(hl_arg_count++, *hl_adj_left));
			OCL_CHECK(hl_err, hl_err = hl_kernels[i].setArg(hl_arg_count++, *hl_offset_right));
			OCL_CHECK(hl_err, hl_err = hl_kernels[i].setArg(hl_arg_count++, *hl_bid_right));
			OCL_CHECK(hl_err, hl_err = hl_kernels[i].setArg(hl_arg_count++, *hl_value_right));
			OCL_CHECK(hl_err, hl_err = hl_kernels[i].setArg(hl_arg_count++, *hl_task_left[i]));
			OCL_CHECK(hl_err, hl_err = hl_kernels[i].setArg(hl_arg_count++, *hl_task_right[i]));
			OCL_CHECK(hl_err, hl_err = hl_kernels[i].setArg(hl_arg_count++, hl_isize_list[i]));
			OCL_CHECK(hl_err, hl_err = hl_kernels[i].setArg(hl_arg_count++, *hl_out[i]));
		}
/*
		OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count++, *hl_offset_left));
		OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count++, *hl_adj_left));
		OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count++, *hl_offset_right));
		OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count++, *hl_bid_right));
		OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count++, *hl_value_right));
		OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count++, *hl_task_left));
		OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count++, *hl_task_right));
		OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count++, hl_task_num));
		OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count++, *hl_out));
*/
		std::cout << "** init ll args ***" << std::endl;
		for(int i = 0 ; i < ll_kernel_num; i++){
			ll_arg_count = 0;
			OCL_CHECK(ll_err, ll_err = ll_kernels[i].setArg(ll_arg_count++, *ll_offset_left));
			OCL_CHECK(ll_err, ll_err = ll_kernels[i].setArg(ll_arg_count++, *ll_adj_left));
			OCL_CHECK(ll_err, ll_err = ll_kernels[i].setArg(ll_arg_count++, *ll_offset_right));
			OCL_CHECK(ll_err, ll_err = ll_kernels[i].setArg(ll_arg_count++, *ll_adj_right));
			OCL_CHECK(ll_err, ll_err = ll_kernels[i].setArg(ll_arg_count++, *ll_task_left[i]));
			OCL_CHECK(ll_err, ll_err = ll_kernels[i].setArg(ll_arg_count++, *ll_task_right[i]));
			OCL_CHECK(ll_err, ll_err = ll_kernels[i].setArg(ll_arg_count++, ll_isize_list[i]));
			OCL_CHECK(ll_err, ll_err = ll_kernels[i].setArg(ll_arg_count++, *ll_out[i]));
		}
/*
		OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count++, *ll_offset_left));
		OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count++, *ll_adj_left));
		OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count++, *ll_offset_right));
		OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count++, *ll_adj_right));
		OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count++, *ll_task_left));
		OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count++, *ll_task_right));
		OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count++, ll_task_num));
		OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count++, *ll_out));
*/
		std::cout << "writing hh csr to fpga" << std::endl;
		OCL_CHECK(hh_err,hh_err = host->q_list[0].enqueueMigrateMemObjects(
						  {*hh_offset_left,
						  *hh_bid_left,
						  *hh_value_left,
						  *hh_offset_right,
						  *hh_bid_right,
						  *hh_value_right,},
						  0));
		std::cout << "writing hh task to fpga" << std::endl;
		for(int i = 0; i < hh_kernel_num; i++){
			//std::cout << "writing hh task no." << i << std::endl;
			OCL_CHECK(hh_err,hh_err = host->q_list[0].enqueueMigrateMemObjects(
									  {
									  *hh_task_left[i],
									  *hh_task_right[i],},
									  0));
		}
		std::cout << "writing hl csr to fpga" << std::endl;
		OCL_CHECK(hl_err,hl_err = host->q_list[1].enqueueMigrateMemObjects(
						  {*hl_offset_left,
						  *hl_adj_left,
						  *hl_offset_right,
						  *hl_bid_right,
						  *hl_value_right,},
						  0));
		std::cout << "writing hl task to fpga" << std::endl;
		for(int i = 0; i < hl_kernel_num; i++){
			OCL_CHECK(hl_err,hl_err = host->q_list[1].enqueueMigrateMemObjects(
						  {
						  *hl_task_left[i],
						  *hl_task_right[i],},
						  0));
		}
		std::cout << "writing ll csr to fpga" << std::endl;
		OCL_CHECK(ll_err,ll_err = host->q_list[2].enqueueMigrateMemObjects(
						  {*ll_offset_left,
						  *ll_adj_left,
						  *ll_offset_right,
						  *ll_adj_right,},
						  0));
		std::cout << "writing hh task to fpga" << std::endl;
		for(int i = 0; i < ll_kernel_num; i++){
			OCL_CHECK(ll_err,ll_err = host->q_list[2].enqueueMigrateMemObjects(
						  {
						  *ll_task_left[i],
						  *ll_task_right[i],},
						  0));
		}
		host->q_list[0].finish();
		host->q_list[1].finish();
		host->q_list[2].finish();

		//int section_num = max(hh_section_count,max(ll_section_count,hl_section_count));

		std::cout << "*** init events ***" << std::endl;
		struct timeval start{}, end{};
		vector<cl::Event*> hh_events(hh_kernel_num);
		vector<cl::Event*> hl_events(hl_kernel_num);
		vector<cl::Event*> ll_events(ll_kernel_num);

		for(int i = 0; i < hh_kernel_num; i++){
			hh_events[i] = new cl::Event();
		}
		for(int i = 0; i < hl_kernel_num; i++){
			hl_events[i] = new cl::Event();
		}
		for(int i = 0; i < ll_kernel_num; i++){
			ll_events[i] = new cl::Event();
		}

		unsigned long long hh_tc = 0, hl_tc = 0, ll_tc = 0;
		std::cout << "*** running ***"<< std::endl;
		gettimeofday(&start,nullptr);
		for(int i = 0; i < hh_kernel_num; i++){
			OCL_CHECK(hh_err, hh_err = host->q_list[0].enqueueTask(hh_kernels[i], NULL, hh_events[i]));
		}
		for(int i = 0; i < hl_kernel_num; i++){
			OCL_CHECK(hl_err, hl_err = host->q_list[1].enqueueTask(hl_kernels[i], NULL, hl_events[i]));
		}
		for(int i = 0; i < hl_kernel_num; i++){
			OCL_CHECK(ll_err, ll_err = host->q_list[2].enqueueTask(ll_kernels[i], NULL, ll_events[i]));
		}
		host->q_list[0].finish();
		host->q_list[1].finish();
		host->q_list[2].finish();

/*
		cl::Event *hh_event = new cl::Event();
		OCL_CHECK(hh_err, hh_err = host->q_list[0].enqueueTask(hh_kernel, NULL, hh_event));
		//host->hh_events.push_back(hh_event);

		cl::Event *hl_event = new cl::Event();
		OCL_CHECK(hl_err, hl_err = host->q_list[1].enqueueTask(hl_kernel, NULL, hl_event));
		//host->hl_events.push_back(hl_event);

		cl::Event *ll_event = new cl::Event();
		OCL_CHECK(ll_err, ll_err = host->q_list[2].enqueueTask(ll_kernel, NULL, ll_event));
		//host->ll_events.push_back(ll_event);
		//gettimeofday(&end,nullptr);
		hh_event->wait();
		hl_event->wait();
		ll_event->wait();
*/
		gettimeofday(&end,nullptr);
		/*
		for(int t = 0; t < section_num; t++){

			if(t < hh_section_count){
				//std::cout << "enqueue hh task " << t << std::endl;
				cl::Buffer *hh_task_left = new cl::Buffer(host->context_list[0], read, sizeof(RAW_ID_TYPE) * hh_task_split_left[t].size(),
												   hh_task_split_left[t].data(),
												   &hh_err);
				cl::Buffer *hh_task_right = new cl::Buffer(host->context_list[0], read, sizeof(RAW_ID_TYPE) * hh_task_split_right[t].size(),
												   hh_task_split_right[t].data(),
												   &hh_err);

				OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count, *hh_task_left));
				OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count+1, *hh_task_right));
				int tmp_task_num = 0;
				if(t != hh_section_count - 1){
					tmp_task_num = TASK_NUM_UP_BOUND;
					//OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count+2, TASK_NUM_UP_BOUND));
				}else{
					tmp_task_num = hh_task_idx;
					//OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count+2, hh_task_idx));
				}
				//std::cout << tmp_task_num << std::endl;
				OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count+2, tmp_task_num));
				OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count+3, t));
				if(t==0){
					OCL_CHECK(hh_err, hh_err = hh_kernel.setArg(hh_arg_count+4, *hh_out));
				}

				OCL_CHECK(hh_err,hh_err = host->q_list[0].enqueueMigrateMemObjects(
										  {*hh_task_left,
										  *hh_task_right,},
										  0));

				//std::cout << "enqueue" << std::endl;
				cl::Event *hh_event = new cl::Event();
				OCL_CHECK(hh_err, hh_err = host->q_list[0].enqueueTask(hh_kernel, NULL, hh_event));
				host->hh_events.push_back(hh_event);

				delete hh_task_left;
				delete hh_task_right;
			}

			if(t < hl_section_count){
				//std::cout << "enqueue hl task " << t << std::endl;
				cl::Buffer *hl_task_left = new cl::Buffer(host->context_list[1], read, sizeof(RAW_ID_TYPE) * hl_task_split_left[t].size(),
												   hl_task_split_left[t].data(),
												   &hl_err);
				cl::Buffer *hl_task_right = new cl::Buffer(host->context_list[1], read, sizeof(RAW_ID_TYPE) * hl_task_split_right[t].size(),
												   hl_task_split_right[t].data(),
												   &hl_err);

				OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count, *hl_task_left));
				OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count+1, *hl_task_right));
				int tmp_task_num2 = 0;
				if(t != hl_section_count - 1){
					tmp_task_num2 = TASK_NUM_UP_BOUND;
					//OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count+2, TASK_NUM_UP_BOUND));
				}else{
					tmp_task_num2 = hl_task_idx;
					//OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count+2, hl_task_idx));
				}
				OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count+2, tmp_task_num2));
				OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count+3, t));
				if(t==0){
					OCL_CHECK(hl_err, hl_err = hl_kernel.setArg(hl_arg_count+4, *hl_out));
				}

				OCL_CHECK(hl_err,hl_err = host->q_list[1].enqueueMigrateMemObjects(
										  {*hl_task_left,
										  *hl_task_right,},
										  0));
				cl::Event *hl_event = new cl::Event();
				OCL_CHECK(hl_err, hl_err = host->q_list[1].enqueueTask(hl_kernel, NULL, hl_event));
				host->hl_events.push_back(hl_event);

				delete hl_task_left;
				delete hl_task_right;
			}

			if(t < ll_section_count){
				//std::cout << "enqueue ll task " << t << std::endl;
				cl::Buffer *ll_task_left = new cl::Buffer(host->context_list[2], read, sizeof(RAW_ID_TYPE) * ll_task_split_left[t].size(),
												   ll_task_split_left[t].data(),
												   &ll_err);
				cl::Buffer *ll_task_right = new cl::Buffer(host->context_list[2], read, sizeof(RAW_ID_TYPE) * ll_task_split_right[t].size(),
												   ll_task_split_right[t].data(),
												   &ll_err);

				OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count, *ll_task_left));
				OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count+1, *ll_task_right));
				int tmp_task_num3 = 0;
				if(t != ll_section_count - 1){
					tmp_task_num3 = TASK_NUM_UP_BOUND;
					//OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count+2, TASK_NUM_UP_BOUND));
				}else{
					tmp_task_num3 = ll_task_idx;
					//OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count+2, ll_task_idx));
				}
				OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count+2, tmp_task_num3));
				OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count+3, t));
				if(t==0){
					OCL_CHECK(ll_err, ll_err = ll_kernel.setArg(ll_arg_count+4, *ll_out));
				}
				OCL_CHECK(ll_err,ll_err = host->q_list[2].enqueueMigrateMemObjects(
										  {*ll_task_left,
										  *ll_task_right,},
										  0));

				cl::Event *ll_event = new cl::Event();
				OCL_CHECK(ll_err, ll_err = host->q_list[2].enqueueTask(ll_kernel, NULL, ll_event));
				host->ll_events.push_back(ll_event);

				delete ll_task_left;
				delete ll_task_right;
			}

			if(t < hh_section_count){
				//std::cout << "waiting for hh" << std::endl;
				(host->hh_events[host->hh_events.size() - 1])->wait();
			}
			if(t < hl_section_count){
				//std::cout << "waiting for hl" << std::endl;
				(host->hl_events[host->hl_events.size() - 1])->wait();
			}
			if(t < ll_section_count){
				//std::cout << "waiting for ll" << std::endl;
				(host->ll_events[host->ll_events.size() - 1])->wait();
			}
			//std::cout << "iteration " << t << " finished" << std::endl;
		}
		*/
		std::cout << "kernel ends" << std::endl;
		double time_cost =(double) (end.tv_sec - start.tv_sec) * 1000 +
							(end.tv_usec - start.tv_usec) / 1000.0;
		//std::cout << "Time cost = " << time_cost << std::endl;
		uint64_t hh_kernel_time = 0;
		uint64_t hl_kernel_time = 0;
		uint64_t ll_kernel_time = 0;

		std::cout << "*** getting results ***" << endl;
		for(int i = 0; i < hh_kernel_num; i++){
			OCL_CHECK(hh_err,hh_err = host->q_list[0].enqueueMigrateMemObjects({*hh_out[i]}, CL_MIGRATE_MEM_OBJECT_HOST));
		}
		for(int i = 0; i < hl_kernel_num; i++){
			OCL_CHECK(hl_err,hl_err = host->q_list[1].enqueueMigrateMemObjects({*hl_out[i]}, CL_MIGRATE_MEM_OBJECT_HOST));
		}
		for(int i = 0; i < ll_kernel_num; i++){
			OCL_CHECK(ll_err,ll_err = host->q_list[2].enqueueMigrateMemObjects({*ll_out[i]}, CL_MIGRATE_MEM_OBJECT_HOST));
		}

		//OCL_CHECK(hh_err,hh_err = host->q_list[0].enqueueMigrateMemObjects({*hh_out}, CL_MIGRATE_MEM_OBJECT_HOST));
		//OCL_CHECK(hl_err,hl_err = host->q_list[1].enqueueMigrateMemObjects({*hl_out}, CL_MIGRATE_MEM_OBJECT_HOST));
		//OCL_CHECK(ll_err,ll_err = host->q_list[2].enqueueMigrateMemObjects({*ll_out}, CL_MIGRATE_MEM_OBJECT_HOST));

		host->q_list[0].finish();
		host->q_list[1].finish();
		host->q_list[2].finish();

		std::cout << "kernel ends, CPU begins" << std::endl;

		gettimeofday(&start,nullptr);
		LLTriCount_CPU(
					lcsr->offset.data(),lcsr->adj.data(),
					lcsr->offset.data(),lcsr->adj.data(),
					ll_task_split_cpu_left.data(),ll_task_split_cpu_right.data(),
					ll_task_split_cpu_left.size(),
					ll_output_cpu.data()
					);
		HLTriCount_CPU(
					lcsr->offset.data(),lcsr->adj.data(),
					hcsr->offset.data(),hcsr->bid.data(),hcsr->value.data(),
					hl_task_split_cpu_left.data(),hl_task_split_cpu_right.data(),
					hl_task_split_cpu_left.size(),
					hl_output_cpu.data()
					);
		HHTriCount_CPU(
					hcsr->offset.data(),hcsr->bid.data(),hcsr->value.data(),
					hcsr->offset.data(),hcsr->bid.data(),hcsr->value.data(),
					hh_task_split_cpu_left.data(),hh_task_split_cpu_right.data(),
					hh_task_split_cpu_left.size(),
					hh_output_cpu.data()
					);
		gettimeofday(&end,nullptr);
		time_cost =(double) (end.tv_sec - start.tv_sec) * 1000 +
									(end.tv_usec - start.tv_usec) / 1000.0;

		//unsigned long long hh_tc = 0, hl_tc = 0, ll_tc = 0;
		std::cout << "calculating results" << std::endl;

		hh_tc +=  hh_output_cpu[0];
		hl_tc +=  hl_output_cpu[0];
		ll_tc +=  ll_output_cpu[0];
		std::cout << "HH TC = " << hh_tc << std::endl;
		std::cout << "HL TC = " << hl_tc << std::endl;
		std::cout << "LL_TC = " << ll_tc << std::endl;

		for(int i = 0 ; i < hh_kernel_num; i++){
			hh_tc += hh_output_multi[i][0];
			//hh_tc += hh_output[0];
			hh_kernel_time += get_duration_ns(hh_events[i]);
		}
		for(int i = 0 ; i < hl_kernel_num; i++){
			hl_tc += hl_output_multi[i][0];
			hl_kernel_time += get_duration_ns(hl_events[i]);
			//hl_tc += hl_output[0];
		}
		for(int i = 0 ; i < ll_kernel_num; i++){
			ll_tc += ll_output_multi[i][0];
			ll_kernel_time += get_duration_ns(ll_events[i]);
			//ll_tc += ll_output[0];
		}


		//std::cout << "Kernel time = " << kernel_time << std::endl;
		std::cout << "CPU Time cost = " << time_cost << std::endl;
		std::cout << "Kernel time = " << hh_kernel_time / hh_kernel_num << " " << hl_kernel_time / hl_kernel_num << " " << ll_kernel_time / ll_kernel_num << std::endl;
		std::cout << "HH TC = " << hh_tc << std::endl;
		std::cout << "HL TC = " << hl_tc << std::endl;
		std::cout << "LL_TC = " << ll_tc << std::endl;
		std::cout << "Total TC = " << hh_tc + hl_tc + ll_tc << std::endl;
		std::cout << hh_output[1] << " " << hh_output[2] << std::endl;

		delete hh_offset_left;
		delete hh_bid_left;
		delete hh_value_left;
		delete hh_offset_right;
		delete hh_bid_right;
		delete hh_value_right;
		//delete hh_out;
		//delete hh_task_left;
		//delete hh_task_right;

		delete hl_offset_left;
		delete hl_adj_left;
		delete hl_offset_right;
		delete hl_bid_right;
		delete hl_value_right;
		//delete hl_out;
		//delete hl_task_left;
		//delete hl_task_right;

		delete ll_offset_left;
		delete ll_adj_left;
		delete ll_offset_right;
		delete ll_adj_right;
		//delete hl_task_left;
		//delete hl_task_right;

	}
};

#endif
