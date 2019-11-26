//
// Created by jayz on 09.09.19.
//

#ifndef CXXMATH_MODELS_BOOL_HPP
#define CXXMATH_MODELS_BOOL_HPP

#include "../concepts/set.hpp"

namespace cxxmath
{
namespace bool_model
{
struct and_ : supports_tag_helper<bool>
{
	static constexpr bool apply( bool b1, bool b2 ) noexcept
	{
		return b1 && b2;
	}
};

struct or_ : supports_tag_helper<bool>
{
	static constexpr bool apply( bool b1, bool b2 ) noexcept
	{
		return b1 || b2;
	}
};

struct not_ : supports_tag_helper<bool>
{
	static constexpr bool apply( bool b ) noexcept
	{
		return !b;
	}
};

struct if_
{
	template<class Then, class Else>
	static constexpr auto apply( bool b, Then &&then, Else &&else_ )
	{
		return b ? then : else_;
	}
};

struct equal : supports_tag_helper<bool>
{
	static constexpr bool apply( bool b1, bool b2 ) noexcept
	{
		return b1 == b2;
	}
};
}

namespace impl {
template<>
struct default_boolean_lattice<bool>
{
	using type = concepts::boolean_lattice<bool_model::and_, bool_model::or_, bool_model::not_>;
};

template<>
struct default_logical<bool>
{
	using type = concepts::logical<default_boolean_lattice_t<bool>, bool_model::if_>;
};

template<>
struct default_set<bool>
{
	using type = concepts::set<bool_model::equal>;
};
}

struct std_bool_constant_tag
{
};

namespace impl
{
template<>
struct tag_of<std::true_type>
{
	using type = std_bool_constant_tag;
};
template<>
struct tag_of<std::false_type>
{
	using type = std_bool_constant_tag;
};
}

namespace model_std_bool_constant
{
struct and_ : supports_tag_helper<std_bool_constant_tag>
{
	template<bool b1, bool b2>
	static constexpr std::bool_constant<b1 && b2> apply( std::bool_constant<b1>, std::bool_constant<b2> ) noexcept
	{ return {}; }
};

struct or_ : supports_tag_helper<std_bool_constant_tag>
{
	template<bool b1, bool b2>
	static constexpr std::bool_constant<b1 || b2> apply( std::bool_constant<b1>, std::bool_constant<b2> ) noexcept
	{ return {}; }
};

struct not_ : supports_tag_helper<std_bool_constant_tag>
{
	template<bool b>
	static constexpr std::bool_constant<!b> apply( std::bool_constant<b> ) noexcept
	{ return {}; }
};

struct if_
{
	template<class Then, class Else>
	static constexpr auto apply( std::true_type, Then &&then, Else && )
	{
		return then;
	}
	
	template<class Then, class Else>
	static constexpr auto apply( std::false_type, Then &&, Else &&else_ )
	{
		return else_;
	}
};

struct equal : supports_tag_helper<std_bool_constant_tag>
{
	template<bool b1, bool b2>
	static constexpr std::bool_constant<b1 == b2> apply( std::bool_constant<b1>, std::bool_constant<b2> ) noexcept
	{ return {}; }
};
}

namespace impl {
template<>
struct default_boolean_lattice<std_bool_constant_tag>
{
	using type = concepts::boolean_lattice<model_std_bool_constant::and_, model_std_bool_constant::or_, model_std_bool_constant::not_>;
};

template<>
struct default_logical<std_bool_constant_tag>
{
	using type = concepts::logical<default_boolean_lattice_t<std_bool_constant_tag>, model_std_bool_constant::if_>;
};

template<>
struct default_set<std_bool_constant_tag>
{
	using type = concepts::set<model_std_bool_constant::equal>;
};
}
}

#endif //CXXMATH_MODELS_BOOL_HPP
