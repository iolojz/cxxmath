//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CORE_FUNCTION_DISPATCH_HPP
#define CXXMATH_CORE_FUNCTION_DISPATCH_HPP

#include "dispatch_tags.hpp"

namespace cxxmath
{
namespace detail
{
template<class T>
struct always_false
{
	static constexpr bool value = false;
};
template<class T> static constexpr bool always_false_v = always_false<T>::value;
}

#ifdef CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION
#error "CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION is already defined."
#endif
#define CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( function, concept ) \
struct default_ ## function ## _dispatch { \
    template<class DispatchTag> static constexpr bool supports_tag( void ) { \
    	if constexpr( has_default_ ## concept<DispatchTag> ) \
        	return decltype(default_ ## concept ##_t<DispatchTag>())::function.template supports_tag<DispatchTag>(); \
        else \
        	return false; \
    }\
    \
    template<class ...Args> static constexpr decltype(auto) apply( Args &&... args ) { \
        using dispatch_tag = common_tag_t<tag_of_t<Args>...>; \
        static_assert( supports_tag<dispatch_tag>() ); \
        \
        return decltype(default_ ## concept ##_t<dispatch_tag>())::function( std::forward<Args>( args )... ); \
    } \
}; \
static constexpr auto function = function_object_v<default_ ## function ## _dispatch>;

template<class ...Args, class F>
static constexpr bool is_default_dispatchable( F f )
{
	if constexpr( have_common_tag_v<tag_of_t<Args>...>  )
		return f.template supports_tag<common_tag_t<tag_of_t<Args>...>>();
	else
		return false;
}

#ifdef CXXMATH_DEFINE_DEFAULT_DISPATCHED_UNARY_OPERATOR
#error "CXXMATH_DEFINE_DEFAULT_DISPATCHED_UNARY_OPERATOR is already defined"
#endif
#define CXXMATH_DEFINE_DEFAULT_DISPATCHED_UNARY_OPERATOR( op, function ) \
template<class Arg, \
class = std::enable_if_t<is_default_dispatchable<tag_of_t<Arg>>(::cxxmath::function )> \
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
