CFLAGS = -g -Wall -Wextra -pedantic -std=c++11 -pthread 

all: SharedPtr_test

SharedPtr_test: SharedPtr_test.o
	g++ $(CFLAGS) -o SharedPtr_test SharedPtr_test.o
SharedPtr_test.o: SharedPtr_test.cpp SharedPtr.hpp
	g++ $(CFLAGS) -c SharedPtr_test.cpp

run: all
	./SharedPtr_test

clean:
	rm -rf *.o SharedPtr_test

memcheck: all
	valgrind ./SharedPtr_test
memcheck++: all
	valgrind --leak-check=full --track-origins=yes -v ./SharedPtr_test
