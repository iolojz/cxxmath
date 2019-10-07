//
// Created by jayz on 06.10.19.
//

#ifndef CXXMATH_DECOMPOSABLE_R_MODULE_HPP
#define CXXMATH_DECOMPOSABLE_R_MODULE_HPP

#include "r_module.hpp"

namespace cxxmath
{
namespace concepts
{
template<class RModule,>
struct decomposable_r_module
{
	using abelian_group = AbelianGroup;
	
	static constexpr auto zero = abelian_group::zero;
	static constexpr auto add = abelian_group::compose;
	static constexpr auto add_assign = abelian_group::compose_assign;
	static constexpr auto negate = abelian_group::inverse;
	static constexpr auto negate_in_place = abelian_group::invert_in_place;
	static constexpr auto scalar_multiply = function_object_v<ScalarMultiply>;
	static constexpr auto scalar_multiply_assign = function_object_v<ScalarMultiplyAssign>;
private:
	using add_impl = typename std::decay_t<decltype( add )>::implementation;
	using add_assign_impl = typename std::decay_t<decltype( add_assign )>::implementation;
	using negate_impl = typename std::decay_t<decltype( negate )>::implementation;
public:
	static constexpr auto subtract_assign = function_object_v<std::conditional_t < std::is_same_v <
															  add_assign_impl, impl::unsupported_implementation>, impl::unsupported_implementation, impl::binary_operator_invert_second<add_assign_impl, negate_impl> >>;
	static constexpr auto subtract = binary_operator_invert_second_v<add_impl, negate_impl>;
};
}

#endif //CXXMATH_DECOMPOSABLE_R_MODULE_HPP
