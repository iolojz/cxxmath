//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_MONOID_HPP
#define CXXMATH_CONCEPTS_MONOID_HPP

#include "monoid_fwd.hpp"
#include "../models/function_object.hpp"

namespace cxxmath
{
namespace concepts
{
template<class ComposeAssign, class Compose, class NeutralElement, class IsAbelian>
struct monoid
{
	static constexpr auto compose_assign = function_object_v<ComposeAssign>;
	static constexpr auto compose = function_object_v<Compose>;
	static constexpr auto neutral_element = function_object_v<NeutralElement>;
	static constexpr auto is_abelian_monoid = function_object_v<IsAbelian>;
};

template<class ComposeAssign, class Compose, class NeutralElement, class IsAbelian>
struct is_monoid<monoid<ComposeAssign, Compose, NeutralElement, IsAbelian>> : std::true_type {};
}

template<class DispatchTag, class Monoid>
struct models_concept<DispatchTag, Monoid, std::enable_if_t<concepts::is_monoid_v<Monoid>>>
{
private:
	static constexpr bool compose_assign_valid =
	std::is_same_v < typename Monoid::compose_assign::implementation, impl::unsupported_implementation>
	? true : Monoid::compose_assign.template supports_tag<DispatchTag>;
public:
	static constexpr bool value = (
		compose_assign_valid &&
		Monoid::compose.template supports_tag<DispatchTag>() &&
		Monoid::neutral_element.template supports_tag<DispatchTag>() &&
		Monoid::is_abelian_monoid.template supports_tag<DispatchTag>()
	);
};

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( compose, monoid )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( compose_assign, monoid )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( is_abelian_monoid, monoid )
}

#endif //CXXMATH_CONCEPTS_MONOID_HPP
