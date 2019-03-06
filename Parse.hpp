#ifndef __PARSE_HPP__
#define __PARSE_HPP__

#include<string>
#include<cstring>
#include"Log.hpp"
#include<sstream>
#include<unordered_map>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/sendfile.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>

#define ROOT_DIR "Root"
#define HOME_PAGE "index.html"
#define PAGE_404 "404.html"
#define PAGE_500 "500.html"
#define PAGE_400 "400.html"

#define BAD_REQUEST 400
#define NOT_FOUND 404
#define INTERNAL_SERVER_ERROR 500
#define OK 200

static std::unordered_map<std::string,std::string> SuffixDesc{
		{".html","text/html"},
		{".css","text/css"},
		{".jpg","image/jpeg"},
		{".js","application/x-javascript"}
};

class ProtocolUtil
{
	public:
		static bool MakeKV(std::unordered_map<std::string,std::string>& makekv_,std::string line_){
			std::size_t pos = line_.find(": ");
			if(std::string::npos != pos){
				makekv_.insert(make_pair(line_.substr(0,pos),line_.substr(pos+2)));
				return true;
			}

			return false;
		}

		static int StringToInt(std::string line_){
			int tmp = 0;
			std::stringstream ss(line_);
			ss >> tmp;
			return tmp;
		}

		static std::string IntToString(int code_){
			std::stringstream ss_;
			ss_<<code_;
			return ss_.str();
		}

		static std::string GetCodeDesc(int code_){
			switch(code_){
				case 200:
					return "OK";
				case NOT_FOUND:
					return "NOT FOUNT";
				case BAD_REQUEST:
					return "Bad Request";
				case INTERNAL_SERVER_ERROR:
					return "internal server error";
				default:
					LOG(WARNING,"unknow code");
					return "UNKNOW";
			}
		}
};

class Request
{
	public:
		std::string rq_line;
		std::string rq_head;
		std::string rq_blank;
		std::string rq_text;
	private:
		std::string method;
		std::string uri;
		std::string version;

		std::string path;
		std::string suffix;
		std::string param;
		int content_length;
		int resource_size;
		std::unordered_map<std::string,std::string> makekv;
	public:
		Request():rq_blank("\n"),path(ROOT_DIR),suffix(".html"),content_length(-1),resource_size(0)
		{}

		int GetResourceSize(){
			return resource_size;
		}
	
		void SetResourceSize(int size){
			resource_size = size;
		}
		std::string GetSuffix(){
			return suffix;
		}

		void SetSuffix(std::string suffix_){
			suffix = suffix_;
		}

		std::string GetPath(){
			return path;
		}

		void SetPath(std::string path_){
			path = path_;
		}

		std::string& GetParam(){
		
			return param;
		}
		
		void UriParse(){
			std::stringstream ss_(rq_line);
			ss_ >> method >> uri >> version;
		}

		bool HeadParse(){
			std::size_t start_ = 0;

			while(start_ < rq_head.size()){
				LOG(INFO,"start makekv");
				std::size_t pos_ = rq_head.find('\n',start_);
				if(pos_ == std::string::npos){
					break;
				}
				std::string line_ = rq_head.substr(start_,pos_-start_);
				LOG(INFO,line_);
				if(line_ != ""){
					if(!ProtocolUtil::MakeKV(makekv,line_)){
						return false;
					}
				}else	{
					LOG(INFO,"str is empty");
					break;
				}
				start_ = pos_+1;
			}
			
			return true;
		}

		int GetContentLength(){
			std::string content_length_ = makekv["Content-Length"];
			if(!content_length_.empty())
				content_length = ProtocolUtil::StringToInt(content_length_);
			LOG(INFO,content_length_);
			return content_length;
		}

		bool IsMethodVaild(int & cgi_){
			if((strcasecmp(method.c_str(),"GET")==0) ||(cgi_= (strcasecmp(method.c_str(),"POST")==0))){
				return true;
			}

			return false;
		}

