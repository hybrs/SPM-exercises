#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <chrono>
#include <cstddef>
#include <math.h>
#include <string>
#include <random>
//
// needed a blocking queue
// here is a sample queue
//

template <typename T>
class queue
{
private:
  std::mutex              d_mutex;
  std::condition_variable d_condition;
  std::deque<T>           d_queue;
public:

  queue(std::string s) { std::cout << "Created " << s << " queue " << std::endl;  }
  queue() {}
  
  void push(T value) {
    {
      std::unique_lock<std::mutex> lock(this->d_mutex);
      d_queue.push_front(value);
    }
    this->d_condition.notify_one();
  }
  
  T pop() {
    std::unique_lock<std::mutex> lock(this->d_mutex);
    this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
    T rc = this->d_queue.back();
    this->d_queue.pop_back();
    return rc;
  }
};

class safeInt{
	private:
	  std::mutex              d_mutex;
	  std::condition_variable d_condition;
	  int count = 0;
	public:
	  int incr(){
		  std::unique_lock<std::mutex> lock(this->d_mutex);
		  return ++count;
	  }

	  int get(){
		  std::unique_lock<std::mutex> lock(this->d_mutex);
		  return count;
	  }
};

//
// needed something to represent the EOS
// here we use null
//
#define EOS NULL

// loose some time
void active_delay(int msecs) {
  // read current time
  auto start = std::chrono::high_resolution_clock::now();
  auto end   = false;
  while(!end) {
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    if(msec>msecs)
      end = true;
  }
  return;
}

// print items in a vector, just for check
void print_vec(std::vector<int> v) {
  for(auto it : v)
    std::cout << it << " ";
  std::cout << std::endl;
  return;
}

bool is_prime(int n){
    if(n == 1)
       return false;
    else if(n <= 3)
       return true;
    else if(n%2 == 0 or n%3 == 0)
       return false;
    int i = 5;
    bool found = true;
    while(i * i <= n && found){
       if(n%i == 0 or n%(i + 2) == 0)
           found = false;
       i = i + 6;
    }
    return found;
}

int n_primes(int x){
	int np = 0;
	for(int i = 2; i <= x; i++)
		if(is_prime(i))
			np++;
	return np;
}
