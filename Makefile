all:
	g++ -std=c++2a -Wall tests.cpp

docs:
	doxygen

clean:
	rm -rf a.out docs/
