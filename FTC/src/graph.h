#ifndef GRAPH_H
#define GRAPH_H

#include<vector>
#include<algorithm>
#include<set>
#include<string>
#include<fstream>
#include<iostream>
#include<map>
#include "kernels.h"
#include "../lib/xcl2.hpp"
using namespace std;

class Vertex{
public:
	int degree;
	RAW_ID_TYPE id;

	bool operator < (const Vertex& b) const{
		return this->degree < b.degree;
	}

	bool operator > (const Vertex& b) const{
		return this->degree > b.degree;
	}

};

class Graph{
public:
	vector<set<RAW_ID_TYPE>> edges;
	vector<Vertex> degrees;
	int vertex_num;
	int maxDegree = -1;
	int minDegree = 1000000000;
	unsigned long long degreeSum = 0;

	void readCompressedGraph(string path){
		cout << "[Construct Graph From File]" << endl;
		ifstream graph_file;
		graph_file.open(path,ios::binary);

		if(!graph_file){
			cout << "graph file opening error!" << endl;
			return;
		}

		RAW_ID_TYPE src,dst;
		vertex_num = 0;

		graph_file.read((char*)&vertex_num,sizeof(unsigned int));
		cout << "Vertex num: " << vertex_num << endl;

		for(int i = 0; i < vertex_num ; i++){
			edges.emplace_back();
		}

		while(1){
			if(!graph_file.read((char * )&src, sizeof(RAW_ID_TYPE))) break;
			if(!graph_file.read((char * )&dst, sizeof(RAW_ID_TYPE))) break;
			if(src > dst) swap(src,dst);
			if(src == dst){
				continue;
			}
			edges[src].insert(dst);
		}

		graph_file.close();

		for(int i = 0; i < vertex_num; i++){
			Vertex tmp;
			tmp.degree = edges[i].size();
			tmp.id = i;
			degrees.push_back(tmp);
		}

		sort(degrees.begin(),degrees.end(),greater<Vertex>());

		cout << "top 10 vertex:" << endl;
		for(int i = 0; i < 10; i++){
			cout << degrees[i].id << "->" << degrees[i].degree << endl;
		}

		cout << "size of edges: " << edges.size() << endl;

		cout << "[Finish Graph Construction]" << endl << endl;

		//printGraph();
		printDetail();
	}


	void readGraph(string path){
		cout << "[Construct Graph From File]" << endl;
		ifstream graph_file;
		graph_file.open(path,ios::in|ios::out);

		if(!graph_file){
			cout << "graph file opening error!" << endl;
			return;
		}

		RAW_ID_TYPE src,dst;
		vertex_num = 0;

		graph_file >> vertex_num;
		cout << "Vertex num: " << vertex_num << endl;

		for(int i = 0; i < vertex_num ; i++){
			edges.emplace_back();
		}

		while(graph_file >> src >> dst){
			if(src > dst) swap(src,dst);
			edges[src].insert(dst);
		}

		graph_file.close();

		for(int i = 0; i < vertex_num; i++){
			Vertex tmp;
			tmp.degree = edges[i].size();
			tmp.id = i;
			degrees.push_back(tmp);
		}

		sort(degrees.begin(),degrees.end(),greater<Vertex>());

		cout << "[Finish Graph Construction]" << endl << endl;

		//printGraph();
		printDetail();
	}

	void printGraph()
	{
		cout << "[Print Graph]" << endl;
		for(int i = 0; i < vertex_num ; i++){
			cout << i << " -> ";
			for(auto& item : edges[i]){
				cout << item << " ";
			}
			cout << endl;
		}

		for(int i = 0; i < vertex_num; i++){
			cout << degrees[i].id << " -- " << degrees[i].degree << endl;
		}

		cout << "[Print Graph End]" << endl << endl;
	}

	void printDetail(){
		cout << "[Print Graph Detail]" << endl;
		maxDegree = 0;
		minDegree = 1000000000;
		degreeSum = 0;
		for(int i = 0; i < vertex_num ; i++){
			if(edges[i].size() > maxDegree)
				maxDegree = edges[i].size();
			if(edges[i].size() < minDegree)
				minDegree = edges[i].size();
			degreeSum += edges[i].size();
		}
		cout << "max degree = " << maxDegree << endl;
		cout << "min degree = " << minDegree << endl;
		cout << "average degree = " << degreeSum / vertex_num << endl;



	}

