#include"HttpServer.hpp"
#include<signal.h>

void Usage(char* arg){
	std::cout<<"Usage:"<<arg<<"[port]"<<std::endl;
}

int main(int argc,char* argv[])
{
	if(argc != 2){
		Usage(argv[0]);
		exit(1);
	}
	signal(SIGPIPE,SIG_IGN);
	HttpServer http(atoi(argv[1]));
	http.InitServer();
	http.Start();
	return 0;
}
