# Limn

Limn is a tiny parser designed to compile quickly.
It was inspired by Boost Spirit.  It's pretty much the same
except it's only for std::string_view and it compiles faster.
On my laptop, the simple tests compile in 0.5s.

# Usage

This is a single header library.  Simply `#include "limn.h"`.

# Example

    #include "limn.h"
    
    using namespace lm;
    
    // Check if a string matches
    bool isHelloWorld(std::string_view sv) {
        return parse(sv, lit_("Hello") >> *space_ >> lit_("World"));
    }
    
    // Return the match
    std::string_view getMatch(std::string_view sv) {
        std::string_view out;
        parse(sv, (lit_("GET") | lit_("POST")) >> space_ >> (*alnum_)[out]);
        return out;
    }


It is easy to create your own character types using `char_if_`.
See tests.cpp for an example.

There is also `action_` to run a user supplied callback when execution
reaches that point.

There are some documenting comments in the header.

