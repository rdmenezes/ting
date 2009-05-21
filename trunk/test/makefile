test: PoolStored_test

PoolStored_test: src/PoolStored_test.cpp
	@g++ -o $@ -I../src -DDEBUG -lpthread -lrt $<
	@./$@
