//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_RING_HPP
#define CXXMATH_CONCEPTS_RING_HPP

#include "../models/function_object.hpp"

namespace cxxmath {
namespace concepts {
template<class AbelianGroup, class Monoid>
struct ring {
	static_assert( is_group_v < AbelianGroup > , "AbelianGroup parameter is not a Group." );
	static_assert( AbelianGroup::is_abelian_group(), "AbelianGroup parameter is not abelian." );
	static_assert( is_monoid_v < Monoid > , "Monoid parameter is not a Monoid." );
	
	using abelian_group = AbelianGroup;
	using monoid_ = Monoid;
	
	static constexpr auto zero = abelian_group::neutral_element;
	static constexpr auto add = abelian_group::compose;
	static constexpr auto add_assign = abelian_group::compose_assign;
	static constexpr auto negate = abelian_group::inverse;
	static constexpr auto negate_in_place = abelian_group::invert_in_place;
	static constexpr auto is_abelian_ring = monoid_::is_abelian_monoid;
	static constexpr auto one = monoid_::neutral_element;
	static constexpr auto multiply = monoid_::compose;
	static constexpr auto multiply_assign = monoid_::compose_assign;
private:
	using add_impl = typename std::decay_t<decltype( add )>::implementation;
	using add_assign_impl = typename std::decay_t<decltype( add_assign )>::implementation;
	using negate_impl = typename std::decay_t<decltype( negate )>::implementation;
public:
	static constexpr auto subtract_assign = function_object_v < std::conditional_t<
		std::is_same_v<
			add_assign_impl, impl::unsupported_implementation
		>, impl::unsupported_implementation, impl::binary_operator_invert_second < add_assign_impl, negate_impl
	> >>;
	static constexpr auto subtract = binary_operator_invert_second_v<add_impl, negate_impl>;
};

template<class> struct is_ring: std::false_type {};
template<class AbelianGroup, class Monoid> struct is_ring<ring<AbelianGroup, Monoid>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( is_ring )
}

template<class Type, class Ring>
struct type_models_concept<Type, Ring, std::enable_if_t<concepts::is_ring_v<Ring>>> {
	static constexpr bool value = (
		type_models_concept_v<Type, typename Ring::abelian_group> &&
		type_models_concept_v<Type, typename Ring::monoid_>
	);
};

CXXMATH_DEFINE_CONCEPT( ring )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( add, ring )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( add_assign, ring )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( negate, ring )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( negate_in_place, ring )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( subtract, ring )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( subtract_assign, ring )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( multiply, ring )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( multiply_assign, ring )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( *, multiply )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( *=, multiply_assign )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( +, add )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( +=, add_assign )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( -, subtract )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( -=, subtract_assign )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_UNARY_OPERATOR( -, negate )
}

#endif //CXXMATH_CONCEPTS_RING_HPP
