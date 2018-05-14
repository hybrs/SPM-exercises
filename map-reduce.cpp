#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>
#include <cmath> 
#include <condition_variable>
#include <deque>
#include <vector>
#include <chrono>
#include <thread>
#include <cstddef>
#include <string>
#include <algorithm>
#include <exception> 

template <typename T>
class queue{
	private:

	  mutable std::mutex      d_mutex;
	  mutable std::condition_variable d_condition;
	  std::deque<T>           d_queue;

	public:

	  queue(){}

	  /*
	  * Constructor overriding to manage non-movable mutex:
	  *
	  * Move Constructor 
	  */
	  queue(queue&& a)
	    {
	        std::unique_lock<std::mutex> lock(a.d_mutex);
	        //d_condition = std::move(a.d_condition);
	        d_queue = std::move(a.d_queue);
	    }
	  /*
	  * Move Assignment 
	  */
	  queue& operator=(queue&& a)
	    {
	      if (this != &a)
	      {
	        std::unique_lock<std::mutex> lock(d_mutex, std::defer_lock);
	        //d_condition = std::move(a.d_condition);
	        d_queue = std::move(a.d_queue);
	      }
	      return *this;
	    }
	  /*
	  * Copy Constructor
	  */
	  
	  queue(const queue& a)
	    {
	        std::unique_lock<std::mutex> lock(a.d_mutex);
	        //d_condition = a.d_condition;
	        d_queue = a.d_queue;
	    }
	   
	  /*
	  * Copy Assignment 
	  */
	  
	  queue& operator=(const queue& a)
	    {
	        if (this != &a)
	        {
	          std::unique_lock<std::mutex> lock(a.d_mutex, std::defer_lock);
	          //d_condition = a.d_condition;
	          d_queue = a.d_queue;
	        }
	        return *this;
	    }

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

template <typename TKey, typename TVal>
class MapReduce{
	private:

		std::vector<std::pair<TKey, TVal>> (*mapper) (std::string);
		TVal (*reducer) (TVal, TVal);
		std::vector<queue<std::pair<TKey, TVal>>> task_queues;
		std::vector<std::map<TKey, TVal>> result_queues;
		queue<std::string> tasks;
		std::vector<std::thread> threads;
		std::string f_in, f_out;
		//map<TKey, TVal> my_map {key : val}

		void push_item(std::pair<TKey, TVal> el, int nred, int delta){
			
			auto key = el.first;
			int f_letter = key[0];
			int worker = std::abs(((f_letter-97)/delta)%(nred));
			try {
			task_queues[worker].push(el);
			} catch(std::exception &e) {
   					std::cout << "some error occurred with el:"<< std::endl <<
   						worker << std::endl;
   					std::cout << e.what() << std::endl;
   					task_queues[nred-1].push(std::pair<std::string, int>("error", 1));
				}
			//std::cout << "pushed <" << el.first <<
			//", " << el.second << "> in queue #" <<
			//worker << " with fl: "<< f_letter << " delta = "<< delta << "and nred = " << nred << std::endl;
		}

		void mapper_thread(int nred, int delta) {
			while(true){
				std::string s = tasks.pop();
				
				// manage EOS (propagate & terminates)
				if (s.compare("-EOS") == 0) {
					//std::cout << "[in mapper] RECIEVED EOS, TERMINATING." << std::endl;
					for(int i = 0; i < task_queues.size(); i++){
						std::pair<TKey, TVal> p (s, 0);
						task_queues[i].push(p);
					}
				  	return;
				}
				
				auto v = mapper(s);
				
				for(auto el : v)
					push_item(el, nred, delta);
			}
		}

		void reducer_thread(int i, int nmap) {
			std::map<TKey, TVal> dict;
			//std::cout << "[in reducer " << i << "] nmp = " << nmap << std::endl;
			int eos_recieved = 0;
			while(eos_recieved < nmap){
				auto p = task_queues[i].pop();
				//std::cout << "[in reducer " << i << "] pop <" << p.first <<
				//	", " << p.second << ">" << std::endl;
				// manage EOS
				if (/*p.first.compare("-EOS")*/p.second == 0) {
				  eos_recieved++;
				  //std::cout << "[in reducer " << i << "] RECIEVED EOS #" << eos_recieved << std::endl;
				}else{
					dict[p.first] = reducer(dict[p.first], p.second);
				}
			}
			/*
			* After nmap EOS recieved no more mappers 
			* are active so the reducer can terminate 
			*/
			result_queues[i] = dict;
			return;
		}

		void populate_task(int nmap){
			 std::ifstream fi(f_in, std::ios::in);
			 for (std::string line; std::getline(fi, line); ){
			 	//std::string data = std::move(line);
				if(line.size() > 1){
					std::transform(line.begin(), line.end(), line.begin(), ::tolower);
					tasks.push(line);
				}
			 }
			 fi.close();
			 for(int i=0; i<nmap; i++)
			 	tasks.push("-EOS");
			 //std::cout <<"[in populate_task] pushed EOS" << std::endl;
			 return;
		}

	public:

		MapReduce(std::string file_in, std::string file_out) {
			f_in = file_in;
			f_out = file_out;
		}

		void setMapper(std::vector<std::pair<TKey, TVal>> f (std::string)){ this->mapper = f; }

		void setReducer(TVal f (TVal, TVal)){ this->reducer = f; }

		/*
		 * nr :number of reducer threads (max = 26 -> no parallelism inside reducers)
		 * nm : number of mapper threads
		 * */

		void compute(int nm, int nr){
			if(nr > 26)
				nr = 26;
			//task_queues = std::vector<queue<std::pair<TKey, TVal>>>(nr);
			result_queues = std::vector<std::map<TKey, TVal>>(nr);
			threads = std::vector<std::thread>();
			std::map<TKey, TVal> result;
			int delta = 26/nr;
			//std::cout << " start computing with" << nm 
			//	<< " mappers and " << nr << " reducers." << std::endl;
			for(int i=0; i<nr; i++)
				task_queues.push_back(queue<std::pair<TKey, TVal>>());
			//std::cout << nm <<" queues pushed" << std::endl;

			populate_task(nm);

			std::cout <<"[in compute] tasks created" << std::endl;

			// create threads
			for(int i=0; i<nm; i++)
				threads.push_back(std::thread(&MapReduce::mapper_thread, this, nr, delta));
			for(int i=0; i<nr; i++)
				threads.push_back(std::thread(&MapReduce::reducer_thread,this, i, nm));

			//std::cout << (nm+nr) <<"[in compute] threads pushed" << std::endl;

			// await termination
			for(auto &th : threads)
				th.join();
			std::cout << "[in compute] Threads terminated" << std::endl;
			//Merge results
			for(std::map<TKey, TVal> mapp : result_queues)
				result.insert(mapp.begin(),mapp.end());
			//Print the result
			std::ofstream out_file;
			out_file.open (f_out);
			for (auto el = result.begin(); el != result.end(); ++el)
				out_file << "<" << el->first << ", " << el->second << ">" << std::endl;
			out_file.close();
			return;
		}

};