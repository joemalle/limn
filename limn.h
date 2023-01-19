/// @file limn.h
/// @author Joseph Malle
/// @brief A tiny parser designed to compile quickly
/// @details Inspired by Boost X3.  See README.md

#pragma once

#include <cctype>
#include <string_view>
#include <functional> // support match function call

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

/// @namespace lm
/// @brief The namesapce for all Limn types, functions, and variables
namespace lm {
    namespace impl {

        /// call the skip function when we run the visit()
        constexpr bool skip(std::string_view& sv) noexcept {
            // whitespace could be: [ \t\r\n]+, see below
            // https://en.cppreference.com/w/cpp/string/byte/isspace
            if (!sv.empty() && 0 != std::isspace(sv.front())) { // skip the whitespace chars
                sv.remove_prefix(1);
                return true;
            }
            return false;
        };

        template <typename Base>
        struct parser_base {
            constexpr auto operator*() const noexcept;
            constexpr auto operator+() const noexcept;
            constexpr auto operator[](std::string_view& output) const noexcept;
            constexpr auto operator[](std::function<void(const std::string_view&)> callback) const noexcept;
        };
    }

    /// @class char_
    /// @brief Single character parser based on a char
    /// @details An object of this type parses a single character.
    ///     For example, `lm::char_('a')` will parse the string "a".
    struct char_ final : public impl::parser_base<char_> {
        /// @brief Construct a char_ parser
        /// @param[in] ch The character to accept.
        constexpr explicit char_(const char ch) noexcept
            : ch(ch)
        {}

        /// @brief Opposite parser
        /// @details Construct a parser that accepts all characters
        ///     besides the one that the base accepts.  For example,
        ///     `! lm::char_('a')` accepts "b", "c", etc but does not
        ///     accept "a"
        constexpr inline auto operator!() const& noexcept {
            struct not_ final : impl::parser_base<not_> {
                constexpr explicit not_(const char ch) noexcept
                    : ch(ch)
                {}

                constexpr inline bool visit(std::string_view& sv) const& noexcept {
                    if (!sv.empty() && sv.front() != ch) {
                        sv.remove_prefix(1);
                        return true;
                    }
                    return false;
                }

            private:
                char ch;
            };

            return not_(ch);
        }

        constexpr inline bool visit(std::string_view& sv) const& noexcept {
            if (!sv.empty() && sv.front() == ch) {
                sv.remove_prefix(1);
                return true;
            }
            return false;
        }

    private:
        char ch;
    };

    /// @class charset_
    /// @brief Single character parser for any of several characters
    /// @details An object of this type parses several characters
    ///     as specified in the argument.  For example,
    ///     `lm::charset_("abc")` will parse "a", "b", or "c".
    ///     lm::charset_ is equivalent to lm::char_ | lm::char | ...
    ///     lm::charset_ is a slow parser at runtime. You may be
    ///     able to improve performance by using `lm::char_if_` and
    ///     a faster callback.
    struct charset_ final : public impl::parser_base<charset_> {
        /// @brief Construct a charset_ parser
        /// @param[in] set A list of characters to accept.
        constexpr explicit charset_(char const* set) noexcept
            : set(set)
        {}

        /// @brief Opposite parser
        /// @details Construct a parser that accepts all characters
        ///     besides the ones that the base accepts.  For example,
        ///     `! lm::charset_("ab")` accepts "c", "d", etc but does not
        ///     accept "a" or "b"
        constexpr inline auto operator!() const& noexcept {
            struct not_ final : impl::parser_base<not_> {
                constexpr explicit not_(char const* set) noexcept
                    : set(set)
                {}

                constexpr inline bool visit(std::string_view& sv) const& noexcept {
                    if (!sv.empty()) {
                        for (char const* s = set; *s; ++s) {
                            if (*s == sv.front()) {
                                return false;
                            }
                        }
                        sv.remove_prefix(1);
                        return true;
                    }
                    return false;
                }

            private:
                char const* set;
            };

            return not_(set);
        }

        constexpr inline bool visit(std::string_view& sv) const& noexcept {
            if (!sv.empty()) {
                for (char const* s = set; *s; ++s) {
                    if (*s == sv.front()) {
                        sv.remove_prefix(1);
                        return true;
                    }
                }
            }
            return false;
        }