	void splitGraph(Graph& light, Graph& heavy,vector<bool>& is_heavy,
					map<RAW_ID_TYPE,RAW_ID_TYPE>& id_ori2light,map<RAW_ID_TYPE,RAW_ID_TYPE>& id_ori2heavy,
					int heavy_num)
	{

		//initialize light graph and heavy graph
		light.edges.clear();
		light.degrees.clear();
		heavy.edges.clear();
		heavy.degrees.clear();
		id_ori2light.clear();
		id_ori2heavy.clear();
		is_heavy.resize(this->vertex_num);
		for(int i = 0; i < vertex_num; i++){
			is_heavy[i] = false;
		}

		heavy.vertex_num = heavy_num;
		light.vertex_num = this->vertex_num - heavy.vertex_num;

		for(int i = 0; i < heavy.vertex_num ; i++){
			heavy.edges.emplace_back();
		}
		for(int i = 0; i < light.vertex_num ; i++){
			light.edges.emplace_back();
		}
		for(int i = 0 ; i < heavy.vertex_num; i++){
			heavy.edges[i] = this->edges[this->degrees[i].id];
			id_ori2heavy[this->degrees[i].id] = i;
			is_heavy[this->degrees[i].id] = true;
			Vertex tmp;
			tmp.degree = heavy.edges[i].size();
			tmp.id = i;
			heavy.degrees.push_back(tmp);
		}
		sort(heavy.degrees.begin(),heavy.degrees.end(),greater<Vertex>());

		for(int i = 0 ; i < light.vertex_num; i++){
			light.edges[i] = this->edges[this->degrees[i + heavy.vertex_num].id];
			id_ori2light[this->degrees[i + heavy.vertex_num].id] = i;
			Vertex tmp;
			tmp.degree = light.edges[i].size();
			tmp.id = i;
			light.degrees.push_back(tmp);
		}
		sort(light.degrees.begin(),light.degrees.end(),greater<Vertex>());

		/*
		cout << "Light Graph:" << endl;
		light.printGraph();
		cout << "Heavy Graph" << endl;
		heavy.printGraph();
		*/
	}

};

class LightCSR{
public:
	vector<RAW_ID_TYPE, aligned_allocator<RAW_ID_TYPE> > offset;
	vector<RAW_ID_TYPE, aligned_allocator<RAW_ID_TYPE> > adj;
	int vertex_num;

	LightCSR(Graph& graph):vertex_num(graph.vertex_num){
		offset.push_back(0);
		for(auto item : graph.edges[0]){
			adj.push_back(item);
		}
		for(int i = 1 ; i < graph.vertex_num ; i++){
			offset.push_back(offset[i-1] + graph.edges[i - 1].size());
			for(auto item : graph.edges[i]){
				adj.push_back(item);
			}
		}
		offset.push_back(offset[graph.vertex_num-1] + graph.edges[graph.vertex_num - 1].size());
		//offset.push_back(offset[graph.vertex_num-1] + graph.edges[graph.vertex_num - 1].size());
		cout << "size of offset: " << offset.size() << endl;
		cout << "size of adj: " << adj.size() << endl;
		cout << "average degree: " << adj.size() / offset.size() << endl;
	}

	// copy construct
	LightCSR(LightCSR& parent): offset(parent.offset), adj(parent.adj){

	}

	void printCSR(){
		cout << "[Offset]" << endl;
		for(auto item : offset){
			cout << item << " ";
		}
		cout << endl;

		cout << "[Adj]" << endl;
		for(auto item : adj){
			cout << item << " ";
		}
		cout << endl;
	}

	void getAdjGivenVertex(RAW_ID_TYPE id){
		cout << "Getting adj of vertex " << id << endl;
		cout << offset[id] << endl;
		cout << offset[id+1] << endl;
		int size = offset[id+1] - offset[id];
		/*
		for(int j = 0; j < size; j++){
			cout << adj[j + offset[id]] << " ";
		}
		*/
		cout << endl;
	}

