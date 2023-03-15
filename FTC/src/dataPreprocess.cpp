#include<iostream>
#include<fstream>
#include<set>
#include<string>
#include<cstdlib>
#include<algorithm>
#include<unordered_map>
#include "kernels.h"
using namespace std;
void getVertexAndEdgeNum(string path){
	ifstream fs;
	ofstream ofs;
	fs.open(path,ios::binary);
	if(!fs){
		cout << "graph file opening error!" << endl;
		return;
	}
	cout << fs.is_open() << endl;
	RAW_ID_TYPE src = 0,dst = 0;
	set<RAW_ID_TYPE> vertex;
	unsigned long long count = 0;
	while(1){
		if(!fs.read((char * )&src, sizeof(RAW_ID_TYPE))) break;
		if(!fs.read((char * )&dst, sizeof(RAW_ID_TYPE))) break;
		count++;
		cout << src << " " << dst << endl;
		if(count>10) break;
		vertex.insert(src);
		vertex.insert(dst);
	}
	cout << "Edge num: " << count << endl;
	cout << "Vertex num: " << vertex.size() << endl;
	cout << "Largest id: " << *(vertex.rbegin()) << endl;
	fs.close();
	ofs.close();
	return;
}

void graphFormatCompressed(string path,string outpath){
	ifstream fs;
	ofstream ofs;
	fs.open(path,ios::binary);
	ofs.open(outpath,ios::binary);
	if(!fs){
		cout << "graph file opening error!" << endl;
		return;
	}
	cout << fs.is_open() << endl;
	RAW_ID_TYPE src = 0,dst = 0;
	set<RAW_ID_TYPE> vertex;
	unordered_map<RAW_ID_TYPE,RAW_ID_TYPE> compressed_vertex;
	unsigned long long count = 0;
	while(1){
		if(!fs.read((char * )&src, sizeof(RAW_ID_TYPE))) break;
		if(!fs.read((char * )&dst, sizeof(RAW_ID_TYPE))) break;
		count++;
		vertex.insert(src);
		vertex.insert(dst);
		//if(count>500) break;
	}
	cout << "Loaded:" << endl;
	cout << "Edge num: " << count << endl;
	cout << "Vertex num: " << vertex.size() << endl;
	cout << "Largest id: " << *(vertex.rbegin()) << endl;

	fs.clear();
	fs.seekg(ios::beg);
	RAW_ID_TYPE compressed_id = 0;

	for(auto item : vertex){
		compressed_vertex[item] = compressed_id;
		compressed_id++;
		//cout << item << " " << compressed_vertex[item] << endl;
	}
	cout << "begin writing" << endl;
	auto k = vertex.size();
	ofs.write((char * )&k, sizeof(RAW_ID_TYPE));
	count = 0;
	RAW_ID_TYPE csrc,cdst;
	while(1){
		if(!fs.read((char * )&src, sizeof(RAW_ID_TYPE))) break;
		if(!fs.read((char * )&dst, sizeof(RAW_ID_TYPE))) break;
		count++;
		csrc = compressed_vertex[src];
		cdst = compressed_vertex[dst];
		ofs.write((char * )&csrc, sizeof(RAW_ID_TYPE));
		ofs.write((char * )&cdst, sizeof(RAW_ID_TYPE));
		//ofs << compressed_vertex[src] << " " << compressed_vertex[dst] << endl;
		//if(count>10) break;
	}
	fs.close();
	ofs.close();
	return;
}

