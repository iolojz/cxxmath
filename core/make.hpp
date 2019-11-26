//
// Created by jayz on 08.10.19.
//

#ifndef CXXMATH_CORE_MAKE_HPP
#define CXXMATH_CORE_MAKE_HPP

#include "../models/function_object.hpp"

namespace cxxmath {
namespace impl {
template<class Tag, class = void> struct make : supports_tag_helper<Tag> {
	static constexpr Tag apply( Tag &&tag ) noexcept {
		return tag;
	}
	
	static constexpr Tag apply( const Tag &tag ) noexcept {
		return tag;
	}
};
}

template<class Tag, class Spec = void> static constexpr auto make = function_object_v<impl::make<Tag, Spec>>;
}

#endif //CXXMATH_CORE_MAKE_HPP
