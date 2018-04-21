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

// queues to host messages from one stage to the next one.
// four stages => three queues
queue<int> toWork, toDeliver;
vector <int> tasks, completed_tasks;
safeInt delivered_tasks;

void Source(int n_task, int nw) {
  cout << "Source running \n"
	    << "generating: " << endl;
  for(int i=0; i<n_task; i++) {
    tasks[i] = rand()%10000 + 1;
    toWork.push(i);
  }
  cout << "Generated: ";
  //print_vec(tasks);
  // terminate all the workers
  for(int i=0; i<nw; i++)
    toWork.push(EOS);
  return;
}



void Worker(int ms_obt, int ms_del) {
  while(true) {
    // blocking read from input queue
	active_delay(ms_obt);
    int v = toWork.pop();

    // manage EOS (propagate & terminate)
    if(v == EOS) {
      toDeliver.push(v);
      return;
    }

    // then deliver the result  to the next stage
    active_delay(ms_del);
    completed_tasks[v] = n_primes(tasks[v]);
    toDeliver.push(v);
    delivered_tasks.incr();
  }
  return;
}


void Drain(int nw) {
  while(true) {
    // blocking read from input queue
    int v = toDeliver.pop();

    // manage EOS: get one EOS per worker
    if(v == EOS && nw == 1)  {
      cout << "Terminating ... " << endl;
      //print_vec(completed_tasks);
      return;
    }
    if(v == EOS) {
      //cout << "got EOS " << nw << endl;
      --nw;
    }
  }
  return;
}


int main(int argc, char * argv[]) {

  if(argc == 1) {
    cout << "Usage is: " << argv[0] << " n-task delay-ms-obtain delay-ms-deliver nw" << endl;
    return(0);
  }

  int n_task = atoi(argv[1]);
  int ms_obtain= atoi(argv[2]);
  int ms_deliver = atol(argv[3]);
  int nw = atol(argv[4]);

  vector<thread> threads;
  tasks = vector<int>(n_task);
  completed_tasks = vector<int>(n_task);
  srand (time(NULL));
  auto start   = chrono::high_resolution_clock::now();
  // create threads
  threads.push_back(thread(Source, n_task, nw));
  for(int i=0; i<nw; i++)
    threads.push_back(thread(Worker, ms_obtain, ms_deliver));
  threads.push_back(thread(Drain,nw));

  // await termination
  for(auto &th : threads)
    th.join();

  auto elapsed = chrono::high_resolution_clock::now() - start;
  auto msec    = chrono::duration_cast<chrono::milliseconds>(elapsed).count();

  cout << "Elapsed time is " << msec << " msecs with inputsize " << n_task
		  << " ms-obt is "<< ms_obtain << " ms-del is " << ms_deliver
		  << " and numb_w " << nw << endl;
  return(0);
}
