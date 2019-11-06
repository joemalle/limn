// Limn
// A tiny parser designed to compile quickly
// Inspired by Boost X3
// see tests.cpp for examples

#pragma once

#include <string_view>
#include <cctype>

namespace lm {
	namespace impl {
		template <typename Base>
		struct parser_base {
			auto operator*() const noexcept;
			auto operator[](std::string_view&) const noexcept;
		};
	}

	/// Match a single type of character
	struct char_ final : public impl::parser_base<char_> {
		explicit char_(
			const char ch
		) noexcept
			: ch(ch)
		{}

		inline bool visit(
			std::string_view::iterator& begin,
			std::string_view::iterator end
		) const& noexcept {
			if (begin != end && *begin == ch) {
				++begin;
				return true;
			}
			return false;
		}

		char const ch;
	};

	/// Supply a callback to determine if the character matches
	/// alnum_, alpha_, space_, and punct_ are defined for you
	struct char_if_ final : public impl::parser_base<char_if_> {
		explicit char_if_(
			bool(*cb)(char)
		) noexcept
			: cb(cb)
		{}

		inline bool visit(
			std::string_view::iterator& begin,
			std::string_view::iterator end
		) const& noexcept {
			if (begin != end && cb(*begin)) {
				++begin;
				return true;
			}
			return false;
		}

		bool(*cb)(char);
	};
	
	[[maybe_unused ]] inline auto const alnum_ = char_if_([](char ch) -> bool {return std::isalnum(ch); });
	[[maybe_unused ]] inline auto const alpha_ = char_if_([](char ch) -> bool {return std::isalpha(ch); });
	[[maybe_unused ]] inline auto const space_ = char_if_([](char ch) -> bool {return std::isspace(ch); });
	[[maybe_unused ]] inline auto const punct_ = char_if_([](char ch) -> bool {return std::ispunct(ch); });

	/// Match a literal string
	struct lit_ final : public impl::parser_base<lit_> {
		explicit lit_(
			char const* str
		) noexcept
			: str(str)
		{}

		bool visit(
			std::string_view::iterator& begin,
			std::string_view::iterator end
		) const& noexcept {
			// UGLY!
			int i = 0;
			for (; begin + i != end; ++i) {
				if (!str[i]) {
					begin += i;
					return true;
				}
				if (str[i] != *(begin + i)) {
					return false;
				}
			}
			return !str[i];
		}

		char const* str;
	};

	/// Call a function when parsing reaches this parser
	/// The function will be called with an iterator to the current parser position
	/// The function must return whether to continue parsing
	template <typename Func>
	struct action_ final : public impl::parser_base<action_<Func>> {
		explicit action_(
			Func func
		) noexcept
			: func(func)
		{}

		inline bool visit(
			std::string_view::iterator& begin,
			std::string_view::iterator end
		) const& noexcept {
			return func(begin, end); // func returns false to fail the parse
		}

		Func func;
	};

	namespace impl {
		template <typename Left, typename Right>
		struct seq_ final : public impl::parser_base<seq_<Left, Right>> {
			explicit seq_(
				Left&& left,
				Right&& right
			) noexcept
				: left(std::forward<Left>(left))
				, right(std::forward<Right>(right))
			{}

			inline bool visit(
				std::string_view::iterator& begin,
				std::string_view::iterator end
			) const& noexcept {
				return left.visit(begin, end) && right.visit(begin, end);
			}
	
			Left left;
			Right right;
		};

		template <typename Left, typename Right>
		struct alt_ final : public impl::parser_base<alt_<Left, Right>> {
			explicit alt_(
				Left&& left,
				Right&& right
			) noexcept
				: left(std::forward<Left>(left))
				, right(std::forward<Right>(right))
			{}

			inline bool visit(
				std::string_view::iterator& begin,
				std::string_view::iterator end
			) const& noexcept {
				return left.visit(begin, end) || right.visit(begin, end);
			}
	
			Left left;
			Right right;
		};

		template <typename Base>
		struct kleene_ final : public impl::parser_base<kleene_<Base>> {
			explicit kleene_(Base base) noexcept
				: base(base)
			{}

			inline bool visit(
				std::string_view::iterator& begin,
				std::string_view::iterator end
			) const& noexcept {
				while (base.visit(begin, end));
				return true;
			}

			Base base;
		};
		
		template <typename Base>
		struct export_ final : public impl::parser_base<export_<Base>> {
			explicit export_(
				Base base,
				std::string_view& sv
			) noexcept
				: base(base)
				, sv(sv)
			{}

			inline bool visit(
				std::string_view::iterator& begin,
				std::string_view::iterator end
			) const& noexcept {
				std::string_view::iterator matchBegin = begin;
				if (base.visit(begin, end)) {
					sv = std::string_view(matchBegin, begin - matchBegin);
					return true;
				}
				return false;
			}

			Base base;
			std::string_view& sv;
		};
	
		struct endtype_ final : public impl::parser_base<endtype_> {
			inline bool visit(
				std::string_view::iterator& begin,
				std::string_view::iterator end
			) const& noexcept {
				return begin == end;
			}
		};
	
		struct emptytype_ final : public impl::parser_base<emptytype_> {
			inline bool visit(
				std::string_view::iterator& begin,
				std::string_view::iterator end
			) const& noexcept {
				return true;
			}
		};
	}

	/// Match the end of the string
	[[maybe_unused]] inline auto const end_ = impl::endtype_();

	/// Match nothing
	[[maybe_unused]] inline auto const empty_ = impl::emptytype_();

	/// A >> B means match A and then match B
	template <typename Left, typename Right>
	inline auto operator>>(Left&& left, Right&& right) noexcept {
		return impl::seq_<Left, Right>(
			std::forward<Left>(left),
			std::forward<Right>(right)
		);
	}

	/// A | B means try to match A, and if that fails, try to match B
	template <typename Left, typename Right>
	inline auto operator|(Left&& left, Right&& right) noexcept {
		return impl::alt_<Left, Right>(
			std::forward<Left>(left),
			std::forward<Right>(right)
		);
	}

	/// Match a parser as many times as possible (greedy Kleene star)
	template <typename Base>
	inline auto impl::parser_base<Base>::operator*() const noexcept {
		return impl::kleene_<Base>(*reinterpret_cast<Base const*>(this));
	}
	
	/// Export a string view of the match
	template <typename Base>
	inline auto impl::parser_base<Base>::operator[](std::string_view& sv) const noexcept {
		return impl::export_<Base>(*reinterpret_cast<Base const*>(this), sv);
	}

	/// Attempt to match [`begin`, `end`) with `parser`
	template <typename Parser>
	bool parseIterators(std::string_view::iterator& begin, std::string_view::iterator end, Parser const& parser) noexcept {
		return parser.visit(begin, end);
	}

	/// Attempt to match `sv` with `parser`
	template <typename Parser>
	bool parse(std::string_view const& sv, Parser const& parser) noexcept {
		auto it = sv.begin();
		return parser.visit(it, sv.end());
	}
}
