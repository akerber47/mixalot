
BINS=mix mixal

all: $(BINS)

mix: mix.o sys.o io.o core.o dbg.o cpu.o

mixal: mixal.o dbg.o core.o

CXX=clang++
CXXFLAGS=--std=c++20 -g -Wall -Wextra
# Use C++ to link .o files
LINK.o=$(LINK.cc)


clean:
	rm -f $(BINS) *.o
