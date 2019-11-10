all:
	g++ -std=c++17 -Wall -I. tests/tests.cpp

http:
	g++ -std=c++17 -Wall -I. tests/http.cpp

docs:
	doxygen

clean:
	rm -rf a.out docs/
