bin=HttpServer
src=HttpServer.cc
cc=g++
LIB=-lpthread -std=c++0x

.PHONY:all
all:$(bin) test_cgi	
	mv test_cgi Root/

$(bin):$(src)
	$(cc) -o $@ $^ $(LIB)
test_cgi:test_cgi.cc
	g++ -o $@ $^

.PHONY:clean
clean:
	rm -rf $(bin) 
