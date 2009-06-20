CXX=g++
CXXFLAGS=-Isrc/

all: test/regress
test/regess: test/regress.cc src/text_config.h
	$(CXX) $(CXXFLAGS) -o $@ -g test/regress.cc
