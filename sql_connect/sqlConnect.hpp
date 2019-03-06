#ifndef _SQL_CONNECT_HPP__
#define _SQL_CONNECT_HPP__

#include<my_global.h>
#include<mysql.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

static const char* host = "localhost";
static const char* password="123";
static const char* user = "root";
static const char* database="mysqlcmd";


void exit_with_error(MYSQL*con){
	printf("%s",mysql_error(con));
	if(con == NULL){
		mysql_close(con);
	}
	//exit(1);
}

MYSQL* mysqlInit(){
	MYSQL* con =  mysql_init(NULL);
	if(con == NULL){
		exit_with_error(con);
	}

	return con;
}

void mysqlConnect(MYSQL* con){
	if(mysql_real_connect(con,host,user,password,database,0,NULL,0)==NULL){
		exit_with_error(con);
	}

	if(mysql_set_character_set(con,"utf8") != 0){
		exit_with_error(con);
	}
}
bool insertUser(MYSQL* con,std::string username,std::string password){
	char query[100];
	memset(query,0x00,100);
	sprintf(query,"insert into  user(username,password) values('%s','%s')",username.c_str(),password.c_str());
	if(mysql_query(con,query)){
		exit_with_error(con);
		return false;
	}
	return true;
}

bool insertString(MYSQL* con,const std::string username,\
		const std::string users_string){
	char query[100];
	memset(query,0x00,100);
	sprintf(query,"insert into  user_string(username,user_string) \
			values('%s','%s')",username.c_str(),users_string.c_str());
	if(mysql_query(con,query)){
		exit_with_error(con);
		return false;
	}
	return true;

}
bool selectString(MYSQL* con,std::string username){
	char query[100];
	memset(query,0x00,100);
	sprintf(query,"select * from user_string where username='%s'",username.c_str());
	
	if(mysql_query(con,query)){
		exit_with_error(con);
		return false;
	}

	MYSQL_RES* result = mysql_store_result(con);
	ulonglong rows = mysql_num_rows(result);
	ulonglong cols = mysql_num_fields(result);
	MYSQL_ROW row;
	while((row = mysql_fetch_row(result))){
		int i = 0;
		for(i=cols-1; i<cols; i++){
			printf("%s",row[i]);
		}
		printf("\n");
	}
	
	mysql_free_result(result);
	return true;
}

bool selectData(MYSQL* con,std::string username,std::string password){
	char query[100];
	memset(query,0x00,100);
	sprintf(query,"select * from user where username='%s'",username.c_str());
	if(mysql_query(con,query)){
		exit_with_error(con);
		return false;
	}

	MYSQL_RES* result = mysql_store_result(con);
	ulonglong rows = mysql_num_rows(result);
	if(rows == 0){
		mysql_close(con);
		return false;
	}
	ulonglong cols = mysql_num_fields(result);
	MYSQL_ROW row;
	bool flag = false;
	row = mysql_fetch_row(result);
	if(row[2] == password){
		flag = true;
	}
	
	mysql_free_result(result);
	return flag;
}

void mysqlClose(MYSQL* con){
	mysql_close(con);
}

#endif //_SQL_CONNECT_HPP__

