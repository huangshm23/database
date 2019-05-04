CXX = g++

HEADER_DIR = ../include

FPTREE_HEADER_DIR = $(HEADER_DIR)/fptree

CXXFLAGS = -std=c++11

CFLAGS = -fPIC -std=c99 -O3 -msse4.2 -mpclmul -march=native -funroll-loops -Wstrict-overflow -Wstrict-aliasing -Wall -Wextra -pedantic -Wshadow

MAIN = ./bin/main

YCSB = ./bin/ycsb

LYCSB = ./bin/lycsb

BIN = bin

FPTREE = fptree.o

UTILITY = utility.o

PALLOCATOR = p_allocator.o

CLHASH = clhash.o

ALL := $(LYCSB) $(YCSB) $(MAIN) $(FPTREE) $(UTILITY) $(PALLOCATOR) $(CLHASH)

#============================TODO: all below============================
all : $(BIN) $(ALL)

$(MAIN) : main.o $(FPTREE) $(UTILITY) $(PALLOCATOR) $(CLHASH)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(FPTREE) : fptree.cpp $(FPTREE_HEADER_DIR)/fptree.h
	$(CXX) $(CXXFLAGS) -c $< -I $(HEADER_DIR) -g -o $@

$(UTILITY) : utility.cpp $(HEADER_DIR)/utility/utility.h $(HEADER_DIR)/utility/clhash.h
	$(CXX) $(CXXFLAGS) -c $< -I $(HEADER_DIR) -g -o $@

$(PALLOCATOR) : p_allocator.cpp  $(HEADER_DIR)/utility/p_allocator.h
	$(CXX) $(CXXFLAGS) -c $< -I $(HEADER_DIR) -g -o $@

$(CLHASH) : clhash.c $(HEADER_DIR)/utility/clhash.h
	$(CC) $(CFLAGS) -c clhash.c -I $(HEADER_DIR) -g -o $@

main.o : main.cpp
	$(CXX) $(CXXFLAGS) -c $< -I $(HEADER_DIR) -g -o $@

$(BIN):
	mkdir -p bin

$(LYCSB) : lycsb.o $(BIN) 
	$(CXX) $(CXXFLAGS) $< -g -o $@ -lleveldb -lpthread 
lycsb.o : lycsb.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -lleveldb -lpthread

$(YCSB) : ycsb.o $(FPTREE)
	$(CXX) $(CXXFLAGS) $^ -g -o $@ -lleveldb -pthread
ycsb.o : ycsb.cpp $(FPTREE_HEADER_DIR)/fptree.h
	$(CXX) $(CXXFLAGS) -c $< -I $(HEADER_DIR) -g -o $@

clean :
	@rm -rf *.o ./bin/* ./bin

cleand : 
	@rm -rf ../data/*