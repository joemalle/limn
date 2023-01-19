#include "limn.h"

#include <cassert>
#include <string>

#include <iostream> // std::cout

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

// Match "Hello", as many spaces as necessary, and then "World"
bool isHelloWorld(std::string_view sv) {
    return lm::parse(sv, lm::lit_("Hello") >> *lm::space_ >> lm::lit_("World") >> lm::end_);
}

using namespace lm; // Laziness

// return the word after the first space or an empty string
// the second word must consist of printable characters
// note that you can make these beauties constexpr!
// also note that lm:: doesn't use exceptions
constexpr std::string_view getSecondWord(std::string_view sv) noexcept {
    std::string_view secondWord;
    parse(
        sv,
        *alpha_
        //>> space_
        >> (*char_if_([](auto ch) -> bool {
            return std::isprint(ch) && !std::isspace(ch);
        }))[secondWord]
        >> end_
    );
    return secondWord;
}

// return the word after "GET" or "POST"
constexpr std::string_view getMatch(std::string_view sv) noexcept {
    std::string_view out;
    //parse(sv, (lit_("GET") | lit_("POST")) >> space_ >> (*alnum_)[out]);
    parse(sv, (lit_("GET") | lit_("POST")) >> (*alnum_)[out]);
    return out;
}

// Recursive example: match valid parentheses
constexpr bool validParentheses(std::string_view& sv) {
    return parse_ref(
        sv,
        +(lit_("()") | (char_('(') >> action_(&validParentheses) >> char_(')')))
    );
}

// Helper function to convert char* to string_view
constexpr bool validParenthesesHelper(std::string_view sv) {
    return validParentheses(sv);
}

bool oneTwoThree(std::string_view sv) {
    return parse(sv, lit_("one") >> (lit_("two") | empty_) >> lit_("three") >> end_);
}

TEST_CASE("test simple text parsing"){
    CHECK(parse("a", char_('a')));
    CHECK(parse("b", !char_('a')));
    CHECK(parse("ab", char_('a') | char_('b')));
    CHECK(parse("ab", +charset_("ab")));
    CHECK(parse("ab", +!charset_("cd")));
    CHECK(!parse("ca", +charset_("ab")));
    CHECK(!parse("bb", char_('a') >> char_('b')));
    CHECK(!parse("aa", char_('a') >> char_('b')));
    CHECK(parse("aa", lit_("aa")));
    CHECK(!parse("cc", lit_("aa")));
    CHECK(parse("", *char_('a')));
    CHECK(parse("a", *char_('a')));
    CHECK(parse("aaaa", *char_('a')));
    CHECK(!parse("bb", *char_('a') >> end_));
    CHECK(isHelloWorld("HelloWorld"));
    CHECK(isHelloWorld("Hello World"));
    CHECK(isHelloWorld("Hello \n\f\n\r\t\vWorld"));
    CHECK(!isHelloWorld("World \n\f\n\r\t\vHello"));
    CHECK(!isHelloWorld("Hello \n\f\n\r\t\vWorld extra"));
    CHECK(getSecondWord("test").empty());
    CHECK("2222" == getSecondWord("abcd 2222"));
    CHECK("2222" == getSecondWord("abcd 2222 defg"));
    CHECK("OK" == getMatch("GET OK"));
    CHECK("OK" == getMatch("POST OK"));
    CHECK(getMatch("NOPE OK").empty());
    CHECK(validParenthesesHelper("()"));
    CHECK(validParenthesesHelper("(())"));
    CHECK(validParenthesesHelper("(())()"));
    CHECK(!validParenthesesHelper("((())()"));
    CHECK(!validParenthesesHelper(")(())()"));
    CHECK(!validParenthesesHelper(""));
    CHECK(!validParenthesesHelper("((((("));
    CHECK(oneTwoThree("onethree"));
    CHECK(oneTwoThree("onetwothree"));

    // test the optional_
    CHECK(parse("ab@", *alnum_ >> optional_(char_('@')) >> end_));
    CHECK(parse("ab", *alnum_ >> optional_(char_('@')) >> end_));
    CHECK(parse("hello world", lit_("hello") >> optional_(lit_("world")) >> end_));
    CHECK(parse("hello", lit_("hello") >> optional_(lit_("world")) >> end_));
}

