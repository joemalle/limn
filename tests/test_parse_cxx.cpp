#include "limn.h"
#include <iostream> // std::cout
#include <doctest/doctest.h>


using namespace lm; // Laziness

// todo, should consider/skip inner comma inside a parentheses
// for example
// int a = f(2,3), int b
// we should stop at the ")", not the "2"
constexpr bool ReadFunctionSingleParameter(std::string_view& sv) {
    return parse_ref(sv, *(!charset_(",)")));
}

// todo, should consider/skip inner <> inside a toplevel <>
// for example
// U = A::B<int, float>, V = double
// we should stop at the end, not after "float>"
constexpr bool ReadTemplateSpecializationParameters(std::string_view& sv) {
    return parse_ref(sv, *(!char_('>')));
}

// todo, should consider/skip inner {} inside a toplevel {}
// so the whole function body is read
constexpr bool ReadFunctionBody(std::string_view& sv) {
    return parse_ref(sv, *(!char_('}')));
}

auto p = [](const std::string_view& output){
    if (output.empty())
        std::cout << "Empty Input Matches!!!" << std::endl;
    else
        std::cout << output << std::endl;
};

auto id = (+alnum_);

// B<x = 5, y = int>
auto scope_name = id >> (char_('<') >> action_(&ReadTemplateSpecializationParameters) >> char_('>')  | empty_);

// A::B<x = 5, y = int>::C
auto qualified_name = opt_(lit_("::")) >> scope_name >> *( lit_("::") >> scope_name);


auto template_single_arg = (lit_("class") | lit_("typename")) >> id >> opt_((char_('=') >> id));
auto template_arg_list = template_single_arg[p] >> *(char_(',') >> template_single_arg);

auto pointer_reference_const_qualifier = *(lit_("const") | lit_("*") | lit_("&"));

// unsigned int a
// A::B::C a
// A<int, float>::B::C a
// unsigned int * a
// float & a
// float && a
// const int a
// const int * a
auto function_arg_default_value = opt_(char_('=') >> action_(&ReadFunctionSingleParameter));
auto function_single_arg = +( qualified_name | pointer_reference_const_qualifier | empty_) >> function_arg_default_value;

// comma seperated function argument
// int a, float b = 6, double c = f(), char d
auto function_arg_list =  *function_single_arg >> *(char_(',') >> function_single_arg);

auto template_function_declaration_grammar = lit_("template") >> char_('<') >> template_arg_list[p] >> char_('>')
    >> +qualified_name[p] >> char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> char_(';');


// a lambda function looks like below:
// [ capture clause ] (parameters) -> return-type
// {
//    definition of method
// }
// [ captures ] ( params ) specs requires(optional) { body }

// specs consists of specifiers, exception, attr and trailing-return-type in that order;
// each of these components is optional

auto lambda_return_type =  opt_( lit_("->") >> qualified_name ); //optional
auto lambda_noexcept_specifier = opt_( lit_("[[") >> id >> lit_("]]") ); //optional
auto lambda_parameters = opt_( char_('(') >> function_arg_list >> char_(')') ); //optional
auto lambda_function_definition = char_('[') >> function_arg_list >> char_(']') >> lambda_parameters
    >> lambda_noexcept_specifier >> lambda_return_type >> char_('{') >> action_(&ReadFunctionBody) >> char_('}');


TEST_CASE("testing of parrsing the C++ code") {

    CHECK(parse("ab@", *alnum_[p] >> char_('@') >> end_));

    CHECK(parse("@", *(*alnum_) >> char_('@') >> end_));

    CHECK(parse("T x", function_single_arg >> end_));
    CHECK(parse("T x = u", function_single_arg >> end_));
    CHECK(parse("T* x", function_single_arg >> end_));
    CHECK(parse("T *x = u", function_single_arg >> end_));

    CHECK(parse("class U = int", template_single_arg[p] >> end_));
    CHECK(parse("class U", template_single_arg[p] >> end_));


    CHECK(parse("typename T, class U = int, typename X", template_arg_list >> end_));

    CHECK(parse("A::B<x = 5, y = int>::C", qualified_name >> end_));
    CHECK(parse("A::B::C", qualified_name >> end_));
    CHECK(parse("::A::B::C", qualified_name >> end_));

    CHECK(parse("()", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));
    CHECK(parse("(int a)", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));
    CHECK(parse("(auto a)", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));
    CHECK(parse("(int* a)", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));
    CHECK(parse("(int& a)", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));
    CHECK(parse("(int** a)", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));
    CHECK(parse("(int&& a)", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));

    CHECK(parse("(A::B::C a)", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));
    CHECK(parse("(A::B::C a, A::B::C b, A::B::C c)", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));
    CHECK(parse("(T x = C)", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));
    CHECK(parse("(T x = C, T y, D* u)", char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> end_));

    CHECK(parse("template <typename T> T A::B::fun();", template_function_declaration_grammar >> end_));
    CHECK(parse("template <typename T> T A::B<x = 5, y = int>::fun(T x = 5, T y, unsigned int u = 6);", template_function_declaration_grammar >> end_));
    CHECK(parse("template <typename T> A::B X::Y::fun();", template_function_declaration_grammar >> end_));
    CHECK(parse("template <typename T> T U X A::B::C();", template_function_declaration_grammar >> end_));
    CHECK(parse("template <typename T> T U X A::B::C(T x = C, T y, D* u);", template_function_declaration_grammar >> end_));
    CHECK(parse("template <typename T> T A::B<x = 5, y = int>::fun(T x = 5, T y, unsigned int u = 6);", template_function_declaration_grammar >> end_));


    SUBCASE("lambda"){
        CHECK(parse("[]", char_('[') >> function_arg_list >> char_(']') >> end_));
        CHECK(parse("(auto a, auto&& b)",  char_('(') >> function_arg_list >> char_(')')  >> end_));

        CHECK(parse("{ return a < b; }",  char_('{') >> action_(&ReadFunctionBody) >> char_('}') >> end_));
        CHECK(parse("-> int",  lambda_return_type >> end_));
        CHECK(parse("[](auto a, auto&& b) { return a < b; }",  lambda_function_definition >> end_));
        CHECK(parse("[](auto a, auto&& b) -> int { return a < b; }", lambda_function_definition >> end_));

    }

}
