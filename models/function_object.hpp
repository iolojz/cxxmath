//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_MODELS_FUNCTION_OBJECT_HPP
#define CXXMATH_MODELS_FUNCTION_OBJECT_HPP

#include "../concepts/monoid_fwd.hpp"

#include "../core/concepts.hpp"
#include "../core/dispatch_tags.hpp"

#include "../helpers/boolean_v.hpp"

namespace cxxmath {
struct function_object_tag {};

template<class Implementation>
struct function_object {
	using cxxmath_dispatch_tag = function_object_tag;
	using implementation = Implementation;
	
	template<class ...Args>
	constexpr auto operator()( Args &&...args ) const
	/* Make us SFINAE-friendly */ -> decltype(implementation::apply( std::forward<Args>( args )... )) {
		return implementation::apply( std::forward<Args>( args )... );
	}
};

template<class Implementation> static constexpr auto function_object_v = function_object<Implementation>{};

namespace impl {
struct identity {
	template<class Arg>
	static constexpr auto apply( Arg &&arg )
	/* Make us SFINAE-friendly */ -> decltype( arg ) {
		return arg;
	}
};

struct unsupported_implementation {
	template<class ...Args, class False = void>
	static constexpr void apply( Args &&... ) {
		static_assert( false_v<False>, "Unsupported implementation." );
	}
};

struct true_implementation {
	template<class ...Args>
	static constexpr bool apply( Args &&... ) { return true; }
};

struct false_implementation {
	template<class ...Args>
	static constexpr bool apply( Args &&... ) { return false; }
};
}

static constexpr auto identity = function_object_v<impl::identity>;

namespace model_function_object {
template<class Impl1, class Impl2>
struct composed_function_object {
	template<class ...Args>
	static constexpr decltype( auto ) apply( Args &&...args ) {
		return Impl1::apply( Impl2::apply( std::forward<Args>( args )... ) );
	}
};

struct compose {
	template<class F1, class F2>
	static constexpr auto apply( F1, F2 ) noexcept {
		using i1 = typename F1::implementation;
		using i2 = typename F2::implementation;
		
		return function_object_v<composed_function_object<i1, i2>>;
	}
};

struct is_abelian {
	static constexpr std::false_type apply( void ) noexcept { return {}; }
};

struct neutral_element {
	static constexpr auto apply( void ) noexcept { return cxxmath::identity; }
};
}

namespace impl {
template<>
struct default_monoid<function_object_tag> {
	using type = concepts::non_assignable_monoid<
		model_function_object::compose, model_function_object::is_abelian, model_function_object::neutral_element
	>;
};
}
}

#endif //CXXMATH_MODELS_FUNCTION_OBJECT_HPP
