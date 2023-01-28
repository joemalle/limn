#include "limn.h"

#include <cassert>
#include <string>

#include <iostream> // std::cout

#include <doctest/doctest.h>

namespace {

using namespace lm; // Laziness

auto ident = lexeme_(alpha_ >> *alnum_);

auto ident2 = alpha_ >> *alnum_;

auto p = [](const std::string_view& output){
    if (output.empty())
        std::cout << "Empty Input Matches!!!" << std::endl;
    else
        std::cout << output << std::endl;
};

TEST_CASE("test simple text parsing"){
    CHECK(parse("a", ident[p] >> end_));

    CHECK(parse("ab", ident[p] >> end_));

    CHECK(parse("aaa   bbbb", ident[p] >> ident[p] >> end_));

    CHECK(parse("a b ", ident[p] >> ident[p] >> end_));

    CHECK(parse("a1 b2 ", ident[p] >> ident[p] >> end_));

    CHECK_FALSE(parse("a b ", ident2[p] >> ident2[p] >> end_));

}

}

