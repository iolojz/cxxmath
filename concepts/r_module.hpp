//
// Created by jayz on 01.10.19.
//

#ifndef CXXMATH_CONCEPTS_R_MODULE_HPP
#define CXXMATH_CONCEPTS_R_MODULE_HPP

#include "ring.hpp"

namespace cxxmath {
namespace concepts {
template<class AbelianGroup, class ScalarRing, class ScalarMultiplyInPlace, class ScalarMultiply>
struct r_module {
	static_assert( is_group_v<AbelianGroup>, "AbelianGroup parameter is not a group." );
	static_assert( AbelianGroup::is_abelian_group(), "AbelianGroup parameter is not abelian." );
	static_assert( is_ring_v<ScalarRing>, "ScalarRing parameter is not a ring." );
	
	using abelian_group = AbelianGroup;
	using scalar_ring = ScalarRing;
	
	static constexpr auto zero = abelian_group::neutral_element;
	static constexpr auto add = abelian_group::compose;
	static constexpr auto add_assign = abelian_group::compose_assign;
	static constexpr auto negate = abelian_group::inverse;
	static constexpr auto negate_in_place = abelian_group::invert_in_place;
	
	static constexpr auto scalar_multiply = function_object_v<ScalarMultiply>;
	static constexpr auto scalar_multiply_in_place = function_object_v<ScalarMultiplyInPlace>;
private:
	using add_impl = typename std::decay_t<decltype( add )>::implementation;
	using add_assign_impl = typename std::decay_t<decltype( add_assign )>::implementation;
	using negate_impl = typename std::decay_t<decltype( negate )>::implementation;
public:
	static constexpr auto subtract_assign = function_object_v<
		std::conditional_t<
			std::is_same_v<
				add_assign_impl, impl::unsupported_implementation
			>,
			impl::unsupported_implementation,
			impl::binary_operator_invert_second<add_assign_impl, negate_impl>
		>
	>;
	static constexpr auto subtract = binary_operator_invert_second_v<add_impl, negate_impl>;
};

namespace detail {
template<class ScalarMultiplyInPlace>
struct scalar_multiply {
	template<class Scalar, class Object>
	static constexpr decltype( auto ) apply( Scalar &&s, Object &&o ) {
		if constexpr( std::is_rvalue_reference_v<Object &&> ) {
			return ScalarMultiplyInPlace::apply( std::forward<Scalar>( s ), std::forward<Object>( o ) );
		} else {
			auto copy = std::decay_t<Object>{ std::forward<Object>( o ) };
			ScalarMultiplyInPlace::apply( std::forward<Scalar>( s ), copy );
			return copy;
		}
	}
};
}

template<class AbelianGroup, class ScalarMultiplyInPlace> using assignable_r_module = r_module<
	AbelianGroup, ScalarMultiplyInPlace, detail::scalar_multiply<ScalarMultiplyInPlace>>;
template<class AbelianGroup, class ScalarMultiply> using non_assignable_r_module = r_module<
	AbelianGroup, impl::unsupported_implementation, ScalarMultiply
>;

template<class> struct is_r_module: std::false_type {};
template<class AbelianGroup, class ScalarMultiplyInPlace, class ScalarMultiply>
struct is_r_module<r_module<AbelianGroup, ScalarMultiplyInPlace, ScalarMultiply>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( is_r_module )
}

template<class ScalarType, class ModuleType, class RModule>
struct type_models_concept<
	multitype<ScalarType, ModuleType>,
	RModule,
	std::enable_if_t<concepts::is_r_module_v<RModule>>
> {
private:
	static constexpr bool scalar_multiply_in_place_is_valid =
		std::is_same_v<typename RModule::scalar_multiply_in_place::implementation, impl::unsupported_implementation>
			? true : CXXMATH_IS_VALID( RModule::scalar_multiply_in_place,
				std::declval<ScalarType>(), std::declval<ModuleType>() );
public:
	static constexpr bool value = (
		type_models_concept_v<ModuleType, typename RModule::abelian_group> &&
		type_models_concept_v<ScalarType, typename RModule::scalar_ring> &&
		CXXMATH_IS_VALID( RModule::scalar_multiply, std::declval<ScalarType>(), std::declval<ModuleType>() )
	);
};

CXXMATH_DEFINE_CONCEPT( r_module )

struct dispatch_scalar_multiply {
	template<class Scalar, class Object>
	constexpr decltype( auto ) operator()( Scalar &&scalar, Object &&object ) const {
		using dispatch_tag = multitag<tag_of_t<Scalar>, tag_of_t<Object>>;
		return default_r_module_t<dispatch_tag>::scalar_multiply(
			std::forward<Scalar>( scalar ),
			std::forward<Object>( object )
		);
	}
};

struct dispatch_scalar_multiply_in_place {
	template<class Scalar, class Object>
	constexpr decltype( auto ) operator()( Scalar &&scalar, Object &&object ) const {
		using dispatch_tag = multitag<tag_of_t<Scalar>, tag_of_t<Object>>;
		return default_r_module_t<dispatch_tag>::scalar_multiply_in_place(
			std::forward<Scalar>( scalar ),
			std::forward<Object>( object )
		);
	}
};

static constexpr dispatch_scalar_multiply scalar_multiply;
static constexpr dispatch_scalar_multiply_in_place scalar_multiply_in_place;
}

#endif //CXXMATH_CONCEPTS_R_MODULE_HPP
