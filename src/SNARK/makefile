

# g++ -std=c++23 \
#     -I./libiop \
#     -I./libiop/depends/libff \
#     aggregate.cpp \
#     -L./libiop/build -liop \
#     -L./libiop/depends/libff/build -lff \
#     -lgmp \
#     -o my_program


# g++ -std=c++23 \
#     -I./libiop/ \
#     aggregate.cpp \
#     -L./libiop/build \
#     -L./libiop/depends/libfqfft \
#     -o aggregate

# g++ -std=c++23 \    
# 	-I./libiop \
# 	-I./libiop/depends/libfqfft \   
# 	aggregate.cpp \    
# 	-L./libiop/build \    
# 	-L./libiop/depends/libfqfft \  
# 	-o aggregate 

# 	g++ -std=c++17 -I./libiop -I./libiop/depends/libff -I./libiop/depends/libfqfft aggregate.cpp \
#     -L./libiop/build -liop -L./libiop/depends/libff/build -lff -L./libiop/depends/libfqfft/build -lfqfft -lgmp \
#     -o aggregate

aggregate: aggregate.cpp 
	@g++ -std=c++23 \
	    -I./libiop \
	    -I./libiop/depends/libff \
	    -I./libiop/depends/libfqfft \
	    aggregate.cpp \
	    -L./libiop/build -liop \
	    -L./libiop/depends/libff/build -lff \
	    -L./libiop/depends/libfqfft/build -lfqfft \
	    -lgmp -pthread \
	    -o aggregate
