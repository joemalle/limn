all:
	g++ -std=c++17 -Wall -I. tests/tests.cpp

http:
	g++ -std=c++17 -Wall -I. tests/http.cpp

docs:
	doxygen

clean:
	rm -rf a.out docs/ *.exp

prep:
	expand -t 4 limn.h > limn.exp
	mv limn.exp limn.h
	expand -t 4 tests/tests.cpp > tests/tests.exp
	mv tests/tests.exp tests/tests.cpp
	expand -t 4 tests/http.cpp > tests/http.exp
	mv tests/http.exp tests/http.cpp
	
