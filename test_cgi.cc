#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

#include"Log.hpp"
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
	
	
	int a,b;
	sscanf(buf_.c_str(),"a=%d&b=%d",&a,&b);

//	size_t pos = buf_.find('a');
//	int a = atoi(buf_.substr(pos+2).c_str());
//	pos = buf_.find('b');
//	int b = atoi(buf_.substr(pos+2).c_str());

	std::cout<<"<html>"<<a+b<<"</html>"<<std::endl;
	return 0;
}
