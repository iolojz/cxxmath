//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CONCEPTS_MONOID_FWD_HPP
#define CXXMATH_CONCEPTS_MONOID_FWD_HPP

#include "../models/function_object_fwd.hpp"
#include "../core/operator_helpers_fwd.hpp"
#include "../core/concepts.hpp"

namespace cxxmath
{
namespace concepts
{
template<class ComposeAssign, class Compose, class NeutralElement, class IsAbelian>
struct monoid;

template<class Compose, class NeutralElement, class IsAbelian> using non_assignable_monoid = monoid<impl::unsupported_implementation, Compose, NeutralElement, IsAbelian>;

template<class ComposeAssign, class NeutralElement, class IsAbelian> using assignable_monoid = monoid<ComposeAssign, impl::binary_operator<ComposeAssign, IsAbelian>, NeutralElement, IsAbelian>;

template<class> struct is_monoid : std::false_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_monoid)
}

CXXMATH_DEFINE_CONCEPT( monoid )
}

#endif //CXXMATH_CONCEPTS_MONOID_FWD_HPP
