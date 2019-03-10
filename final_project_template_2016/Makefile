LDFLAGS=-static -s -lws2_32
.PHONY:	clean
search.exe:	main.cc anqi.cc ClientSocket.cpp Protocol.cpp
	$(CXX) -std=c++11 -O2 -o $@ $^ $(LDFLAGS)
clean:
	DEL search.exe 2>NUL
