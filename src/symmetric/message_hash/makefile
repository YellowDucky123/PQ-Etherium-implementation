# Compiler commands
CC = gcc
CXX = g++

# Compiler flags
CFLAGS = -O3 -Wall -Wextra -std=c11
CXXFLAGS = -O3 -Wall -Wextra -std=c++23

# Source files
C_SOURCES = Blake/c/blake3.c Blake/c/blake3_dispatch.c Blake/c/blake3_portable.c
CPP_SOURCES = blake3.cpp

# Assembly sources
ASM_SOURCES = Blake/c/blake3_sse2_x86-64_unix.S \
              Blake/c/blake3_sse41_x86-64_unix.S \
              Blake/c/blake3_avx2_x86-64_unix.S \
              Blake/c/blake3_avx512_x86-64_unix.S

# Object files
C_OBJECTS = $(C_SOURCES:.c=.o)
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)
ASM_OBJECTS = $(ASM_SOURCES:.S=.o)

# Target executable
TARGET = blake

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(CPP_OBJECTS) $(ASM_OBJECTS)
	$(CXX) -O3 -o $@ $^ -lssl -lcrypto

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

%.o: %.S
	$(CC) -c $< -o $@

clean:
	rm -f $(C_OBJECTS) $(CPP_OBJECTS) $(ASM_OBJECTS) $(TARGET) 
