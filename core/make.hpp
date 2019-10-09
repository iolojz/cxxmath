//
// Created by jayz on 08.10.19.
//

#ifndef CXXMATH_CORE_MAKE_HPP
#define CXXMATH_CORE_MAKE_HPP

#include "models/function_object.hpp"

namespace cxxmath {
namespace impl {
template<class Tag> struct make : unsupported_implementation {};
}

template<class Tag> static constexpr auto make = function_object_v<impl::make<Tag>>;
}

#endif //CXXMATH_CORE_MAKE_HPP
