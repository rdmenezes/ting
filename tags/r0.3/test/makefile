test: PoolStored_test Timer_test


PoolStored_test: src/PoolStored_test.cpp
	@g++ -o $@ -I../src -DDEBUG -lpthread -lrt $<
	@./$@

Timer_test: src/Timer_test.cpp
	@g++ -o $@ -I../src -DDEBUG -lpthread -lrt $<
	@./$@

clean:
	@rm PoolStored_test
	@rm Timer_test