#include "limn.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <string>

using namespace lm;

// Match "Hello", as many spaces as necessary, and then "World"
bool isHelloWorld(std::string_view sv) {
	return parse(sv, lit_("Hello") >> *space_ >> lit_("World"));
}

// return the word after the first space or an empty string
// the second word must consist of printable characters
std::string_view getSecondWord(std::string_view sv) {
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
std::string_view getMatch(std::string_view sv) {
    std::string_view out;
    parse(sv, (lit_("GET") | lit_("POST")) >> space_ >> (*alnum_)[out]);
    return out;
}

int main () {
	assert(parse("a", char_('a')));
	assert(parse("ab", char_('a') | char_('b')));
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
	assert(isHelloWorld("Hello \n\f\n\r\t\vWorld extra"));
	assert(getSecondWord("test").empty());
	assert("2222" == getSecondWord("abcd 2222"));
	assert("2222" == getSecondWord("abcd 2222 defg"));
	assert("OK" == getMatch("GET OK"));
	assert("OK" == getMatch("POST OK"));
	assert(getMatch("NOPE OK").empty());
}
