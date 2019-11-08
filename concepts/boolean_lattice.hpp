//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_BOOLEAN_LATTICE_HPP
#define CXXMATH_CONCEPTS_BOOLEAN_LATTICE_HPP

#include "models/function_object.hpp"

namespace cxxmath
{
namespace concepts
{
template<class And, class Or, class Not>
struct boolean_lattice
{
	static constexpr auto and_ = function_object_v<And>;
	static constexpr auto or_ = function_object_v<Or>;
	static constexpr auto not_ = function_object_v<Not>;
};
}

template<class DispatchTag, class And, class Or, class Not>
struct models_concept<DispatchTag, concepts::boolean_lattice<And, Or, Not>>
{
	using boolean_lattice = concepts::boolean_lattice<And, Or, Not>;
	static constexpr bool value = ( boolean_lattice::and_.template supports_tag<DispatchTag>() &&
									boolean_lattice::or_.template supports_tag<DispatchTag>() &&
									boolean_lattice::not_.template supports_tag<DispatchTag>());
};

CXXMATH_DEFINE_CONCEPT( boolean_lattice )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( and_, boolean_lattice )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( or_, boolean_lattice )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( not_, boolean_lattice )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( &&, and_ )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( ||, or_ )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_UNARY_OPERATOR( !, not_ )
}

#endif //CXXMATH_CONCEPTS_BOOLEAN_ALGEBRA_HPP