		bool IsNeedRecvText(){
			if(strcasecmp(method.c_str(),"POST")==0){
				return true;
			}
			return false;
		}
		void GetPathAndParam(int & cgi_){
			if(strcasecmp(method.c_str(),"GET") == 0){
				std::size_t pos = uri.find('?');
				if( std::string::npos != pos){
					path += uri.substr(0,pos);
					cgi_ = true;
					param = uri.substr(pos+1);
				}else{
					path += uri;
				}
			}else{
				path += uri;
			}

			if(uri[uri.size()-1] == '/'){
				path += HOME_PAGE;
			}
		}

		
		bool IsPathVaild(int& cgi_){
			struct stat st;
			if(stat(path.c_str(),&st)<0){
				LOG(ERROR,"path is not vaild");
				return false;
			}
			if(S_ISDIR(st.st_mode)){
				path += '/';
				path += HOME_PAGE;
				struct stat st2;
				stat(path.c_str(),&st2);
				resource_size = st2.st_size;
			}else{
				resource_size = st.st_size;
			}

			if(st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH){
				cgi_ = true;
			}

			std::size_t pos = path.rfind('.');
			if(std::string::npos != pos){
				suffix = path.substr(pos);
			}

			return true;
		}
};

class Response
{
	public:
		std::string rsp_line;
		std::string rsp_head;
		std::string rsp_blank;
		std::string rsp_text;
		int code;
		int fd;
	public:
		Response():rsp_blank("\n"),code(200),fd(-1)
		{}

		void GetResponseLine(){
			rsp_line += "HTTP/1.0";
			rsp_line += ' ';
			rsp_line += ProtocolUtil::IntToString(code); 
			rsp_line += ' ';
			rsp_line += ProtocolUtil::GetCodeDesc(code);
			rsp_line += '\n';
		}

		void GetResponseHead(Request*& rq_){
			rsp_head += "Content-Length: ";
			rsp_head += ProtocolUtil::IntToString(rq_->GetResourceSize());
			rsp_head += '\n';
			rsp_head += "Content-Type: ";
			rsp_head += SuffixDesc[rq_->GetSuffix()];
			rsp_head += '\n';
		}

		void OpenResource(Request*&rq_){
			fd = open(rq_->GetPath().c_str(),O_RDONLY);
		}

		~Response(){
			if(fd != -1){
				close(fd);
			}
		}
};

class Connect
{
	private:
		int sock;
	public:
		Connect(int sock_):sock(sock_)
		{}

		void SendResponse(Response*& rsp_,Request*&rq_,bool cgi_){
			std::string rsp_line_ = rsp_->rsp_line;
			std::string rsp_head_ = rsp_->rsp_head;
			std::string rsp_blank_ = rsp_->rsp_blank;
			
			send(sock,rsp_line_.c_str(),rsp_line_.size(),0);
			send(sock,rsp_head_.c_str(),rsp_head_.size(),0);
			send(sock,rsp_blank_.c_str(),rsp_blank_.size(),0);
			if(cgi_){
				std::string rsp_text_ = rsp_->rsp_text;
				send(sock,rsp_text_.c_str(),rsp_text_.size(),0);
			}else{
				int& fd_ =  rsp_->fd;	
				sendfile(sock,fd_,0,rq_->GetResourceSize());		
			}
		}	
		
		void ProcessNonCgi(Request* & rq_,Response*& rsp_){
			rsp_->GetResponseLine();
			rsp_->GetResponseHead(rq_);
			rsp_->OpenResource(rq_);
			SendResponse(rsp_,rq_,false);
		}	

		void ProcessCgi(Request*& rq_,Response*& rsp_){
			int input[2];
			int output[2];
			
			pipe(input);
			pipe(output);

			std::string path_ = rq_->GetPath();
			std::string param_ = rq_->GetParam();
			int param_size_ = param_.size();

			pid_t pid = fork();
			if(pid < 0){
				return;
			}
			if(pid==0){//child
				close(input[1]);
				close(output[0]);

				dup2(input[0],0);
				dup2(output[1],1);

				std::string ContentParamSize_ = "ContentParamSize=";
				ContentParamSize_ += ProtocolUtil::IntToString(param_size_);
				putenv((char*)ContentParamSize_.c_str());
				
				execl(path_.c_str(),path_.c_str(),NULL);
				exit(1);
			}else{//parent
				close(input[0]);
				close(output[1]);
				
				int total = 0;
				int cur = 0;
				while(total < param_size_ && \
				(cur = write(input[1],param_.c_str()+total,param_size_-total)) > 0){
						total += cur;	
				}

				char c = 0;
				while(read(output[0],&c,1) > 0){
					rsp_->rsp_text.push_back(c);				
				}
				waitpid(pid,NULL,0);

				rq_->SetResourceSize(rsp_->rsp_text.size());
		    	rsp_->GetResponseLine();
		    	rsp_->GetResponseHead(rq_);
		    	SendResponse(rsp_,rq_,true);
			}
		}