void graphNTFormat(string path,string outpath){
	ifstream fs;
	ofstream ofs;
	fs.open(path,ios::binary);
	ofs.open(outpath);
	if(!fs){
		cout << "graph file opening error!" << endl;
		return;
	}
	cout << fs.is_open() << endl;
	RAW_ID_TYPE src = 0,dst = 0;
	set<RAW_ID_TYPE> vertex;
	unordered_map<RAW_ID_TYPE,RAW_ID_TYPE> compressed_vertex;
	unsigned long long count = 0;
	while(1){
		if(!fs.read((char * )&src, sizeof(RAW_ID_TYPE))) break;
		if(!fs.read((char * )&dst, sizeof(RAW_ID_TYPE))) break;
		count++;
		vertex.insert(src);
		vertex.insert(dst);
		//if(count>500) break;
	}
	cout << "Loaded:" << endl;
	cout << "Edge num: " << count << endl;
	cout << "Vertex num: " << vertex.size() << endl;
	cout << "Largest id: " << *(vertex.rbegin()) << endl;

	fs.clear();
	fs.seekg(ios::beg);
	RAW_ID_TYPE compressed_id = 0;

	for(auto item : vertex){
		compressed_vertex[item] = compressed_id;
		compressed_id++;
		//cout << item << " " << compressed_vertex[item] << endl;
	}
	cout << "begin writing" << endl;
	auto k = vertex.size();
	vector<vector<int>> adj_lists(k);
	count = 0;
	RAW_ID_TYPE csrc,cdst;
	int label_total = 32;
	int label = 0;
	vector<int> label_count(label_total,0);
	while(1){
		if(!fs.read((char * )&src, sizeof(RAW_ID_TYPE))) break;
		if(!fs.read((char * )&dst, sizeof(RAW_ID_TYPE))) break;
		count++;
		csrc = compressed_vertex[src];
		cdst = compressed_vertex[dst];
		if(csrc > cdst){
			adj_lists[cdst].push_back(csrc);
		}else{
			adj_lists[csrc].push_back(cdst);
		}
		//ofs.write((char * )&csrc, sizeof(RAW_ID_TYPE));
		//ofs.write((char * )&cdst, sizeof(RAW_ID_TYPE));
		//ofs << compressed_vertex[src] << " " << compressed_vertex[dst] << endl;
		//if(count>10) break;
	}
	cout << "writing v" << endl;
	ofs << "t 1 " << k << endl;
	//ofs << k << " " << count << " " << label_total << " " << 1 << endl;
	for(int i = 0; i < k; i++){
		label = rand() % label_total;
		label_count[label]++;
		ofs << "v " << i << " " << label + 1 << endl;
	}

	cout << "writing e" << endl;
	for(int i = 0; i < k; i++){
		sort(adj_lists[i].begin(),adj_lists[i].end());
		for(int j = 0; j < adj_lists[i].size(); j++){
			ofs << "e " << i << " " << adj_lists[i][j] << " 0" << endl;
		}
	}

	fs.close();
	ofs.close();

	for(int i = 0 ; i < label_total ; i++){
		cout << "Number of label " << i << " is " << label_count[i] << endl;
	}
	return;
}

void graphFormat(string path,string outpath){
	ifstream fs;
	ofstream ofs;
	fs.open(path,ios::binary);
	ofs.open(outpath,ios::binary);
	if(!fs){
		cout << "graph file opening error!" << endl;
		return;
	}
	cout << fs.is_open() << endl;
	RAW_ID_TYPE src = 0,dst = 0;
	set<RAW_ID_TYPE> vertex;
	unordered_map<RAW_ID_TYPE,RAW_ID_TYPE> compressed_vertex;
	unsigned long long count = 0;
	while(1){
		if(!fs.read((char * )&src, sizeof(RAW_ID_TYPE))) break;
		if(!fs.read((char * )&dst, sizeof(RAW_ID_TYPE))) break;
		count++;
		vertex.insert(src);
		vertex.insert(dst);
		//if(count>500) break;
	}
	cout << "Loaded:" << endl;
	cout << "Edge num: " << count << endl;
	cout << "Vertex num: " << vertex.size() << endl;
	cout << "Largest id: " << *(vertex.rbegin()) << endl;

	fs.clear();
	fs.seekg(ios::beg);

	cout << "begin writing" << endl;
	auto k = *(vertex.rbegin())+1;
	ofs.write((char * )&k, sizeof(RAW_ID_TYPE));
	count = 0;
	while(1){
		if(!fs.read((char * )&src, sizeof(RAW_ID_TYPE))) break;
		if(!fs.read((char * )&dst, sizeof(RAW_ID_TYPE))) break;
		count++;
		ofs.write((char * )&src, sizeof(RAW_ID_TYPE));
		ofs.write((char * )&dst, sizeof(RAW_ID_TYPE));
		//ofs << compressed_vertex[src] << " " << compressed_vertex[dst] << endl;
		//if(count>10) break;
	}
	fs.close();
	ofs.close();
	return;
}



int main(int argc, char **argv){
	//getVertexAndEdgeNum("../../data/cit-Patents.bin");
	graphNTFormat(argv[5],argv[4]);
	return 0;
}










