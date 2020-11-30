
BINS=mix

all: $(BINS)

mix: mix.o io.o

CXX=clang++
CXXFLAGS=--std=c++20 -g -Wall -Wextra
# Use C++ to link .o files
LINK.o=$(LINK.cc)


clean:
	rm -f $(BINS) *.o
