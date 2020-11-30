
BINS=mix

all: $(BINS)

CXX=clang++
CXXFLAGS=--std=c++20 -g -Wall -Wextra
# Use C++ to link .o files
LINK.o=$(LINK.cc)


clean:
	rm -f $(BINS) *.o
