//==========================================================================================================
// Name        : es1.cpp
// Author      : Mario Salinas
// Version     :
// Copyright   : 
// Description : pipeline an random vectors
// Performance : 18202690 usecs - 4 stages pipeline - input size 80000000 - on average 182026 usecs
//==========================================================================================================

#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unistd.h>

unsigned int microseconds = 2;

using namespace std;

int main(int argc, char * argv[]) {
  	string text = "";
	double lower_bound = 0;
	double upper_bound = 10000;
	std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
	std::default_random_engine re;
	int n = atoi(argv[1]); int m = atoi(argv[2]), tot = n*m;
	vector<vector<double>> data(m);

	auto usecTot = 0; int NEXES = 100;
	for(int exes = 0; exes < NEXES; exes++){
		auto start = std::chrono::high_resolution_clock::now();

		//Stage1: fill with random doubles
		for(int i = 0; i <m; i++){
			data[i].resize(n);
			for (int j = 0; j < n; j++)
				data[i][j]= unif(re);
		}
		//Stage2: increments
		for(int i = 0; i <m; i++)
			for (int j = 0; j < n; j++)
				data[i][j]++;
		//Stage3: x2
		for(int i = 0; i <m; i++)
			for (int j = 0; j < n; j++)
				data[i][j]*=2;
		//Stage4:print

		for(int i = 0; i <m; i++){
			//cout<<"vect-"<<i<<": ";
			for (int j = 0; j < n; j++)
				 j = j + 1 -1;
		}
		auto elapsed = std::chrono::high_resolution_clock::now() - start;
		auto usec    = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		// and print time
		cout << "[" << exes+1 << "/" <<NEXES<< "] Spent " << usec 
			<< " usecs to process in pipeline with 4 stages" <<  " input size " << (n*m) << endl;
		usecTot += usec;
	}

	cout << "Spent " << usecTot 
	     << " usecs to process in pipeline with 4 stages" 
	     <<  " input size " << (n*m*NEXES) << ", on average " 
	     << (usecTot/NEXES) << endl;
	return(0);
}
