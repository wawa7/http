#include<iostream>
#include<vector>
#include<unordered_map>

std::vector<std::string>  split(std::string &str,const std::string sf){
	int start = 0;
	size_t pos = 0;
	std::vector<std::string> v;
	do{
		pos = str.find(sf.c_str(),start);
		v.push_back(str.substr(start,pos-start));
		start = pos+1;
	}while(pos != std::string::npos);
	
	return v;
}

std::unordered_map<std::string,std::string> Unencoding(std::string &str){
	int size = str.size();
	std::string params;
	for(int i=0; i<size; i++){
		switch(str[i]){
			case '+':
				params.push_back(' ');
				break;
			case '%':{
				int tmp = 0;
				sscanf(str.c_str()+i+1,"%02x",tmp);
				params.push_back(tmp);
				i+=2;
				break;
			}
			default:
				params.push_back(str[i]);
		}
	}
	
	std::unordered_map<std::string,std::string> result;
	std::vector<std::string> vparams;
	vparams = split(params,"&");
	size = vparams.size();
	for(int i=0; i<size; i++){
		std::vector<std::string> v = split(vparams[i],"=");
		result.insert(std::pair<std::string,std::string>(v[0],v[1]));
	}
	return result;
}

//int main()
//{
//	string str = "username=wang&password=123";
//	unordered_map<std::string,std::string> result = Unencoding(str);	
//	
//	cout<<result["username"]<<endl;
//	cout<<result["password"]<<endl;
//	return 0;
//}
