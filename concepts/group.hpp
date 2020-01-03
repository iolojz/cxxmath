//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_GROUP_HPP
#define CXXMATH_CONCEPTS_GROUP_HPP

#include "../models/function_object.hpp"
#include "../core/operator_helpers.hpp"

namespace cxxmath
{
namespace concepts
{
template<class Monoid, class InvertInPlace, class Inverse>
struct group
{
	static_assert( is_monoid_v<Monoid>, "Monoid parameter is not a Monoid." );
	using monoid_ = Monoid;
	
	static constexpr auto compose = monoid_::compose;
	static constexpr auto compose_assign = monoid_::compose_assign;
	static constexpr auto is_abelian_group = monoid_::is_abelian_monoid;
	static constexpr auto neutral_element = monoid_::neutral_element;
	static constexpr auto invert_in_place = function_object_v<InvertInPlace>;
	static constexpr auto inverse = function_object_v<Inverse>;
};

template<class Monoid, class InvertInPlace> using assignable_group = group<Monoid, InvertInPlace, impl::unary_operator<InvertInPlace>>;

template<class Monoid, class Inverse> using non_assignable_group = group<Monoid, impl::unsupported_implementation, Inverse>;

template<class> struct is_group : std::false_type {};
template<class Monoid, class InvertInPlace, class Inverse> struct is_group<group<Monoid, InvertInPlace, Inverse>>
: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_group)
}

template<class DispatchTag, class Group>
struct models_concept<DispatchTag, Group, std::enable_if_t<concepts::is_group_v<Group>>>
{
private:
	static constexpr bool invert_in_place_valid =
	std::is_same_v < typename Group::invert_in_place::implementation, impl::unsupported_implementation>
	? true : Group::invert_in_place.template supports_tag<DispatchTag>;
public:
	static constexpr bool value = (
		models_concept_v<DispatchTag, typename Group::monoid_> &&
		invert_in_place_valid &&
		Group::inverse.template supports_tag<DispatchTag>()
	);
};

CXXMATH_DEFINE_CONCEPT( group )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( inverse, group )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( is_abelian_group, group )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( invert_in_place, group )
}

#endif //CXXMATH_CONCEPTS_GROUP_HPP
