//
// Created by jayz on 03.04.20.
//

#ifndef CXXMATH_HELPERS_STD_ARRAY_HPP
#define CXXMATH_HELPERS_STD_ARRAY_HPP

#include <array>

namespace cxxmath {
template<class> struct is_std_array : std::false_type {};
template<class T, std::size_t N> struct is_std_array<std::array<T, N>> : std::true_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_std_array)

struct std_array_tag {};

namespace impl {
template<class T, std::size_t N> struct tag_of<std::array<T, N>> {
	using type = std_array_tag;
};
}

namespace detail {
template<class StdArrayType, class ForwardIterator, class ...Unpacked>
StdArrayType unpack_into_array( ForwardIterator current, ForwardIterator end, Unpacked &&...unpacked ) {
	if constexpr( sizeof...(Unpacked) == std::tuple_size<StdArrayType>::value )
		return { std::forward<Unpacked>( unpacked )... };
	else if( current == end )
		throw std::out_of_range( "unpack_into_array(): too few elements provided." );
	else {
		ForwardIterator current_copy = current;
		return unpack_into_array<StdArrayType>( ++current, end, std::forward<Unpacked>( unpacked )..., *current_copy );
	}
}
}

template<class StdArrayType, class Range> StdArrayType to_array( Range &&range ) {
	static_assert( is_std_array_v<StdArrayType>, "Given StdArrayType is not an std::array<>." );
	return detail::unpack_into_array<StdArrayType>( boost::begin( range ), boost::end( range ) );
}
}

#endif // CXXMATH_HELPERS_STD_ARRAY_HPP
