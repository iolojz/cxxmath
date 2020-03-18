//
// Created by jayz on 18.03.20.
//

#ifndef CXXMATH_CORE_IS_VALID_HPP
#define CXXMATH_CORE_IS_VALID_HPP

#include <boost/hana.hpp>

#define CXXMATH_IS_VALID(f, ...) \
::boost::hana::value<decltype( \
	::boost::hana::is_valid( f, __VA_ARGS__ ) \
)>()

#endif //CXXMATH_CORE_IS_VALID_HPP
