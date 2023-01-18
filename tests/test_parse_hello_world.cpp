#include "limn.h"

#include <cassert>
#include <string>

#include <iostream> // std::cout


#include <doctest/doctest.h>

using namespace lm; // Laziness


auto callback_hello = [](const std::string_view&){
    std::cout << "I will be called even though I'm in an expression that later fails to match" << std::endl;
};

auto callback_universe = [](const std::string_view&){
    std::cout << "I will be called (which is probably expected)" << std::endl;
};

TEST_CASE("test parsing hello world and hello universe"){
    CHECK(parse("hello world", (lit_("hello")[callback_hello] >> lit_("world")) | (lit_("hello universe")[callback_universe]) >> end_));
}










#if 0

int main () {



    //printMatchedItem("xyz uvw abc def");
}

#endif

