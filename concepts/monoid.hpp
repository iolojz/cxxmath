//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_MONOID_HPP
#define CXXMATH_CONCEPTS_MONOID_HPP

#include "monoid_fwd.hpp"
#include "../models/function_object.hpp"

namespace cxxmath {
namespace concepts {
template<class ComposeAssign, class Compose, class NeutralElement, class IsAbelian>
struct monoid {
	static constexpr auto compose_assign = static_function_object<ComposeAssign>;
	static constexpr auto compose = static_function_object<Compose>;
	static constexpr auto neutral_element = static_function_object<NeutralElement>;
	static constexpr auto is_abelian_monoid = static_function_object<IsAbelian>;
};

template<class ComposeAssign, class Compose, class NeutralElement, class IsAbelian>
struct is_monoid<monoid<ComposeAssign, Compose, NeutralElement, IsAbelian>>: std::true_type {};
}

template<class Type, class Monoid>
struct type_models_concept<Type, Monoid, std::enable_if_t<concepts::is_monoid_v<Monoid>>> {
private:
	static constexpr bool compose_assign_valid =
		std::is_same_v<typename Monoid::compose_assign::implementation, impl::unsupported_implementation>
			? true : std::is_invocable_v<decltype(Monoid::compose_assign), Type, Type>;
public:
	static constexpr bool value = (
		compose_assign_valid &&
		std::is_invocable_v<decltype(Monoid::compose), Type, Type> &&
		std::is_invocable_v<decltype(Monoid::neutral_element)> &&
		std::is_invocable_v<decltype(Monoid::is_abelian_monoid)>
	);
};

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( compose, monoid )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( compose_assign, monoid )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( is_abelian_monoid, monoid )
}

#endif //CXXMATH_CONCEPTS_MONOID_HPP
