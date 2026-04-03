CXX = g++
CXXFLAGS = -g -Wall -Wextra -pthread -std=c++20
TARGETS = future thread_pool mypool
SRC_FUTURE = future.cpp
SRC_THREAD_POOL = thread_pool.cpp
SRC_MYPOOL = mypool.cpp

all: $(TARGETS)

future: $(SRC_FUTURE)
	$(CXX) $(CXXFLAGS) $(SRC_FUTURE) -o future

thread_pool: $(SRC_THREAD_POOL)
	$(CXX) $(CXXFLAGS) $(SRC_THREAD_POOL) -o thread_pool

mypool: $(SRC_MYPOOL)
	$(CXX) $(CXXFLAGS) $(SRC_MYPOOL) -o mypool

clean:
	rm -f $(TARGETS)
