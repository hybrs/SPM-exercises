//==========================================================================================================
// Name        : es1.cpp
// Author      : Mario Salinas
// Version     :
// Copyright   : 
// Description : pipeline on random-double vectors
// Performance : 93492126 usecs - 4 stages pipeline - input size 80000000 - on average 934921 usecs
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
#include <string>

int NEXES = 100;

using namespace std;
string text = "", text1 = "";
class SafeQueue {
	private:
		queue<double> q;
		mutex m;
		condition_variable cv;

	public:

	    void push(double elem) {
	        unique_lock<mutex> lock(m);
	        q.push(elem);
	        cv.notify_one();
	    }

	    double next() {
	    	double elem;
	    	unique_lock<mutex> lock(m);
			while(q.empty())
				cv.wait(lock);
			elem = q.front();
			q.pop();
	    	return elem;
	    }
};


SafeQueue q1, q2, q3;

void stage1(int n, int m){
	double lower_bound = 0;
	double upper_bound = 10000;
	uniform_real_distribution<double> unif(lower_bound,upper_bound);
	default_random_engine re;
	for(int i = 0; i < m; i++){
		auto iS = to_string(i);
		text1.append("[A"+iS+"]");
		text1.append("\n");
		for (int j = 0; j < n; j++){
			double el = unif(re);
			q1.push(el);
			auto d = to_string(el);
			text1.append(d);
			text1.append("\n");
		}
		//cout << "[stage1] array " << i << " sent." << endl;
	}
}

void stage2(int n, int m){
	for(int i = 0; i < m; i++){
		//cout << "[stage2] array " << i << " recieved." << endl;
		for (int j = 0; j < n; j++){
			double el = q1.next();
			el++;
			q2.push(el);
		}
		//cout << "[stage2] array " << i << " sent." << endl;
	}

}

void stage3(int n, int m){
	for(int i = 0; i < m; i++){
		//cout << "[stage3] array " << i << " recieved." << endl;
		for (int j = 0; j < n; j++){
			double el = q2.next();
			el*=2;
			q3.push(el);
		}
		//cout << "[stage3] array " << i << " sent." << endl;
	}
}


void stage4(int n, int m){
	for(int i = 0; i < m; i++){
		auto iS = to_string(i);
		//cout << "[stage4] array " << i << " recieved." << endl;
		text.append("[A"+iS+"]");
		text.append("\n");
		for (int j = 0; j < n; j++){
			double el = q3.next();
			auto s = to_string(el);
			text.append(s);
			text.append("\n");
		}
	}
}


int main(int argc, char * argv[]) {
	string filename = "data";
  	ifstream fd(filename);   // open output file
	double lower_bound = 0;
	double upper_bound = 10000;
	uniform_real_distribution<double> unif(lower_bound,upper_bound);
	default_random_engine re;
	int n = atoi(argv[1]); int m = atoi(argv[2]), tot = n*m;
	
	auto compute_vect = [&](int i) {   // function to compute a chunk
		switch (i){
			case 0:
				stage1(n, m);
			break;
			case 1:
				stage2(n, m);
			break;
			case 2:
				stage3(n, m);
			break;
			case 3:
				stage4(n, m);
		}
    };


	// pipeline with 4 stages
	auto usecTot = 0;
	vector<thread> tids; int nw = 4;

	for(int exes = 0; exes < NEXES; exes++){
		auto start = chrono::high_resolution_clock::now();
		for(int i=0; i<nw; i++)   
				tids.push_back(thread(compute_vect,i));

		for(thread& t: tids)
			t.join();

		for(int i=0; i<nw; i++)
			tids.pop_back();

		auto elapsed = chrono::high_resolution_clock::now() - start;
		auto usec = chrono::duration_cast<chrono::microseconds>(elapsed).count();
		cout << "[" << exes+1 << "/" <<NEXES<< "] Spent " << usec 
			<< " usecs to process in pipeline with 4 stages" <<  " input size " << (n*m) << endl;
		usecTot += usec;
	}


	cout << "Spent " << usecTot << " usecs to process in pipeline with 4 stages" <<  " input size " << (n*m*NEXES) << ", on average " << (usecTot/NEXES) << endl;
	cout << "Writing output log..." << endl;

	// write result to file
	ofstream fdo(filename);

	fdo << text1;
	fdo.close();

	filename.append("_stage4.txt");
	ofstream fdo4(filename);

	fdo4 << text;
	fdo4.close();

	return(0);
}
