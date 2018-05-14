//============================================================================
// Name        : es3.cpp
// Author      : Mario Salinas
// Description : Using C++ only, implement an object providing the programmer
//				 with a parallel google mapreduce pattern. The user must be
//				 able to provide two filenames (an input and an output file)
//				 the mapper (function<vector<pair<Tkey,Tvalue>>(string)>)
//				 processing lines of the file, the reducer
//				 (function<Tvalue(Tvalue,Tvalue)>) and a method void compute(void)
//				 to compute the results in the output file out of the lines
//				 of the input file.
//============================================================================
#include "map-reduce.cpp"

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

auto reducer = [](int a, int b){ return a+b;};

int main(int argc, char * argv[]) {

	if(argc <= 4) {
		std::cout << "Usage is: " << argv[0] << " fIn fOut n-mapper n-reducer" << std::endl;
		return(0);
	}
	std::string fIn = argv[1];
	std::string fOut = argv[2];
	int nm = atoi(argv[3]);
	int nr = atoi(argv[4]);
	
	MapReduce<std::string, int> mr(argv[1], argv[2]);
    std::cout << "[in main] mapreduce created" << std::endl;

	mr.setMapper(mapper);
	std::cout << "[in main] mapper set,";
	mr.setReducer(reducer);
	std::cout << " reducer set, start computing..." << std::endl;

	auto start   = std::chrono::high_resolution_clock::now();

	mr.compute(nm, nr);

	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	auto msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

	std::cout << "Elapsed time " << msec << " msecs" << 
		" - n_mapper " << nm << " - n_reducer " << nr << std::endl;
	return(0);
	}