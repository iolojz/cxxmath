//
// Created by jayz on 08.10.19.
//

#ifndef CXXMATH_CORE_MAKE_HPP
#define CXXMATH_CORE_MAKE_HPP

#include "../models/function_object.hpp"

namespace cxxmath {
namespace impl {
template<class DispatchTag, class = void> struct make : unsupported_implementation {};
}

template<class DispatchTag> static constexpr auto make = static_function_object<impl::make<DispatchTag>>;
}

#endif //CXXMATH_CORE_MAKE_HPP
