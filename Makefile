CXX=g++
CXXFLAGS=-Iinclude/

all: test/regress
test/regess: test/regress.cc include/setting.h
	$(CXX) $(CXXFLAGS) -o $@ -g test/regress.cc