	void testMergeTwo(RAW_ID_TYPE a, RAW_ID_TYPE b){
		static RAW_ID_TYPE adj_buffer_left[ADJ_NUM_UP_BOUND];
		static RAW_ID_TYPE adj_buffer_right[ADJ_NUM_UP_BOUND];
		auto id_left = a;
		auto id_right = b;

		std::cout << "dealing with " << id_left << " & " << id_right << std::endl;

		auto adj_begin_left = offset[id_left];
		auto adj_begin_right = offset[id_right];

		auto adj_end_left = offset[id_left + 1];
		auto adj_end_right = offset[id_right + 1];

		auto adj_size_left = adj_end_left - adj_begin_left;
		auto adj_size_right = adj_end_right - adj_begin_right;

		for(int li = 0; li < adj_size_left; li++){
			adj_buffer_left[li] = adj[adj_begin_left + li];
		}

		for(int ri = 0; ri < adj_size_right; ri++){
			adj_buffer_right[ri] = adj[adj_begin_right + ri];
		}

		std::cout << "adj loaded" << std::endl;

		auto value_left = adj_buffer_left[0];
		auto value_right = adj_buffer_right[0];
		bool is_equal;
		bool is_greater;
		int res = 0;

		DO_MERGE: for(int li = 0, ri = 0; li < adj_size_left && ri < adj_size_right;){
			value_left = adj_buffer_left[li];
			value_right = adj_buffer_right[ri];
			std::cout<<"merging " << value_left << " and " << value_right << endl;
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
		cout << "res = " << res << endl;

	}
};

class HeavyCSR{
public:
	vector<RAW_ID_TYPE,aligned_allocator<RAW_ID_TYPE> > offset;
	vector<BID_TYPE,aligned_allocator<BID_TYPE> > bid;
	vector<VID_TYPE,aligned_allocator<VID_TYPE> > value;
	int vertex_num;

	HeavyCSR(Graph& graph):vertex_num(graph.vertex_num){
		cout << "[Begin Heavy CSR Construct]" << endl;
		int bcount = 0;
		cout << vertex_num << endl;
		for(int i = 0; i < vertex_num ; i++){
			offset.push_back(bcount);
			if(graph.edges[i].size() > 0){
				auto iter = graph.edges[i].begin();
				BID_TYPE b = 0;
				VID_TYPE v = 0;
				int v_width = sizeof(v) * 8;
				b = *iter / v_width;
				v = v | (1 << (v_width - *iter%v_width - 1));
				iter++;
				for(;iter!=graph.edges[i].end();iter++){
					//cout << "hello" << endl;
					BID_TYPE tmpb = *iter / v_width;
					if(tmpb == b){
						v = v | (1 << (v_width - *iter%v_width - 1));
					}else{
						bid.push_back(b);
						value.push_back(v);
						b = tmpb;
						v = (1 << (v_width - *iter%v_width - 1));
						bcount++;
					}
				}
				bid.push_back(b);
				value.push_back(v);
				bcount++;
			}
		}
		offset.push_back(bcount);
		//offset.push_back(bcount);
		cout << "size of offset: " << offset.size() << endl;
		cout << "size of block: " << bid.size() << endl;
		cout << "average degree: " << bid.size() / offset.size() << endl;
	}

	template<typename T>
	string toStringBinary(T num){
		int bits = sizeof(num) * 8;
		string tmp;
		while(bits--){
			tmp += to_string(num&1);
			num = num >> 1;
		}
		reverse(tmp.begin(),tmp.end());
		return tmp;
	}

	void printCSR(){
		cout << "[Offset]" << endl;
		for(auto item : offset){
			cout << item << " ";
		}
		cout << endl;

		cout << "[BID]" << endl;
		for(auto item : bid){
			cout << toStringBinary<BID_TYPE>(item) << " ";
		}
		cout << endl;

		cout << "[VALUE]" << endl;
		for(auto item : value){
			cout << toStringBinary<VID_TYPE>(item) << " ";
		}
		cout << endl;
	}

};

#endif
