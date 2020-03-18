/** \defgroup concepts Concepts
 * \brief These are the mathematical concepts that are modelled by cxxmath
 * \{
 *
 * \file concepts/boolean_lattice.hpp
 * \brief The abstract concept of a boolean lattice over a set \f$ B \f$, as defined by
 * - \f$ and_ : B \times B \to B \f$
 * - \f$ or_ : B \times B \to B \f$
 * - \f$ not_ : B \to B \f$
 * where \f$ and_ \f$ and \f$ or_ \f$ are associative, commutative, distribute over
 * each other and are connected by the absorption law. Furthermore, we require \f$ B \f$
 * to contain two distinguished elements \f$ false_ \f$ and \f$ true_ \f$.
 */

#ifndef CXXMATH_CONCEPTS_BOOLEAN_LATTICE_HPP
#define CXXMATH_CONCEPTS_BOOLEAN_LATTICE_HPP

#include "../models/function_object.hpp"

namespace cxxmath {
namespace concepts {
template<class And, class Or, class Not> struct boolean_lattice {
	static constexpr auto and_ = function_object_v<And>;
	static constexpr auto or_ = function_object_v<Or>;
	static constexpr auto not_ = function_object_v<Not>;
};

template<class> struct is_boolean_lattice: std::false_type {};
template<class And, class Or, class Not> struct is_boolean_lattice<boolean_lattice<And, Or, Not>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( is_boolean_lattice )
}

template<class Type, class BooleanLattice>
struct type_models_concept<
	Type,
	BooleanLattice,
	std::enable_if_t<concepts::is_boolean_lattice_v<BooleanLattice>>
> {
	static constexpr bool value = (
		CXXMATH_IS_VALID( BooleanLattice::and_, std::declval<Type>(), std::declval<Type>() ) &&
		CXXMATH_IS_VALID( BooleanLattice::or_, std::declval<Type>(), std::declval<Type>() ) &&
		CXXMATH_IS_VALID( BooleanLattice::not_, std::declval<Type>() )
	);
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

/** \} */