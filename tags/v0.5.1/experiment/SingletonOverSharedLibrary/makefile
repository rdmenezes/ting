all:
	g++ testso.cpp -shared -o libtestso.so -I../../src
	g++ main.cpp -o main -I../../src -L. -ltestso