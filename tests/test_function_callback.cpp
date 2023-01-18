#include "limn.h"

#include <cassert>
#include <string>

#include <iostream> // std::cout

#include <doctest/doctest.h>

using namespace lm; // Laziness


// a function for print the matched string
void testCallbackFunction(const std::string_view& output) {
    std::cout << output << "|\n";
}

// a functor which will be called when some item get metched
std::function<void(const std::string_view&)> fn = testCallbackFunction;


TEST_CASE("test function callback of matched item"){
    // a test function to run the parser and their callback
    CHECK(parse("xyz uvw abc def", *(*alnum_ >> *space_ >> *alnum_ >> *space_)[fn]));
}


