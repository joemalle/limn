all:
	g++ -std=c++17 -Wall tests.cpp

docs:
	doxygen

clean:
	rm -rf a.out docs/
