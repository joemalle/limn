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

auto ident = lexeme_(alpha_ >> *alnum_);

// B<x = 5, y = int>
auto scope_name = ident >> opt_(char_('<') >> action_(&ReadTemplateSpecializationParameters) >> char_('>'));

// A::B<x = 5, y = int>::C
auto qualified_name = opt_(lit_("::")) >> scope_name >> *( lit_("::") >> scope_name);


auto template_single_arg = (lit_("class") | lit_("typename")) >> ident >> opt_(char_('=') >> ident);
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
auto function_arg_default_value = opt_(char_('=') >> action_(&ReadFunctionSingleParameter));
auto function_single_arg = +( qualified_name | pointer_reference_const_qualifier | empty_) >> function_arg_default_value;

// comma seperated function argument
// int a, float b = 6, double c = f(), char d
auto function_arg_list =  *function_single_arg >> *(char_(',') >> function_single_arg);

auto template_function_declaration_grammar = lit_("template") >> char_('<') >> template_arg_list[p] >> char_('>')
    >> +qualified_name[p] >> char_('(')[p] >> function_arg_list[p] >> char_(')')[p] >> char_(';');

//    auto ident = (+alnum_);
//
//    // B<x = 5, y = int>
//    auto scope_name = ident >> (char_('<') >> action_(&ReadTemplateSpecializationParameters) >> char_('>')  | empty_);
//
//    // A::B<x = 5, y = int>::C
//    auto qualified_name = (lit_("::") | empty_) >> scope_name >> *( lit_("::") >> scope_name);
//


// demonstrate how to use the parser inside a class and fill the class members
struct FunctionDeclarationTag {

    std::string return_type;
    std::string name;
    std::string scope;
    std::string args;
    std::string template_args;

    std::vector<std::string_view> qualified_name_vector;

    bool Parse(std::string input);

    void SetReturnType(const std::string_view& sv) {return_type = sv;}
    void PushQualifiedName(const std::string_view& sv) {
        qualified_name_vector.push_back(std::string(sv));
    }
    void SetArgs(const std::string_view& sv) {args = sv;}

    void Finish()
    {
        std::cout << "Finish function:" << std::endl;
        for (auto item : qualified_name_vector)
            std::cout << item << std::endl;

        // the last item of the qualified_name_vector is the function name
        // "A::B<m>::C X::Y<u>::Z();
        // in the above case, the "Z" is the function, it is defined in the scope "X::Y<u>::"
        // and has the return type "A::B<m>::C"

        if (qualified_name_vector.size() == 0)
            return;

        std::string last_scope_name = std::string(qualified_name_vector.back());
        if (last_scope_name.length() > 0)
        {
            std::string_view ident_str;
            std::string_view template_args_str;
            // B<x = 5, y = int>
            auto scope_name = ident[ident_str] >> opt_(char_('<') >> action_(&ReadTemplateSpecializationParameters)[template_args_str] >> char_('>'));
            parse(last_scope_name, scope_name);
            name = std::string(ident_str);
            template_args = std::string(template_args_str);
        }
    }

    // clear the member variables
    void Init()
    {
        return_type.clear();
        name.clear();
        scope.clear();
        args.clear();
    }
};


bool FunctionDeclarationTag::Parse(std::string input)
{
    Init();
    auto push_var = [&](const std::string_view& sv){
        qualified_name_vector.push_back(sv);
    };
    auto set_args = [&](const std::string_view& sv){
        args = sv;
    };
    auto ident = lexeme_(alpha_ >> *alnum_);

    // B<x = 5, y = int>
    auto scope_name = ident[p] >> opt_(char_('<') >> action_(&ReadTemplateSpecializationParameters) >> char_('>'));

    // A::B<x = 5, y = int>::C
    auto qualified_name = opt_(lit_("::")) >> scope_name >> *( lit_("::") >> scope_name);

    std::cout << "start parse!!!" << std::endl;
    auto function_declaration = opt_(lit_("const"))
        >> +(qualified_name[push_var] >> opt_(pointer_reference_const_qualifier))
        >> char_('(') >> function_arg_list[set_args] >> char_(')')>> char_(';');
    if (parse(input, function_declaration >> end_) == true)
    {
        Finish();
        return true;
    }
    else
    {
        std::cout << "failed!!!" << std::endl;
        return false;
    }
}


TEST_CASE("parrsing the C++ function declaration") {
    FunctionDeclarationTag f;
    CHECK(f.Parse("int sum (int x, int y);"));
    CHECK(f.Parse("int* sum (int x, int y);"));
    CHECK(f.Parse("T A::B::fun();"));
    CHECK(f.Parse("T A::B<x = 5, y = int>::fun(T x = 5, T y, unsigned int u = 6);"));
    CHECK(f.Parse("A::B X::Y::fun();"));
    CHECK(f.Parse("T U X A::B::C();"));
    CHECK(f.Parse("T U X A::B::C(T x = C, T y, D* u);"));
    CHECK(f.Parse("T A::B<x = 5, y = int>::fun(T x = 5, T y, unsigned int u = 6);"));
    CHECK(f.Parse("T A::B<x = 5, y = int>::fun(T x = 5, T y, unsigned int u = 6);"));
    CHECK(f.Parse("A::B<m>::C X();"));
    CHECK(f.Parse("A::B<m>::C X::Y<u>::Z();"));
}

}  // unnamed namespace
