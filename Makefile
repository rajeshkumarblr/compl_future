CXX = g++
CXXFLAGS = -g -Wall -Wextra -pthread -std=c++20
TARGET = future
SRC = future.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
