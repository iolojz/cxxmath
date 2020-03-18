//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CORE_FUNCTION_DISPATCH_HPP
#define CXXMATH_CORE_FUNCTION_DISPATCH_HPP

#include "dispatch_tags.hpp"

namespace cxxmath {
#ifdef CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION
#error "CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION is already defined."
#endif
#define CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( function, concept_ ) \
struct dispatch_ ## function { \
    template<class ...Args> static constexpr decltype(auto) apply( Args &&... args ) { \
        using dispatch_tag = common_tag_t<tag_of_t<Args>...>; \
        static_assert( has_default_ ## concept_ ## _v<dispatch_tag>, \
            "The common tag of the given arguments does not implement a default concept for" #concept_ ); \
        \
        return default_ ## concept_ ## _t<dispatch_tag>::function( std::forward<Args>( args )... ); \
    } \
}; \
static constexpr auto function = function_object_v<dispatch_ ## function>;

template<class ...Args, class F>
static constexpr bool is_default_dispatchable( F &&f ) {
	return have_common_tag_v<tag_of_t<Args>...> );
}

#ifdef CXXMATH_DEFINE_DEFAULT_DISPATCHED_UNARY_OPERATOR
#error "CXXMATH_DEFINE_DEFAULT_DISPATCHED_UNARY_OPERATOR is already defined"
#endif
#define CXXMATH_DEFINE_DEFAULT_DISPATCHED_UNARY_OPERATOR( op, function ) \
template<class Arg, \
class = std::enable_if_t<is_default_dispatchable<Arg>(::cxxmath::function )> \
> static constexpr decltype(auto) operator op( Arg &&arg ) { \
    return::cxxmath::function( std::forward<Arg>( arg ) ); \
}

#ifdef CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR
#error "CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR is already defined"
#endif
#define CXXMATH_DEFINE_DEFAULT_DISPATCHED_BINARY_OPERATOR( op, function ) \
template<class Arg1, class Arg2, \
class = std::enable_if_t<is_default_dispatchable<Arg1, Arg2>(::cxxmath::function )> \
> static constexpr decltype(auto) operator op( Arg1 &&arg1, Arg2 &&arg2 ) { \
    return::cxxmath::function( std::forward<Arg1>( arg1 ), std::forward<Arg2>( arg2 ) ); \
}
}

#endif //CXXMATH_CORE_FUNCTION_DISPATCH_HPP
