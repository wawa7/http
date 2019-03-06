#ifndef __THREADTOOL_TOOL__
#define __THREADTOOL_TOOL__
#include<queue>
#include"Log.hpp"
#include<pthread.h>

#define NUM 5
typedef int(*Handler)(int);

class Task{
	private:
		int sock;
		Handler handler;
	public:
		Task(int sock_=-1,Handler handler_=NULL):sock(sock_),handler(handler_)
		{}

		void Run(){
			if(handler == NULL)
				return;
			std::cout<<"sock : "<<sock<<std::endl;
			handler(sock);
		}

};	

class ThreadPool{
	private:
		int thread_nums;
		int thread_idle_nums;
		std::queue<Task> thread_queue;
		bool is_quit;
		pthread_mutex_t mutex;
		pthread_cond_t cond;
	public:
		ThreadPool(int thread_nums_=NUM, int thread_idle_nums_=0)
			:thread_nums(thread_nums_)
			,thread_idle_nums(thread_idle_nums)
			,is_quit(false)
		{}

		void Lock(){
			pthread_mutex_lock(&mutex);	
		}

		void Unlock(){
			pthread_mutex_unlock(&mutex);
		}

		bool IsEmpty(){
			return thread_queue.size() == 0;
		}

		void ThreadIdle(){
			if(is_quit){
				Unlock();
				LOG(INFO,"thread exit");
				thread_nums--;
				pthread_exit(0);
			}
			thread_idle_nums++;
			pthread_cond_wait(&cond,&mutex);
			thread_idle_nums--;
		}

		void WakeOneUpThread(){
			pthread_cond_signal(&cond);
		}

		void WakeAllUpThread(){
			pthread_cond_broadcast(&cond);
		}

		void PopTask(Task& t_){
			t_ = thread_queue.front();
			thread_queue.pop();
		}

		void PushTask(Task& t_){
			Lock();
			if(is_quit){
				Unlock();
				return;
			}
			LOG(INFO,"push one task into queue");
			thread_queue.push(t_);
			WakeOneUpThread();
			Unlock();
		}

		static void *thread_routine(void *arg){
			ThreadPool* tp = (ThreadPool*)arg;
			pthread_detach(pthread_self());
			LOG(INFO,"one thread start run");
			std::cout<<"now thread is "<<pthread_self()<<std::endl;
			
			for(;;){
				tp->Lock();
				while(tp->IsEmpty()){
					tp->ThreadIdle();
				}

				Task t_;
				tp->PopTask(t_);
				LOG(INFO,"thread start task");
				std::cout<<"now thread is "<<pthread_self()<<std::endl;
				tp->Unlock();
				t_.Run();
			}
		}

		void ThreadInit(){
			pthread_mutex_init(&mutex,NULL);
			pthread_cond_init(&cond,NULL);
			for(int i=0; i<thread_nums; i++){
				pthread_t tid;
				LOG(INFO,"create one thread");
				pthread_create(&tid,NULL, thread_routine ,this);
			}

			LOG(INFO,"thread pool init finish");
		}

		void Stop(){
			Lock();
			is_quit = true;
			Unlock();

			while(thread_idle_nums > 0){
				WakeAllUpThread();
			}
		}

		~ThreadPool()
		{
			pthread_cond_destroy(&cond);
			pthread_mutex_destroy(&mutex);
		}
};


#endif // __THREADTOOL_TOOL__
