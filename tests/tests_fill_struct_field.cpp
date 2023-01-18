#include "limn.h"

#include <cassert>
#include <string>

#include <iostream> // std::cout


#include <doctest/doctest.h>


using namespace lm; // Laziness


struct abc
{
    void set_a(const std::string_view& out) {a = out[0];};
    void set_b(const std::string_view& out) {b = out[0];};
    void set_c(const std::string_view& out) {c = out[0];};
    void build_abc(const std::string_view& out)
    {
        std::cout << a << b << c << std::endl;
    }
private:
    char a;
    char b;
    char c;
};

abc g;

std::function<void(const std::string_view&)> fa = std::bind(&abc::set_a, &g, std::placeholders::_1);
std::function<void(const std::string_view&)> fb = std::bind(&abc::set_b, &g, std::placeholders::_1);
std::function<void(const std::string_view&)> fc = std::bind(&abc::set_c, &g, std::placeholders::_1);
std::function<void(const std::string_view&)> fabc = std::bind(&abc::build_abc, &g, std::placeholders::_1);

TEST_CASE("test fill struct field"){
    CHECK(parse("aec abc",  *(   ( (char_('a')[fa] >> char_('b')[fb])
                            | (char_('a')[fa] >> char_('e')[fb]) )  >> char_('c')[fc] >> *space_)[fabc]));
}


