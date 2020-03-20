//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_COMPARABLE_HPP
#define CXXMATH_CONCEPTS_COMPARABLE_HPP

#include "logical.hpp"
#include "monoid.hpp"

namespace cxxmath {
namespace concepts {
template<class Equal>
struct comparable {
	static constexpr auto equal = function_object_v<Equal>;
	static constexpr auto not_equal = compose( not_, equal );
};

template<class> struct is_comparable: std::false_type {};
template<class Equal> struct is_comparable<comparable<Equal>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( is_comparable )
}

template<class Type, class Comparable>
struct type_models_concept<Type, Comparable, std::enable_if_t<concepts::is_comparable_v<Comparable>>> {
	static constexpr bool value = (
		std::is_invocable_v<decltype(Comparable::equal), Type, Type> &&
		std::is_invocable_v<decltype(Comparable::not_equal), Type, Type>
	);
};

CXXMATH_DEFINE_CONCEPT( comparable )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( equal, comparable )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( not_equal, comparable )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( ==, equal )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( !=, not_equal )
}

#endif //CXXMATH_CONCEPTS_COMPARABLE_HPP
