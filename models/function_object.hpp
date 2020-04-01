//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_MODELS_FUNCTION_OBJECT_HPP
#define CXXMATH_MODELS_FUNCTION_OBJECT_HPP

#include "../concepts/monoid_fwd.hpp"

#include "../core/concepts.hpp"
#include "../core/dispatch_tags.hpp"

#include "../helpers/boolean_v.hpp"

#include <boost/hana/not.hpp>

namespace cxxmath {
struct function_object_tag {};

namespace detail {
template<class Implementation>
struct static_function_object {
	using cxxmath_dispatch_tag = function_object_tag;
	using implementation = Implementation;
	
	template<class ...Args>
	constexpr auto operator()( Args &&...args ) const
	/* Make us SFINAE-friendly */ -> decltype( implementation::apply( std::forward<Args>( args )... ) ) {
		return implementation::apply( std::forward<Args>( args )... );
	}
};

template<class Function> class stateful_function_object {
	Function function;
public:
	using cxxmath_dispatch_tag = function_object_tag;
	template<class F, class = std::enable_if_t<std::is_constructible_v<Function, F &&>>>
	constexpr stateful_function_object( F &&f ) : function{ std::forward<F>( f ) } {}
	
	constexpr stateful_function_object( stateful_function_object && ) = default;
	constexpr stateful_function_object( const stateful_function_object & ) = default;
	
	constexpr stateful_function_object &operator=( stateful_function_object && ) = default;
	constexpr stateful_function_object &operator=( const stateful_function_object & ) = default;
	
	template<class ...Args>
	constexpr auto operator()( Args &&...args ) const
	/* Make us SFINAE-friendly */ -> decltype( function( std::forward<Args>( args )... ) ) {
		return function( std::forward<Args>( args )... );
	}
};
}

template<class Implementation> using static_function_object_t = detail::static_function_object<Implementation>;
template<class Implementation> static constexpr auto static_function_object = detail::static_function_object<Implementation>{};

template<class Function>
static constexpr auto make_function_object( Function &&f ) {
	return detail::stateful_function_object<std::decay_t<Function>>( std::forward<Function>( f ) );
}

namespace impl {
struct identity {
	template<class Arg>
	static constexpr auto apply( Arg &&arg )
	/* Make us SFINAE-friendly */ -> decltype( arg ) {
		return arg;
	}
};

struct true_ {
	template<class ...Args, class False = void>
	static constexpr bool apply( Args &&... ) {
		return true;
	}
};

struct false_ {
	template<class ...Args, class False = void>
	static constexpr bool apply( Args &&... ) {
		return false;
	}
};

struct unsupported_implementation {
	template<class ...Args, class False = void>
	static constexpr void apply( Args &&... ) {
		static_assert( false_v<False>, "Unsupported implementation." );
	}
};
}

static constexpr auto identity = static_function_object<impl::identity>;
static constexpr auto lazy_true = static_function_object<impl::true_>;
static constexpr auto lazy_false = static_function_object<impl::false_>;

static constexpr auto and_ = make_function_object( boost::hana::and_ );
static constexpr auto or_ = make_function_object( boost::hana::not_ );
static constexpr auto not_ = make_function_object( boost::hana::not_ );

namespace model_function_object {
template<class FunctionObject1, class FunctionObject2>
struct composed_function_object {
	using cxxmath_dispatch_tag = function_object_tag;
	FunctionObject1 function1;
	FunctionObject2 function2;
	
	template<class F1, class F2>
	constexpr composed_function_object( F1 &&f1, F2 &&f2 )
	: function1{ std::forward<F1>( f1 ) }, function2{ std::forward<F2>( f2 ) } {}
	
	constexpr composed_function_object( composed_function_object && ) = default;
	constexpr composed_function_object( const composed_function_object & ) = default;
	
	constexpr composed_function_object &operator=( composed_function_object && ) = default;
	constexpr composed_function_object &operator=( const composed_function_object & ) = default;
	
	template<class ...Args>
	constexpr auto operator()( Args &&...args ) const
	/* Make us SFINAE-friendly */ -> decltype( function1( function2( std::forward<Args>( args )... ) ) ) {
		return function1( function2( std::forward<Args>( args )... ) );
	}
};

struct compose {
	template<class F1, class F2>
	static constexpr auto apply( F1 &&f1, F2 &&f2 ) noexcept {
		return composed_function_object<std::decay_t<F1>, std::decay_t<F2>>(
			std::forward<F1>( f1 ),
			std::forward<F2>( f2 )
		);
	}
};

struct is_abelian {
	static constexpr bool apply( void ) noexcept { return false; }
};

struct neutral_element {
	static constexpr auto apply( void ) noexcept { return cxxmath::identity; }
};
}

namespace impl {
template<>
struct default_monoid<function_object_tag> {
	using type = concepts::non_assignable_monoid<
		model_function_object::compose,
		model_function_object::is_abelian,
		model_function_object::neutral_element
	>;
};
}
}

#endif //CXXMATH_MODELS_FUNCTION_OBJECT_HPP
