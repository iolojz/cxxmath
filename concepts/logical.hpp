//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_LOGICAL_HPP
#define CXXMATH_CONCEPTS_LOGICAL_HPP

#include "boolean_lattice.hpp"

namespace cxxmath
{
namespace concepts
{
template<class BooleanLattice, class If>
struct logical
{
	static_assert( is_boolean_lattice_v<BooleanLattice>, "BooleanLattice parameter is not a BooleanLattice." );
	using boolean_lattice_ = BooleanLattice;
	
	static constexpr auto and_ = boolean_lattice_::and_;
	static constexpr auto or_ = boolean_lattice_::or_;
	static constexpr auto not_ = boolean_lattice_::not_;
	
	static constexpr auto if_ = function_object_v<If>;
};

template<class> struct is_logical : std::false_type {};
template<class BooleanLattice, class If> struct is_logical<logical<BooleanLattice, If>> : std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_logical)
}

template<class DispatchTag, class Logical>
struct models_concept<DispatchTag, Logical, std::enable_if_t<concepts::is_logical_v<Logical>>>
{
	using boolean_lattice = typename Logical::boolean_lattice;
	static constexpr bool value = (
		models_concept_v<DispatchTag, boolean_lattice> &&
		Logical::if_.template supports_tag<DispatchTag>()
	);
};

CXXMATH_DEFINE_CONCEPT( logical )

struct default_if__dispatch
{
	template<class Cond, class ...Args>
	constexpr decltype( auto ) operator()( Cond &&cond, Args &&... args ) const
	{
		using dispatch_tag = tag_of_t<Cond>;
		return default_logical_t<dispatch_tag>::if_( std::forward<Cond>( cond ), std::forward<Args>( args )... );
	}
};
static constexpr default_if__dispatch if_;
}

#endif //CXXMATH_CONCEPTS_LOGICAL_HPP