		void ProcessResponse(Request* &rq_,Response*& rsp_,int& cgi_){
			if(cgi_){
				ProcessCgi(rq_,rsp_);
			}else{
				ProcessNonCgi(rq_,rsp_);
			}
		}

		void RecvOneLine(std::string& line_)
		{
			char c = 'X';
			while(c!='\n'){
				ssize_t ret = recv(sock,&c,1,0); 
				if(ret <= 0){
					break;
				}
				if(c == '\r'){
					ret = recv(sock,&c,1,MSG_PEEK);
					if(ret > 0){
						if(c == '\n'){
							recv(sock,&c,1,0);
						}
					}
					c = '\n';
				}
				line_.push_back(c);
			}
		}

		void RecvRequestHead(std::string& head_){
			std::string line_ = "X";
			LOG(INFO,"start read head");
			while(line_ != "\n"){
				line_ = "";
				//LOG(INFO,"read one line");
				RecvOneLine(line_);
				head_ += line_;
			}
			LOG(INFO,"parse head finish");
		}

		void RecvRequestText(std::string& param_,std::string& text_ ,int  len_){
			char c = 'X';
			while(len_>0){
				len_--;
				int ret = recv(sock,&c,1,0);
				if(ret <= 0){
					break;
				}
				text_.push_back(c);
			}

				param_ = text_;
		}

		~Connect(){
			if(sock >= 0) {
				printf("~Connect\n");
				close(sock);			
			}
		}
};

class Entry
{
	private:
		int sock;
	public:
	static void ProcessError(Request*& rq_,Response*& rsp_){
		//path
		std::string path_ = rq_->GetPath();
			
		struct stat st;
		stat(path_.c_str(),&st);

		rq_->SetResourceSize((int)st.st_size);
		rq_->SetSuffix(".html");
	}

	static void HandlerError(Request*& rq_,Response*& rsp_){
		int code_ = rsp_->code;
		std::string path_ = ROOT_DIR;
		switch(code_){
			case 404:
				path_ += '/';
				path_ += PAGE_404;
				rq_->SetPath(path_);
				ProcessError(rq_,rsp_);
				break;
			case 500:
				path_ += '/';
				path_ += PAGE_500;
				rq_->SetPath(path_);
				ProcessError(rq_,rsp_);
				break;
			case 400:
				path_ += '/';
				path_ += PAGE_400;
				rq_->SetPath(path_);
				ProcessError(rq_,rsp_);
				break;
			default:
				ProcessError(rq_,rsp_);
		}
	}
	
	static int HandlerRequest(int arg)
		{
			int cgi_ = 0;
			int sock = arg;
			Connect* conn = new Connect(sock);
			Request* rq = new Request;
			Response* rsp = new Response;

			conn->RecvOneLine(rq->rq_line);
			rq->UriParse();
			
			LOG(INFO,"uri parse finish");
			int & code_ = rsp->code;
			if(!rq->IsMethodVaild(cgi_)){
				code_  = BAD_REQUEST;
				conn->RecvRequestHead(rq->rq_head);
				goto end;
			}

			rq->GetPathAndParam(cgi_);

			LOG(INFO,"get path and param");
			if(!rq->IsPathVaild(cgi_)){
				conn->RecvRequestHead(rq->rq_head);
				code_ = NOT_FOUND;
				goto end;
			}

			conn->RecvRequestHead(rq->rq_head);
			if(!rq->HeadParse()){
				code_ = BAD_REQUEST;
				goto end;
			}

			LOG(INFO,"finish head parse");
			if(rq->IsNeedRecvText()){
				conn->RecvRequestText(rq->GetParam(),rq->rq_text,rq->GetContentLength());
			}

end:
			if(code_ != 200){
				HandlerError(rq,rsp);;
				cgi_ = 0;
			}

			conn->ProcessResponse(rq,rsp,cgi_);
			delete conn;
			delete rsp;
			delete rq;
			printf("Handle Request\n");
		}
		
};

#endif // __PARSE_HPP__
