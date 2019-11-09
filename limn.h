// Limn
// A tiny parser designed to compile quickly
// Inspired by Boost X3
// see tests.cpp for examples

#pragma once

#include <cctype>
#include <string_view>

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace lm {
	namespace impl {
		template <typename Base>
		struct parser_base {
			constexpr auto operator*() const noexcept;
			constexpr auto operator+() const noexcept;
			constexpr auto operator[](std::string_view&) const noexcept;
		};
	}

	/// Match a single type of character
	struct char_ final : public impl::parser_base<char_> {
		constexpr explicit char_(const char ch) noexcept
			: ch(ch)
		{}

		constexpr inline bool visit(std::string_view& sv) const& noexcept {
			if (!sv.empty() && sv.front() == ch) {
				sv.remove_prefix(1);
				return true;
			}
			return false;
		}

		char ch;
	};

	/// Supply a callback to determine if the character matches
	struct char_if_ final : public impl::parser_base<char_if_> {
		constexpr explicit char_if_(bool(*func)(char)) noexcept
			: func(func)
		{}

		constexpr inline bool visit(std::string_view& sv) const& noexcept {
			if (!sv.empty() && func(sv.front())) {
				sv.remove_prefix(1);
				return true;
			}
			return false;
		}

		bool(*func)(char);
	};

#pragma push_macro("char_if_cctype")
#define char_if_cctype(X) \
	[[maybe_unused ]] constexpr static inline auto X##_ = char_if_([](char const ch) noexcept -> bool { return 0 != std::is##X(ch); })
	
	char_if_cctype(alnum);
	char_if_cctype(alpha);
	char_if_cctype(lower);
	char_if_cctype(upper);
	char_if_cctype(digit);
	char_if_cctype(xdigit);
	char_if_cctype(cntrl);
	char_if_cctype(graph);
	char_if_cctype(space);
	char_if_cctype(blank);
	char_if_cctype(print);
	char_if_cctype(punct);
#undef char_if_cctype
#pragma pop_macro("char_if_cctype")

	/// Match a literal string
	struct lit_ final : public impl::parser_base<lit_> {
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

		std::string_view str;
	};

	/// Call a function when parsing reaches this parser
	/// The function will be called with a view of the remaining string 
	/// The function must return whether to continue parsing
	struct action_ final : public impl::parser_base<action_> {
		constexpr explicit action_(bool(*func)(std::string_view&)) noexcept
			: func(func)
		{}

		constexpr inline bool visit(std::string_view& sv) const& noexcept {
			return func(sv); // func returns false to fail the parse
		}

		bool(*func)(std::string_view&);
	};

	namespace impl {
		template <typename Left, typename Right>
		struct seq_ final : public impl::parser_base<seq_<Left, Right>> {
			constexpr explicit seq_(Left&& left, Right&& right) noexcept
				: left(std::forward<Left>(left))
				, right(std::forward<Right>(right))
			{}

			constexpr inline bool visit(std::string_view& sv) const& noexcept {
				return left.visit(sv) && right.visit(sv);
			}
	
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
				return left.visit(sv) || right.visit(sv);
			}
	
			Left left;
			Right right;
		};

		template <typename Base>
		struct kleene_ final : public impl::parser_base<kleene_<Base>> {
			constexpr explicit kleene_(Base base) noexcept
				: base(std::move(base))
			{}

			constexpr inline bool visit(std::string_view& sv) const& noexcept {
				while (base.visit(sv));
				return true;
			}

			Base base;
		};

		template <typename Base>
		struct plus_ final : public impl::parser_base<plus_<Base>> {
			constexpr explicit plus_(Base base) noexcept
				: base(std::move(base))
			{}

			constexpr inline bool visit(std::string_view& sv) const& noexcept {
				if (!base.visit(sv)) {
					return false;
				}
				while (base.visit(sv));
				return true;
			}

			Base base;
		};
		
		template <typename Base>
		struct export_ final : public impl::parser_base<export_<Base>> {
			constexpr explicit export_(Base base, std::string_view& sv) noexcept
				: base(std::move(base))
				, out(sv)
			{}

			constexpr inline bool visit(std::string_view& sv) const& noexcept {
				std::string_view save = sv;
				if (base.visit(sv)) {
					out = save.substr(0, save.size() - sv.size());
					return true;
				}
				return false;
			}

			Base base;
			std::string_view& out;
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

	/// Match the end of the string
	[[maybe_unused]] constexpr static inline auto end_ = impl::endtype_();

	/// Match nothing
	[[maybe_unused]] constexpr static inline auto empty_ = impl::emptytype_();

	/// A >> B means match A and then match B
	template <typename Left, typename Right>
	constexpr inline auto operator>>(Left&& left, Right&& right) noexcept {
		return impl::seq_<Left, Right>(
			std::forward<Left>(left),
			std::forward<Right>(right)
		);
	}

	/// A | B means try to match A, and if that fails, try to match B
	template <typename Left, typename Right>
	constexpr inline auto operator|(Left&& left, Right&& right) noexcept {
		return impl::alt_<Left, Right>(
			std::forward<Left>(left),
			std::forward<Right>(right)
		);
	}

	/// Match a parser as many times as possible (greedy Kleene star)
	template <typename Base>
	constexpr inline auto impl::parser_base<Base>::operator*() const noexcept {
		return impl::kleene_<Base>(*static_cast<Base const*>(this));
	}

	/// Match a parser one or more times
	template <typename Base>
	constexpr inline auto impl::parser_base<Base>::operator+() const noexcept {
		return impl::plus_<Base>(*static_cast<Base const*>(this));
	}
	
	/// Export a string view of the match
	template <typename Base>
	constexpr inline auto impl::parser_base<Base>::operator[](std::string_view& sv) const noexcept {
		return impl::export_<Base>(*static_cast<Base const*>(this), sv);
	}

	/// Attempt to match `sv` with `parser`
	template <typename Parser>
	constexpr bool parse(std::string_view sv, Parser const& parser) noexcept {
		return parser.visit(sv);
	}

	/// Attempt to match `sv` with `parser`
	template <typename Parser>
	constexpr bool parse_ref(std::string_view& sv, Parser const& parser) noexcept {
		return parser.visit(sv);
	}
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
