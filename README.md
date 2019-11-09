# Limn

Limn is a tiny parser library for C++17 and up.
It is designed to be easy to use and to compile quickly.
It was inspired by Boost Spirit.

# Compilation time

On my laptop, the Limn tests compile in about half a second.
For reference, my laptop compiles the
[Boost Spirit employee example](https://www.boost.org/doc/libs/1_68_0/libs/spirit/example/x3/employee.cpp)
in about 10 seconds.

# Compiler support

I have tested this on:

 - GCC 9.2
 - Clang 9.0
 - MSVC 19.22

... using C++17 and C++2a.
On MSVC I only tested compilation since I don't have a Windows computer.

# Usage

This is a single header library.
Simply `#include "limn.h"`.
There are no dependencies besides the STL's string_view and cctype.

Then call `bool parse(std:string_view, Parser)`.
Take a look at the example `Parser`s below.

For reference style documentation, run `make docs`.
To run the tests, run `make`.

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
    constexpr bool validParentheses(std::string_view& sv) {
        return parse_ref(
            sv,
            +(lit_("()") | (char_('(') >> action_(validParentheses) >> char_(')')))
        );
    }

Look at `tests.cpp` for more.

