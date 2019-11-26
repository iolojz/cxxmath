//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_MODELS_STD_VECTOR_HPP
#define CXXMATH_MODELS_STD_VECTOR_HPP

#include "../concepts/monoid.hpp"

#include <vector>

namespace cxxmath
{
template<class Symbol, class Allocator = std::allocator<Symbol>>
struct std_vector_tag;

namespace impl
{
template<class Symbol, class Allocator>
struct tag_of<std::vector<Symbol, Allocator>>
{
	using type = std_vector_tag<Symbol, Allocator>;
};
}

namespace model_std_vector
{
template<class Symbol, class Allocator>
struct is_abelian : supports_tag_helper<std_vector_tag<Symbol, Allocator>>
{
	static constexpr std::bool_constant<std::is_empty_v<Symbol>> apply( void )
	{
		return {};
	}
};

template<class Symbol, class Allocator>
struct neutral_element : supports_tag_helper<std_vector_tag<Symbol, Allocator>>
{
	static std::vector<Symbol, Allocator> apply( void )
	{
		return {};
	}
};

template<class Symbol, class Allocator>
struct compose_assign : supports_tag_helper<std_vector_tag<Symbol, Allocator>>
{
	static constexpr std::vector<Symbol, Allocator> &
	apply( std::vector<Symbol, Allocator> &v1, const std::vector<Symbol, Allocator> &v2 )
	{
		v1.insert( v1.end(), v2.begin(), v2.end());
		return v1;
	}
	
	static constexpr std::vector<Symbol, Allocator> &
	apply( std::vector<Symbol, Allocator> &v1, std::vector<Symbol, Allocator> &&v2 )
	{
		v1.insert( v1.end(), std::make_move_iterator( v2.begin()), std::make_move_iterator( v2.end()));
		return v1;
	}
};

template<class Symbol, class Allocator>
struct equal : supports_tag_helper<std_vector_tag<Symbol, Allocator>>
{
	static constexpr bool apply( const std::vector<Symbol, Allocator> &v1, const std::vector<Symbol, Allocator> &v2 )
	{
		return v1 == v2;
	}
};

template<class Symbol, class Allocator = std::allocator<Symbol>>
struct vector_monoid
{
private:
	using tag = std_vector_tag<Symbol, Allocator>;
	using compose_assign_impl = compose_assign<Symbol, Allocator>;
	using neutral_element_impl = neutral_element<Symbol, Allocator>;
	using is_abelian_impl = is_abelian<Symbol, Allocator>;
public:
	using type = concepts::assignable_monoid<compose_assign_impl, neutral_element_impl, is_abelian_impl>;
};
}

namespace impl
{
template<class Symbol, class Allocator>
struct default_monoid<std_vector_tag<Symbol, Allocator>>
{
	using type = typename model_std_vector::vector_monoid<Symbol, Allocator>::type;
};

template<class Symbol, class Allocator>
struct default_set<std_vector_tag<Symbol, Allocator>>
{
	using type = concepts::set<model_std_vector::equal<Symbol, Allocator>>;
};

template<class Symbol, class Allocator>
struct make<std_vector_tag<Symbol, Allocator>> {
	template<class ...Args> static std::vector<Symbol, Allocator> apply( Args &&...args ) {
		return { std::forward<Args>( args )... };
	}
};
}
}

#endif //CXXMATH_MODELS_STD_VECTOR_HPP
