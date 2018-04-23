//============================================================================
// Name        : es2.cpp
// Author      : Mario Salinas
// Version     :
// Copyright   :
// Description : implement, using only C++ standard mechanisms and threads,
//				 a program that computes in parallel a set of independent tasks,
// 				 initially stored in a shared data structure, and delivers
// 				 results using a second shared data structure. The number of
//				 tasks to be computed is known and accessible to the parallel
//				 executors, as well as the number of tasks already
//				 computed/to compute. Assume an input task is given by an
//				 integer number N and the result to compute is the number
//				 of prime numbers included in range [1-N]. The initial set of
//				 tasks is picked up randomly in the range [1-10K]. Once the
//				 problem is working, add some delay in the procedure obtaining
//				 a task to be computed and delivering a result and observe
//				 the impact on scalability.
//============================================================================
#include <iostream>
#include <random>
#include <mutex>
#include <algorithm>
#include <thread>
#include <condition_variable>
#include <deque>
#include <vector>
#include <chrono>
#include <cstddef>
#include <math.h>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "util.cpp"

using namespace std;

int main(int argc, char * argv[]) {

  if(argc == 1) {
    cout << "Usage is: " << argv[0] << " n-task delay-ms-obtain delay-ms-deliver" << endl;
    return(0);
  }
  srand (time(NULL));
  int n_task = atoi(argv[1]);
  int ms_obtain= atoi(argv[2]);
  int ms_deliver = atol(argv[3]);

  vector<int>tasks(n_task);
  vector<int>completed_tasks(n_task);

  auto start   = chrono::high_resolution_clock::now();

  cout << "Source running \n"
	    << "generating." << endl;
  for(int i=0; i<n_task; i++)
    tasks[i] = rand()%10000 + 1;

  cout << "Generated.";
  for(int i=0; i<n_task; i++) {
    // blocking read from input queue
  	active_delay(ms_obtain);

	// then deliver the result  to the next stage
	active_delay(ms_deliver);
	completed_tasks[i] = n_primes(tasks[i]);
  }

  auto elapsed = chrono::high_resolution_clock::now() - start;
  auto msec    = chrono::duration_cast<chrono::milliseconds>(elapsed).count();

  cout << "Elapsed time " << msec << " msecs - inputsize " << n_task
		  << " - ms-obt "<< ms_obtain << " - ms-del " << ms_deliver << endl;
  return(0);
}
