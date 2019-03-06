#ifndef __HTTP_SERVER_HPP__
#define __HTTP_SERVER_HPP__

#include<iostream>
#include<string>
#include<stdlib.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include"Log.hpp"
#include"Parse.hpp"
#include"ThreadPool.hpp"
class HttpServer
{
private:
	int listen_sock;
	int port;
	ThreadPool* tp;
public:
	HttpServer(int port_):listen_sock(-1),port(port_),tp(NULL)
	{}

	~HttpServer(){
		if(listen_sock != -1){
			close(listen_sock);
		}
		if(tp != NULL){
			delete tp;
			tp->Stop();
		}
	}
	void InitServer(){
		if(port == -1)
			return;

		listen_sock = socket(AF_INET,SOCK_STREAM,0);
		if(listen_sock == -1){
			LOG(ERROR,"create socket error");
		}
		int opt_ = 1;
		setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt_, sizeof(opt_));

		LOG(INFO,"create listen_sock sucess");
		struct sockaddr_in server;
		server.sin_family =AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = INADDR_ANY;

		if(bind(listen_sock,(struct sockaddr*)&server,sizeof(server)) == -1){
			LOG(ERROR,"bind socket error");
			exit(3);
		}

		if(listen(listen_sock,5) == -1){
			LOG(ERROR,"listen socket error");
			exit(4);
		}

		tp = new ThreadPool;
		tp->ThreadInit();
	}

	void Start(){
		if(listen_sock == -1 || port == -1){
			LOG(ERROR,"no initserver");
			exit(5);
		}

		LOG(INFO,"HttpServer start...");
		while(1){
			struct sockaddr_in client;
			socklen_t len = sizeof(client);
			int sock = accept(listen_sock,(struct sockaddr*)&client,&len);
			if(sock < 0){
				LOG(WARNING,"accpet warning");
				continue;
			}
		
			LOG(INFO,"get a new link...");
			Task* t_ = new Task(sock,Entry::HandlerRequest);
			tp->PushTask(*t_);
		}
	}
};


#endif // __HTTP_SERVER_HPP__
