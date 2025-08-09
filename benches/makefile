CXX = g++
CXXFLAGS = -std=c++23 -lssl -lcrypto
# Change based on test cases to implement
SRC ?= sha.cpp
# Change based on test cases to implement
OUT ?= sha


all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)