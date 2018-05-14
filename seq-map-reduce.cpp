#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <cstddef>
#include <string>
#include <algorithm> 

auto mapper = [](std::string s){
	std::vector<std::pair<std::string, int>> v;
	std::stringstream ss(s);
	std::string w;
	while(ss >> w){
		std::pair<std::string, int>p(w, 1);
		v.push_back(p);
		//std::cout << "pushed " <<
		// p.first << " - " << p.second << std::endl;
	}
	return v;
};

auto reducer = [](int a, int b){return a+b;};

int main(int argc, char * argv[]) {

	if(argc <= 2) {
		std::cout << "Usage is: " << argv[0] << " fIn fOut " << std::endl;
		return(0);
	}
	std::string fIn = argv[1];
	std::string fOut = argv[2];

	std::map<std::string, int> result;

	std::cout << "Sequential computation started..." << std::endl;

	auto start   = std::chrono::high_resolution_clock::now();

	std::ifstream fi(fIn, std::ios::in);
	for (std::string line; std::getline(fi, line); ){
		std::string data = line, res;
		std::transform(data.begin(), data.end(), data.begin(), ::tolower);
		//Tokenization
		auto v = mapper(data);
		//Reduction
		for(auto el : v)
			result[el.first] = reducer(result[el.first], el.second);
	}
	fi.close();

	//Print the result on file
	std::ofstream out_file;
	out_file.open (fOut);
	for (auto el = result.begin(); el != result.end(); ++el)
		out_file << "<" << el->first << ", " << el->second << ">" << std::endl;
	out_file.close();
	
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

	std::cout << "[Sequential] Elapsed time " << msec << " msecs" << std::endl;
	//std::cout << "[in main] Computation terminated" << std::endl;
	return(0);
	}