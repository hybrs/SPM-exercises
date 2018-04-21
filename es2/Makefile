CXX      = g++
CXXFLAGS = -O3 -std=c++17 -I /usr/local/include/grppi/
LDFLAGS  = -pthread
OBJS     = es2 seq

all:	$(OBJS)

es2.cpp: 	util.cpp
seq.cpp: 	util.cpp

clean:	
	rm -f $(OBJS)
