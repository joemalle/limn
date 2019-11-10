#include "limn.h"

#include <cassert>
#include <string>

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
        >> space_
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
    parse(sv, (lit_("GET") | lit_("POST")) >> space_ >> (*alnum_)[out]);
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

int main () {
    assert(parse("a", char_('a')));
    assert(parse("b", !char_('a')));
    assert(parse("ab", char_('a') | char_('b')));
    assert(parse("ab", +charset_("ab")));
    assert(parse("ab", +!charset_("cd")));
    assert(!parse("ca", +charset_("ab")));
    assert(!parse("bb", char_('a') >> char_('b')));
    assert(!parse("aa", char_('a') >> char_('b')));
    assert(parse("aa", lit_("aa")));
    assert(!parse("cc", lit_("aa")));
    assert(parse("", *char_('a')));
    assert(parse("a", *char_('a')));
    assert(parse("aaaa", *char_('a')));
    assert(!parse("bb", *char_('a') >> end_));
    assert(isHelloWorld("HelloWorld"));
    assert(isHelloWorld("Hello World"));
    assert(isHelloWorld("Hello \n\f\n\r\t\vWorld"));
    assert(!isHelloWorld("World \n\f\n\r\t\vHello"));
    assert(!isHelloWorld("Hello \n\f\n\r\t\vWorld extra"));
    assert(getSecondWord("test").empty());
    assert("2222" == getSecondWord("abcd 2222"));
    assert("2222" == getSecondWord("abcd 2222 defg"));
    assert("OK" == getMatch("GET OK"));
    assert("OK" == getMatch("POST OK"));
    assert(getMatch("NOPE OK").empty());
    assert(validParenthesesHelper("()"));
    assert(validParenthesesHelper("(())"));
    assert(validParenthesesHelper("(())()"));
    assert(!validParenthesesHelper("((())()"));
    assert(!validParenthesesHelper(")(())()"));
    assert(!validParenthesesHelper(""));
    assert(!validParenthesesHelper("((((("));
	assert(oneTwoThree("onethree"));
	assert(oneTwoThree("onetwothree"));
}
