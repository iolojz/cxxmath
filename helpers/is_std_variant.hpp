//
// Created by jayz on 13.11.19.
//

#ifndef CXXMATH_HELPERS_IS_VARIANT_HPP
#define CXXMATH_HELPERS_IS_VARIANT_HPP

#include <variant>

#include "wrap_template_members.hpp"

namespace cxxmath {
template<class T> struct is_std_variant : std::false_type {};
template<class ...Alternatives> struct is_std_variant<std::variant<Alternatives...>> : std::true_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_std_variant)
}

#endif //CXXMATH_HELPERS_IS_VARIANT_HPP
