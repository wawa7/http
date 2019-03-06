all:insertUser insertString selectString output
insertString:insertString.cc
	g++ -std=c++11 `mysql_config --cflags --libs`  -g $^ -o $@
selectString:selectString.cc
	g++ -std=c++11 `mysql_config --cflags --libs`  -g $^ -o $@
insertUser:insertUser.cc
	g++ -std=c++11 `mysql_config --cflags --libs`  -g $^ -o $@
output:
	mv insertUser insertString selectString ../Root/
.PHONY:clean
clean:
	rm insertString selectString insertUser
