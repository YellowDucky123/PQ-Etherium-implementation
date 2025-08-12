CXX = g++
CXXFLAGS = -std=c++23 -lssl -lcrypto
# Change based on test cases to implement
SRC ?= test_endian.cpp
# 
OPEN_SSL = -I/opt/homebrew/opt/openssl@3/include
# Change based on test cases to implement
OUT ?= test

# Catch2 amalgamated
CATCH_SRC = catch_amalgamated.cpp
CATCH_HDR = catch_amalgamated.hpp

all:
	$(CXX) $(CXXFLAGS) $(OPEN_SSL) $(CATCH_SRC) $(SRC) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)