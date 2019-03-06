#include<iostream>
#include"unencode.hpp"
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<unordered_map>
#include"sqlConnect.hpp"
int main(){
	char* ParamSize_ = getenv("ContentParamSize");
	if(ParamSize_ == NULL){
		return 0;
	}
	int len_ = atoi(ParamSize_);

	std::string buf_;	
	char c_;
	while(len_>0 &&read(0,&c_,1)>0){
		buf_.push_back(c_);
		len_--;
	}

	std::unordered_map<std::string,std::string> params = Unencoding(buf_);
	MYSQL * con = mysqlInit();
	mysqlConnect(con);
	if(!insertUser(con,params["username"],params["password"])){
		printf("username is exist\n");
		return 1;
	}
	
	mysqlClose(con);
	return 0;	
}
