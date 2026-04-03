CXX = g++-14
CXXFLAGS = -g -Wall -Wextra -pthread -std=c++23
TARGETS = future thread_pool my_prod_cons async_producer
SRC_FUTURE = future.cpp
SRC_THREAD_POOL = thread_pool.cpp
SRC_MY_PROD_CONS = my_prod_cons.cpp
SRC_ASYNC_PRODUCER = async_producer.cpp

all: $(TARGETS)

future: $(SRC_FUTURE)
	$(CXX) $(CXXFLAGS) $(SRC_FUTURE) -o future

thread_pool: $(SRC_THREAD_POOL)
	$(CXX) $(CXXFLAGS) $(SRC_THREAD_POOL) -o thread_pool

my_prod_cons: $(SRC_MY_PROD_CONS)
	$(CXX) $(CXXFLAGS) $(SRC_MY_PROD_CONS) -o my_prod_cons

async_producer: $(SRC_ASYNC_PRODUCER)
	$(CXX) $(CXXFLAGS) $(SRC_ASYNC_PRODUCER) -o async_producer

clean:
	rm -f $(TARGETS)
