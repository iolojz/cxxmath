//
// Created by jayz on 08.10.19.
//

#ifndef CXXMATH_CORE_IS_RANGE_HPP
#define CXXMATH_CORE_IS_RANGE_HPP

#include <iterator>
#include <type_traits>

namespace cxxmath {
namespace detail {
struct call_std_begin {
	template<class Arg> constexpr operator()( Arg &&arg ) {
		return std::begin( std::forward<Arg>( arg ) );
	}
};
}

template<class Range> struct is_range {
	static constexpr bool value = std::is_invocable_v<detail::call_std_begin, Range>;
};
template<class Range> static constexpr is_range_v = is_range<Range>::value;
}

#endif //CXXMATH_CORE_IS_RANGE_HPP
