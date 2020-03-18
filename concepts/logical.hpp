//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_LOGICAL_HPP
#define CXXMATH_CONCEPTS_LOGICAL_HPP

#include "boolean_lattice.hpp"

namespace cxxmath {
namespace concepts {
template<class BooleanLattice, class If>
struct logical {
	static_assert( is_boolean_lattice_v<BooleanLattice> , "BooleanLattice parameter is not a BooleanLattice." );
	using boolean_lattice_ = BooleanLattice;
	
	static constexpr auto and_ = boolean_lattice_::and_;
	static constexpr auto or_ = boolean_lattice_::or_;
	static constexpr auto not_ = boolean_lattice_::not_;
	
	static constexpr auto if_ = function_object_v<If>;
};

template<class> struct is_logical: std::false_type {};
template<class BooleanLattice, class If> struct is_logical<logical<BooleanLattice, If>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( is_logical )
}

namespace impl {
template<class DispatchTag> struct make_true : unsupported_implementation;
template<class DispatchTag> struct make_false : unsupported_implementation;
}

template<class DispatchTag> static constexpr auto make_true = function_object_v<impl::make_true<DispatchTag>>;
template<class DispatchTag> static constexpr auto make_false = function_object_v<impl::make_false<DispatchTag>>;

template<class Type, class Logical>
struct type_models_concept<Type, Logical, std::enable_if_t<concepts::is_logical_v<Logical>>> {
	using boolean_lattice = typename Logical::boolean_lattice;
	static constexpr bool value = (
		type_models_concept_v<Type, boolean_lattice> &&
		Logical::if_( make_true<tag_of_t<Type>>(), 42, 23 ) == 42 &&
		Logical::if_( make_false<tag_of_t<Type>>(), 42, 23 ) == 23
	);
};

CXXMATH_DEFINE_CONCEPT( logical )

struct dispatch_if_ {
	template<class Cond, class Then, class Else>
	constexpr decltype( auto ) operator()( Cond &&cond, Then &&then, Else &&else_ ) const {
		using dispatch_tag = tag_of_t<Cond>;
		return default_logical_t<dispatch_tag>::if_(
			std::forward<Cond>( cond ),
			std::forward<Then>( then ),
			std::forward<Else>( else_ )
		);
	}
};
static constexpr dispatch_if_ if_;
}

#endif //CXXMATH_CONCEPTS_LOGICAL_HPP
