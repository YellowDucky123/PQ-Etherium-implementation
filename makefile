g++ -std=c++20 -fmodules-ts -xc++-system-header iostream 

main: main.cpp test.cpp 
	g++ -std=c++20 -fmodules-ts main.cpp test.cpp -o main