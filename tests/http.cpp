#include "limn.h"

#include <cassert>
#include <string_view>

// This is only meant to show a Limn example.  It is not meant
// to be a useful HTTP parser.
//
// Also I can't imagine you're ever parsing HTTP at compile time!
//
// As you can see at the bottom, all of this gets compiled to nothing.
// Takes 0.3-0.6 seconds to compile.  Occassionaly longer if I haven't
// recently used the compiler.

constexpr static auto method =
    lm::lit_("GET")
    | lm::lit_("HEAD")
    | lm::lit_("POST")
    | lm::lit_("PUT")
    | lm::lit_("DELETE")
    | lm::lit_("CONNECT")
    | lm::lit_("OPTIONS")
    | lm::lit_("TRACE");

constexpr static auto uri = +!lm::char_(' ');

// std::isdigit isn't constexpr...
constexpr static auto digit = lm::char_if_([](auto ch) {
    return '0' <= ch && ch <= '9';
});

constexpr static auto version =
    lm::lit_("HTTP/")
    >> digit
    >> lm::char_('.')
    >> digit;

constexpr static auto eol = lm::lit_("\r\n") | lm::char_('\n');

constexpr static auto header =
    +!lm::char_(':')
    >> lm::lit_(": ")
    >> +!lm::charset_("\r\n");

constexpr static auto headers = +(header >> eol);


constexpr static auto request =
    method >> lm::char_(' ') >> uri >> lm::char_(' ') >> version >> eol
    >> headers
    >> lm::end_;

constexpr auto parseHTTP(std::string_view input) {
    return lm::parse(input, request);
}

int main() {
    static_assert(
        parseHTTP("GET /hello.htm HTTP/1.1\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\nHost: www.tutorialspoint.com\r\nAccept-Language: en-us\r\nAccept-Encoding: gzip, deflate\r\nConnection: Keep-Alive\r\n")
    );
}