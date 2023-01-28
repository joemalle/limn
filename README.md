# Limn

[![Build Status](https://travis-ci.com/joemalle/limn.svg?branch=master)](https://travis-ci.com/joemalle/limn)
[![Documentation](https://codedocs.xyz/joemalle/limn.svg)](https://codedocs.xyz/joemalle/limn/)

Limn is a tiny parser library for C++17 and up.
It is designed to be easy to use and to compile quickly.
It was inspired by the excellent [Boost Spirit](https://www.boost.org/doc/libs/develop/libs/spirit/doc/x3/html/index.html).

# Compilation time

On my laptop, the Limn tests compile in about half a second.
For reference, my laptop compiles the
[Boost Spirit employee example](https://www.boost.org/doc/libs/1_68_0/libs/spirit/example/x3/employee.cpp)
in about 10 seconds.

# Compiler support

The earliest compiler versions that Limn supports are:

 - GCC 7.4 with `-std=c++17`
 - Clang 7.0 with `-std=c++17`
 - MSVC 19.14 with `/std:c++17`

N.B. On MSVC I only tested compilation since I don't have a Windows computer.

# Usage

This is a single header library.
Simply `#include "limn.h"`.
There are no dependencies besides the STL's `string_view` and `cctype`.

Then call `bool parse(std:string_view, Parser)`.
Take a look at the example `Parser`s below.

For reference style documentation, go to [codedocs](https://codedocs.xyz/joemalle/limn/namespacelm.html) or run `make docs`.
To run the tests, run `make && ./a.out`.

# Examples

    #include "limn.h"
    
    using namespace lm;
    
    // Check if a string matches
    bool isHelloWorld(std::string_view sv) {
        return parse(sv, lit_("Hello") >> *space_ >> lit_("World") >> end_);
    }
    
    // Return the match
    std::string_view getMatch(std::string_view sv) {
        std::string_view out;
        parse(sv, (lit_("GET") | lit_("POST")) >> space_ >> (*alnum_)[out]);
        return out;
    }
    
    // Recursive example: match valid parentheses
    // Can run at compile time
    constexpr bool validParentheses(std::string_view& sv) {
        return parse_ref(
            sv,
            +(lit_("()") | (char_('(') >> action_(validParentheses) >> char_(')')))
        );
    }

Look at [tests.cpp](tests/tests.cpp) and [http.cpp](tests/http.cpp) for more example code.

# Skip whitespace and Lexer mode

The parser automatically skip the whitespace between tokens, but in some cases you do not want to skip whitespace.
For example, in lexer mode, if you to match an identifier such as `alpha_ >> *alnum_`, this will match the whole string `a b`,
because when running the `>>` seq function, the space between `a` and `b` will get skipped, so the matched string is not you want.
In this case, you have to write `lexeme_(alpha_ >> *alnum_)`, here the class `lexeme_` has skip whitespace feature disabled for all its sub parsers,
so, you get a correct identifier `a`.

