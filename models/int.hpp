//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_MODELS_INT_HPP
#define CXXMATH_MODELS_INT_HPP

#include "../concepts/set.hpp"

namespace cxxmath
{
namespace model_int
{
struct is_abelian_ring
{
	static constexpr std::true_type apply( void )
	{
		return {};
	}
};

struct zero
{
	static constexpr int apply( void )
	{
		return 0;
	}
};

struct one
{
	static constexpr int apply( void )
	{
		return 1;
	}
};

struct add_assign : supports_tag_helper<int>
{
	static constexpr int &apply( int &a, int b )
	{
		return a += b;
	}
};

struct negate_in_place : supports_tag_helper<int>
{
	static constexpr int &apply( int &a )
	{
		return a = -a;
	}
};

struct multiply_assign : supports_tag_helper<int>
{
	static constexpr int &apply( int &a, int b )
	{
		return a *= b;
	}
};

struct equal : supports_tag_helper<int>
{
	static constexpr bool apply( int a, int b )
	{
		return a == b;
	}
};

struct less : supports_tag_helper<int>
{
	static constexpr bool apply( int a, int b )
	{
		return a < b;
	}
};
}

namespace impl {
template<>
struct default_monoid<int>
{
	using type = concepts::assignable_monoid<model_int::multiply_assign, model_int::one, model_int::is_abelian_ring>;
};

template<>
struct default_group<int>
{
	using type = concepts::assignable_group<concepts::assignable_monoid<model_int::add_assign, model_int::zero, model_int::is_abelian_ring>, model_int::negate_in_place>;
};

template<>
struct default_ring<int>
{
	using type = concepts::ring<default_group_t<int>, default_monoid_t<int>>;
};

template<>
struct default_set<int>
{
	using type = concepts::set<model_int::equal>;
};

template<>
struct default_total_order<int>
{
	using type = concepts::total_order<model_int::less>;
};
}
}

#endif //CXXMATH_MODELS_INT_HPP
