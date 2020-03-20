//
// Created by jayz on 09.09.19.
//

#ifndef CXXMATH_MODELS_BOOL_HPP
#define CXXMATH_MODELS_BOOL_HPP

#include "concepts/comparable.hpp"

namespace cxxmath {
namespace bool_model {
struct and_ {
	static constexpr bool apply( bool b1, bool b2 ) noexcept {
		return b1 && b2;
	}
};

struct or_ {
	static constexpr bool apply( bool b1, bool b2 ) noexcept {
		return b1 || b2;
	}
};

struct not_ {
	static constexpr bool apply( bool b ) noexcept {
		return !b;
	}
};

struct if_ {
	template<class Then, class Else>
	static constexpr auto apply( bool b, Then &&then, Else &&else_ ) {
		return b ? then : else_;
	}
};

struct equal {
	static constexpr bool apply( bool b1, bool b2 ) noexcept {
		return b1 == b2;
	}
};
}

namespace impl {
template<>
struct default_boolean_lattice<bool> {
	using type = concepts::boolean_lattice<bool_model::and_, bool_model::or_, bool_model::not_>;
};

template<>
struct default_logical<bool> {
	using type = concepts::logical<default_boolean_lattice_t < bool>, bool_model::if_>;
};

template<>
struct default_comparable<bool> {
	using type = concepts::comparable<bool_model::equal>;
};
}

struct std_bool_constant_tag {
};

namespace impl {
template<>
struct tag_of<std::true_type> {
	using type = std_bool_constant_tag;
};
template<>
struct tag_of<std::false_type> {
	using type = std_bool_constant_tag;
};
}

namespace model_std_bool_constant {
struct and_ {
	template<bool b1, bool b2>
	static constexpr std::bool_constant<b1 && b2>
	apply( std::bool_constant<b1>, std::bool_constant<b2> ) noexcept { return {}; }
};

struct or_ {
	template<bool b1, bool b2>
	static constexpr std::bool_constant<b1 || b2>
	apply( std::bool_constant<b1>, std::bool_constant<b2> ) noexcept { return {}; }
};

struct not_ {
	template<bool b>
	static constexpr std::bool_constant<!b> apply( std::bool_constant<b> ) noexcept { return {}; }
};

struct if_ {
	template<class Then, class Else>
	static constexpr auto apply( std::true_type, Then &&then, Else && ) {
		return then;
	}
	
	template<class Then, class Else>
	static constexpr auto apply( std::false_type, Then &&, Else &&else_ ) {
		return else_;
	}
};

struct equal {
	template<bool b1, bool b2>
	static constexpr std::bool_constant<b1 == b2>
	apply( std::bool_constant<b1>, std::bool_constant<b2> ) noexcept { return {}; }
};
}

namespace impl {
template<>
struct default_boolean_lattice<std_bool_constant_tag> {
	using type = concepts::boolean_lattice<
		model_std_bool_constant::and_, model_std_bool_constant::or_, model_std_bool_constant::not_
	>;
};

template<>
struct default_logical<std_bool_constant_tag> {
	using type = concepts::logical<default_boolean_lattice_t<std_bool_constant_tag>, model_std_bool_constant::if_>;
};

template<>
struct default_comparable<std_bool_constant_tag> {
	using type = concepts::comparable<model_std_bool_constant::equal>;
};
}
}

#endif //CXXMATH_MODELS_BOOL_HPP
