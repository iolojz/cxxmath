//
// Created by jayz on 01.10.19.
//

#ifndef CXXMATH_CONCEPTS_R_ALGEBRA_HPP
#define CXXMATH_CONCEPTS_R_ALGEBRA_HPP

#include "r_module.hpp"

namespace cxxmath {
namespace concepts {
template<class RModule, class Monoid>
struct r_algebra {
	static_assert( is_r_module_v<RModule>, "RModule parameter is not an RModule." );
	static_assert( is_monoid_v<Monoid>, "Monoid parameter is not an Monoid." );
	
	using r_module_ = RModule;
	using monoid_ = Monoid;
	
	static constexpr auto is_abelian_r_algebra = monoid_::is_abelian_monoid;
	static constexpr auto zero = r_module_::zero;
	static constexpr auto add = r_module_::add;
	static constexpr auto add_assign = r_module_::add_assign;
	static constexpr auto subtract = r_module_::subtract;
	static constexpr auto subtract_assign = r_module_::subtract_assign;
	static constexpr auto negate = r_module_::negate;
	static constexpr auto negate_in_place = r_module_::negate_in_place;
	static constexpr auto scalar_multiply = r_module_::scalar_multiply;
	static constexpr auto scalar_multiply_in_place = r_module_::scalar_multiply_in_place;
	static constexpr auto multiply = monoid_::compose;
	static constexpr auto multiply_assign = monoid_::compose_assign;
	static constexpr auto one = monoid_::neutral_element;
};

template<class> struct is_r_algebra: std::false_type {};
template<class RModule, class Monoid> struct is_r_algebra<r_algebra<RModule, Monoid>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( is_r_algebra )
}

template<class Type, class RAlgebra>
struct type_models_concept<Type, RAlgebra, std::enable_if_t<concepts::is_r_algebra_v<RAlgebra>>> {
	static constexpr bool value = (
		type_models_concept<Type, typename RAlgebra::r_module_>::value &&
		type_models_concept<Type, typename RAlgebra::monoid_>::value
	);
};

CXXMATH_DEFINE_CONCEPT( r_algebra )
}

#endif //CXXMATH_CONCEPTS_R_ALGEBRA_HPP
