//
// Created by jayz on 07.10.19.
//

#ifndef CXXMATH_MODELS_FREE_MONOID_HPP
#define CXXMATH_MODELS_FREE_MONOID_HPP

#include "std_vector.hpp"

namespace cxxmath {
template<class Symbol, class Allocator = std::allocator<Symbol>>
using free_monoid = typename vector_monoid<Symbol, Allocator>::type;
}

#endif //CXXMATH_MODELS_FREE_MONOID_HPP
