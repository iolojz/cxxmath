//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_SET_HPP
#define CXXMATH_CONCEPTS_SET_HPP

#include "logical.hpp"
#include "monoid.hpp"

namespace cxxmath
{
namespace concepts
{
template<class Equal>
struct set
{
	static constexpr auto equal = function_object_v<Equal>;
	static constexpr auto not_equal = compose( not_, equal );
};

template<class> struct is_set : std::false_type {};
template<class Equal> struct is_set<set<Equal>> : std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_set)
}

template<class DispatchTag, class Set>
struct models_concept<DispatchTag, Set, std::enable_if_t<concepts::is_set_v<Set>>>
{
	static constexpr bool value = Set::equal.template supports_tag<DispatchTag>();
};

CXXMATH_DEFINE_CONCEPT( set )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( equal, set )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( not_equal, set )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( ==, equal )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( !=, not_equal )
}

#endif //CXXMATH_CONCEPTS_SET_HPP
