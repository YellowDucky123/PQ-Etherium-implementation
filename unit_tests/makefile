CXX = g++
CXXFLAGS = -std=c++23 -lssl -lcrypto
# Change based on test cases to implement
SRC ?= test_randRange.cpp
# Change based on test cases to implement
OUT ?= test_randRange

# Catch2 amalgamated
CATCH_SRC = catch_amalgamated.cpp
CATCH_HDR = catch_amalgamated.hpp

all:
	$(CXX) $(CXXFLAGS) $(CATCH_SRC) $(SRC) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)