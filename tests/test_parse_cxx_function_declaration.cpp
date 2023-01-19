#include "limn.h"
#include <iostream> // std::cout
#include <doctest/doctest.h>
#include <vector>




namespace {
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

// todo, should consider/skip inner () inside a toplevel ()
// so the whole function body is read
constexpr bool ReadFunctionArgs(std::string_view& sv) {
    return parse_ref(sv, *(!char_(')')));
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
auto scope_name = id >> optional_(char_('<') >> action_(&ReadTemplateSpecializationParameters) >> char_('>'));

// A::B<x = 5, y = int>::C
auto qualified_name = optional_(lit_("::")) >> scope_name >> *( lit_("::") >> scope_name);


auto template_single_arg = (lit_("class") | lit_("typename")) >> id >> optional_(char_('=') >> id);
auto template_arg_list = template_single_arg[p] >> *(char_(',') >> template_single_arg);

constexpr
auto pointer_reference_const_qualifier = *(lit_("const") | lit_("*") | lit_("&"));

// unsigned int a
// A::B::C a
// A<int, float>::B::C a
// unsigned int * a
// float & a
// float && a
// const int a
// const int * a
auto function_arg_default_value = (char_('=') >> action_(&ReadFunctionSingleParameter)) | empty_;
auto function_single_arg = +( qualified_name | pointer_reference_const_qualifier | empty_) >> function_arg_default_value;

// comma seperated function argument
// int a, float b = 6, double c = f(), char d
auto function_arg_list =  *function_single_arg >> *(char_(',') >> function_single_arg);

auto template_function_declaration_grammar = lit_("template") >> char_('<') >> template_arg_list[p] >> char_('>')
    >> +qualified_name[p] >> char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> char_(';');

//    auto id = (+alnum_);
//
//    // B<x = 5, y = int>
//    auto scope_name = id >> (char_('<') >> action_(&ReadTemplateSpecializationParameters) >> char_('>')  | empty_);
//
//    // A::B<x = 5, y = int>::C
//    auto qualified_name = (lit_("::") | empty_) >> scope_name >> *( lit_("::") >> scope_name);
//


//struct FunctionDeclarationTag {

    std::string return_type;
    std::string name;
    std::string scope;
    std::string args;

    std::vector<std::string> qualified_name_vector;

    //bool Parse(std::string input);

    void SetReturnType(const std::string_view& sv) {return_type = sv;}
    void PushQualifiedName(const std::string_view& sv) {qualified_name_vector.push_back(std::string(sv));}
    void SetArgs(const std::string_view& sv) {args = sv;}

    void Finish()
    {
        for (auto item : qualified_name_vector)
            std::cout << item << std::endl;
    }
//};


auto function_declaration = optional_(lit_("const"))
    >> +((qualified_name)[PushQualifiedName] >> optional_(pointer_reference_const_qualifier))
    >> char_('(') >> function_arg_list[SetArgs] >> char_(')')>> char_(';');


TEST_CASE("parrsing the C++ function declaration") {

    CHECK(parse("int sum (int x, int y);", function_declaration));
    CHECK(parse("int* sum (int x, int y);", function_declaration));
    CHECK(parse("T A::B::fun();", function_declaration >> end_));
    CHECK(parse("T A::B<x = 5, y = int>::fun(T x = 5, T y, unsigned int u = 6);", function_declaration >> end_));
    CHECK(parse("A::B X::Y::fun();", function_declaration >> end_));
    CHECK(parse("T U X A::B::C();", function_declaration >> end_));
    CHECK(parse("T U X A::B::C(T x = C, T y, D* u);", function_declaration >> end_));
    CHECK(parse("T A::B<x = 5, y = int>::fun(T x = 5, T y, unsigned int u = 6);", function_declaration >> end_));
    CHECK(parse("T A::B<x = 5, y = int>::fun(T x = 5, T y, unsigned int u = 6);", function_declaration >> end_));
    CHECK(parse("A::B<m>::C X();", function_declaration >> end_));
    std::cout << "print vector:" << std::endl;
    Finish();

}

}  // unnamed namespace
