//
// Created by jayz on 30.03.20.
//

#define BOOST_TEST_MODULE cxxmath - variant

#include <boost/test/included/unit_test.hpp>

#include "../cxxmath.hpp"

BOOST_AUTO_TEST_CASE( test_variant ) {
	using variant = boost::variant<int, double>;
	variant v1 = 5;
	variant v2 = 5.0;
	
	constexpr auto is_int = [] ( auto &&x ) {
		return std::is_same_v<std::decay_t<decltype(x)>, int>;
	};
	constexpr auto is_double = [] ( auto &&x ) {
		return std::is_same_v<std::decay_t<decltype(x)>, double>;
	};
	
	BOOST_TEST( cxxmath::visit( is_int, v1 ) );
	BOOST_TEST( !cxxmath::visit( is_double, v1 ) );
	
	BOOST_TEST( !cxxmath::visit( is_int, v2 ) );
	BOOST_TEST( cxxmath::visit( is_double, v2 ) );
}