#include"ThreadPool.hpp"
#include<unistd.h>

int print(int sock){
	std::cout<<"now handler "<<sock<<std::endl;
	usleep(1000);
}

int main(){
	ThreadPool* tp = new ThreadPool;
	tp->ThreadInit();
	for(int i=0; i<100; i++){
		Task t_(0,print);
		tp->PushTask(t_);
	}
	
	tp->Stop();
	return 0;
}