    private:
        char const* set;
    };

    /// @class char_if_
    /// @brief Single character parser based on a function
    /// @details An object of this type uses a predicate to
    ///     parse each character.  This way you can create
    ///     any single character parser imaginable.  For
    ///     example, `lm::char_if_(isEven)` would accept the
    ///     characters with even ASCII values such as "B".
    struct char_if_ final : public impl::parser_base<char_if_> {
        /// @brief Construct a char_if_ parser
        /// @param[in] pred The function that determines whether
        ///     to parse a character.
        constexpr explicit char_if_(bool(*pred)(char)) noexcept
            : pred(pred)
        {}

        constexpr inline bool visit(std::string_view& sv) const& noexcept {
            if (!sv.empty() && pred(sv.front())) {
                sv.remove_prefix(1);
                return true;
            }
            return false;
        }

    private:
        bool(*pred)(char);
    };

    // These are not constexpr because the underlying functions in cctype aren't

    /// @var alnum_
    /// @brief Single character parser based on std::isalnum
    [[maybe_unused ]] static inline auto alnum_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::isalnum(ch); });

    /// @var alpha_
    /// @brief Single character parser based on std::isalpha
    [[maybe_unused ]] static inline auto alpha_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::isalpha(ch); });

    /// @var lower_
    /// @brief Single character parser based on std::islower
    [[maybe_unused ]] static inline auto lower_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::islower(ch); });

    /// @var upper_
    /// @brief Single character parser based on std::isupper
    [[maybe_unused ]] static inline auto upper_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::isupper(ch); });

    /// @var digit_
    /// @brief Single character parser based on std::isdigit
    [[maybe_unused ]] static inline auto digit_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::isdigit(ch); });

    /// @var xdigit_
    /// @brief Single character parser based on std::xdigit
    [[maybe_unused ]] static inline auto xdigit_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::isxdigit(ch); });

    /// @var cntrl_
    /// @brief Single character parser based on std::cntrl
    [[maybe_unused ]] static inline auto cntrl_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::iscntrl(ch); });

    /// @var graph_
    /// @brief Single character parser based on std::graph
    [[maybe_unused ]] static inline auto graph_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::isgraph(ch); });

    /// @var space_
    /// @brief Single character parser based on std::space
    [[maybe_unused ]] static inline auto space_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::isspace(ch); });

    /// @var blank_
    /// @brief Single character parser based on std::blank
    [[maybe_unused ]] static inline auto blank_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::isblank(ch); });

    /// @var print_
    /// @brief Single character parser based on std::print
    [[maybe_unused ]] static inline auto print_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::isprint(ch); });

    /// @var punct_
    /// @brief Single character parser based on std::punct
    [[maybe_unused ]] static inline auto punct_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::ispunct(ch); });

    /// @class lit_
    /// @brief String literal parser
    /// @details An object of this type matches a character sequence (AKA
    ///     a string literal).  For example, `lm::lit_("tautological")`
    ///     would parse the string "tautological".
    struct lit_ final : public impl::parser_base<lit_> {
        /// @brief Construct a lit_ parser.
        /// @param[in] str The string literal to parse.
        constexpr lit_(std::string_view str) noexcept
            : str(str)
        {}

        constexpr inline bool visit(std::string_view& sv) const& noexcept {
            if (sv.substr(0, str.size()) == str) {
                sv.remove_prefix(str.size());
                return true;
            }
            return false;
        }

    private:
        std::string_view str;
    };

    /// @class action_
    /// @brief Customizable parser
    /// @details An object of this type uses a callback to
    ///     determine a match.  This enables recursive grammars
    ///     and interaction with other parsers.  For an example,
    ///     look at tests.cpp:validParentheses (which can also be
    ///     found in README.md).
    template <typename Func>
    struct action_ final : public impl::parser_base<action_<Func>> {
        /// @brief Construct an action_ parser
        /// @details This function receives
        ///     a string_view by reference and returns a bool.  The string_view
        ///     argument starts at the current parse position and ends at the
        ///     end of the input string.  To indicate a match, return true
        ///     and adjust the string_view's start position to the end of the
        ///     matched section.  If there is no match, the function must
        ///     return false and not modify the input.
        ///
        ///     For example, suppose you have a callback that parses "ABC".
        ///     You could have used `lm::lit_("ABC")`, but you like making things
        ///     complicated I guess.
        ///
        ///     Your function could be called with "ABCDEFG". In that case, you
        ///     would set the input to "DEFG" and return true.
        ///
        ///     If your function was invoked with "QWERTY", then you would
        ///     return false without modifying the input.
        ///
        /// @param[in] func The function to call.
        constexpr explicit action_(Func&& func) noexcept
            : func(std::forward<Func>(func))
        {}

        constexpr inline bool visit(std::string_view& sv) const& noexcept {
            return func(sv); // func returns false to fail the parse
        }

    private:
        Func func;
    };

    /// @class optional_
    /// @brief Zero or one of the match parser
    /// @details An object of this type happens for one match of the item or empty
    /// For example, `optional_( lm::lit_("hello") )` can parse the string "hello" or the empty
    /// string, so it is a simplied form of `lm::lit_("hello") | empty_`.
    template <typename T>
    struct optional_ final : public impl::parser_base<optional_<T>> {
        constexpr explicit optional_(T&& p) noexcept
            : base(std::forward<T>(p))
        {}

        constexpr inline bool visit(std::string_view& sv) const& noexcept {
            base.visit(sv);
            return true;
        }

    private:
        T base;
    };

    namespace impl {
        template <typename Left, typename Right>
        struct seq_ final : public impl::parser_base<seq_<Left, Right>> {
            constexpr explicit seq_(Left&& left, Right&& right) noexcept
                : left(std::forward<Left>(left))
                , right(std::forward<Right>(right))
            {}

            constexpr inline bool visit(std::string_view& sv) const& noexcept {
                impl::skip(sv);
                bool left_result = left.visit(sv);
                if (left_result)
                {
                    impl::skip(sv);
                    return right.visit(sv);
                }
                else
                    return false;
            }

        private:
            Left left;
            Right right;
        };

        template <typename Left, typename Right>
        struct alt_ final : public impl::parser_base<alt_<Left, Right>> {
            constexpr explicit alt_(Left&& left, Right&& right) noexcept
                : left(std::forward<Left>(left))
                , right(std::forward<Right>(right))
            {}

            constexpr inline bool visit(std::string_view& sv) const& noexcept {
                impl::skip(sv);
                const std::string_view save = sv; // rewind the string_view if left failed, save should not be reference
                return left.visit(sv) || right.visit(sv = save);  // reset the sv when calling the right parser
            }

        private:
            Left left;
            Right right;
        };

        template <typename Base>
        struct kleene_ final : public impl::parser_base<kleene_<Base>> {
            constexpr explicit kleene_(Base base) noexcept
                : base(std::move(base))
            {}

            constexpr inline bool visit(std::string_view& sv) const& noexcept {
                impl::skip(sv);
                std::string_view save = sv;
                // save != sv means we does step forward (base.visit(sv) consume some chars)
                // the asignment save = sv means the sv get updated, so try next loop
                // to see it goes forward again
                while (base.visit(sv) && !sv.empty() && save != sv)
                    save = sv;
                return true;
            }

        private:
            Base base;
        };

        template <typename Base>
        struct plus_ final : public impl::parser_base<plus_<Base>> {
            constexpr explicit plus_(Base base) noexcept
                : base(std::move(base))
            {}

            constexpr inline bool visit(std::string_view& sv) const& noexcept {
                impl::skip(sv);
                if (!base.visit(sv)) {
                    return false;
                }
                std::string_view save = sv;
                // save != sv means we does step forward (base.visit(sv) consume some chars)
                // the asignment save = sv means the sv get updated, so try next loop
                // to see it goes forward again
                while (base.visit(sv) && !sv.empty() && save != sv)
                    save = sv;
                return true;
            }

        private:
            Base base;
        };

        template <typename Base>
        struct match_ final : public impl::parser_base<match_<Base>> {
            constexpr explicit match_(Base base, std::string_view& sv) noexcept
                : base(std::move(base))
                , out(sv)
            {}

            constexpr inline bool visit(std::string_view& sv) const& noexcept {
                impl::skip(sv);
                std::string_view save = sv;
                if (base.visit(sv)) {
                    out = save.substr(0, save.size() - sv.size());
                    return true;
                }
                return false;
            }

        private:
            Base base;
            std::string_view& out;
        };

        template <typename Base>
        struct match_call_ final : public impl::parser_base<match_call_<Base>> {
            constexpr explicit match_call_(Base base, std::function<void(const std::string_view&)> callback) noexcept
                : base(std::move(base))
                , callback(std::move(callback))
            {}

            constexpr inline bool visit(std::string_view& sv) const& noexcept {
                impl::skip(sv);
                std::string_view save = sv;
                if (base.visit(sv)) {
                    callback(save.substr(0, save.size() - sv.size()));
                    return true;
                }
                return false;
            }

        private:
            Base base;
            std::function<void(const std::string_view&)> callback;
        };

        struct endtype_ final : public impl::parser_base<endtype_> {
            constexpr inline bool visit(std::string_view& sv) const& noexcept {
                return sv.empty();
            }
        };

        struct emptytype_ final : public impl::parser_base<emptytype_> {
            constexpr inline bool visit(std::string_view&) const& noexcept {
                return true;
            }
        };
    }

    /// @var end_
    /// @brief End of input parser
    /// @details This object matches the end of the input.  For example,
    ///     `lm::end_` by itself matches "".  Usually this parser is used
    ///     in combination with another parser to ensure that the entire
    ///     input is matched.  `lm::lit("Prefix")` matches "Prefix", but it
    ///     also matches "PrefixSuffix", "PrefixSuffixSuffix", etc.
    ///     `lm::lit("Prefix") >> lm::end_` will only match "Prefix".
    [[maybe_unused]] constexpr static inline auto end_ = impl::endtype_();

    /// @var empty_
    /// @brief Empty parser
    /// @details This object matches anything.  It does not advance the parser's
    ///     position.  It is occasionally useful to define optional matches with
    ///     the pattern `(parser | empty)`. For example,
    ///     `lm::lit_("Hello") >> (lm::lit_("World") | lm::empty_) >> lm::end_`
    ///     would match "Hello" and "HelloWorld".  The word "World" is optional
    ///     in this parser
    [[maybe_unused]] constexpr static inline auto empty_ = impl::emptytype_();

    /// @brief The sequence parser combinator
    /// @details This function combines two parsers in sequence.  For example,
    ///     `lm::lit_("Hello") >> lm::lit_("World")` parses "HelloWorld" using
    ///     two literal parsers combined in sequence.
    ///
    ///     If this first parser in a sequence fails, then the second parser
    ///     is never evaluated.
    ///
    ///     This operator is found using ADL.
    ///
    /// @param[in] left The first parser to evaluate.
    /// @param[in] right The second parser to evaluate.  Only evaluated if
    ///     the first parser succeeded.
    template <typename Left, typename Right>
    constexpr inline auto operator>>(Left&& left, Right&& right) noexcept {
        return impl::seq_<Left, Right>(
            std::forward<Left>(left),
            std::forward<Right>(right)
        );
    }

    /// @brief The alternate parser combinator
    /// @details This function combines two parsers as alternatives.  For example,
    ///     `lm::lit_("Hello") | lm::lit_("World")` parses "Hello" or "World" using
    ///     two literal parsers.
    ///
    ///     If this first parser in a sequence fails, then the second parser
    ///     is evaluated.  If the first parser succeeds, then the second parser
    ///     is never evaluated.  This is analogous to "short circuiting" of ||.
    ///
    ///     This operator is found using ADL.
    ///
    /// @param[in] left The first parser to evaluate.  If true, then the combined
    ///     parse succeeds.
    /// @param[in] right The second parser to evaluate.  Only evaluated if the
    ///     first parser failed.
    template <typename Left, typename Right>
    constexpr inline auto operator|(Left&& left, Right&& right) noexcept {
        return impl::alt_<Left, Right>(
            std::forward<Left>(left),
            std::forward<Right>(right)
        );
    }

    /// @brief The Kleene star or "any number of times" parser combinator
    /// @details This function returns a parser that matches its input
    ///     any number of times (including 0 times).  For example,
    ///     `* lm::char_('a')` matches "", "a", "aa", etc.  This is a greedy
    ///     match; `* lm::char_('a') >> lm::lit_("aa")` will NEVER parse an
    ///     input string because the Kleene star will eat up all the 'a's.
    template <typename Base>
    constexpr inline auto impl::parser_base<Base>::operator*() const noexcept {
        return impl::kleene_<Base>(*static_cast<Base const*>(this));
    }

    /// @brief The plus parser or "one or more" parser combinator
    /// @details This function returns a parser that matches its input
    ///     one or more times (not including 0 times).  For example,
    ///     `+ lm::char_('a')` matches "a", "aa", etc.  This is a greedy
    ///     match; `+ lm::char_('a') >> lm::lit_("aa")` will NEVER parse an
    ///     input string because the plus parser will eat up all the 'a's.
    template <typename Base>
    constexpr inline auto impl::parser_base<Base>::operator+() const noexcept {
        return impl::plus_<Base>(*static_cast<Base const*>(this));
    }

    /// @brief The match operator
    /// @details This side-effect-only function copies the matched part of
    ///     input to its argument output.  If there is no match, then output
    ///     is unchanged.  For example, `(* lm::char_('a'))[output]` will match
    ///     any number of `a` characters in sequence, and it will copy that
    ///     sequence to the `std::string_view` `output`.
    ///
    ///     This is useful for getting the matched string out of the parser.
    ///     It's also sometimes useful for printf-debugging.
    ///
    ///     See tests.cpp or README.md for an example.
    ///
    /// @param[out] output The argument to receive the match portion.
    template <typename Base>
    constexpr inline auto impl::parser_base<Base>::operator[](std::string_view& ouput) const noexcept {
        return impl::match_<Base>(*static_cast<Base const*>(this), ouput);
    }

    /// @brief The match callback operator
    /// @details The std::function will be called when an item get matched.
    ///     The std::function should have the protytype
    ///     `std::function<void(const std::string_view&)>` where the argument
    ///     is the matched string_view.
    ///     If there is no match, then this function will not be called.
    ///     For example, `(* lm::char_('a'))[functor]` will match
    ///     any number of `a` characters in sequence, and it will call the functor.
    ///
    ///     This is useful for add the user defined action in this std::function
    ///     For example, adding the AST of the matched item to the parsing tree
    ///
    ///     See tests.cpp or README.md for an example.
    ///
    /// @param[in] callback The std::function callback function which will be called when
    ///     matched portion, the functor's parameter is the matched string_view.
    template <typename Base>
    constexpr inline auto impl::parser_base<Base>::operator[](std::function<void(const std::string_view&)> callback) const noexcept {
        return impl:: match_call_<Base>(*static_cast<Base const*>(this), std::move(callback));
    }

    /// @brief The parse function
    /// @details This is the top level function you should call to evaluate
    ///     a parser with an input.
    ///
    ///     For example:
    ///
    ///         parse(
    ///             "Hello World",
    ///             lm::lit_(Hello) >> lm::space_ >> lm::lit_("World") >> lm::end_
    ///         );
    ///
    ///     ...would return true
    ///
    /// @param[in] input The input string to parse
    /// @param[in] parser The parser to evaluate on \p input
    /// @returns true if the parser matched the input or false otherwise
    template <typename Parser>
    constexpr bool parse(std::string_view input, Parser const& parser) noexcept {
        return parser.visit(input);
    }

    /// @brief The parse function that takes \p input by reference
    /// @details This function is useful for recursive grammars. For
    ///     an example, see tests.cpp or README.md.
    ///
    ///     Prefer to use `lm::parse()`.
    ///
    /// @param[inout] input The input string to parse
    /// @param[in] parser The parser to evaluate on \p input
    /// @returns true if the parser matched the input or false otherwise
    template <typename Parser>
    constexpr bool parse_ref(std::string_view& input, Parser const& parser) noexcept {
        return parser.visit(input);
    }
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
