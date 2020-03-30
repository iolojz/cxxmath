//
// Created by jayz on 28.09.19.
//

#define BOOST_TEST_MODULE cxxmath - product_comparable

#include <boost/test/included/unit_test.hpp>

#include "../cxxmath.hpp"

BOOST_AUTO_TEST_CASE( test_product_comparable_int_int ) {
	using namespace cxxmath;
	using int_comparable = default_comparable_t<tag_of_t<int>>;
	
	using int_int_comparable = product_comparable<std_pair_product, int_comparable, int_comparable>;
	
	auto pair42_23 = make_pair( 42, 23 );
	auto pair23_42 = make_pair( 23, 42 );
	
	BOOST_TEST( int_int_comparable::equal( pair42_23, pair42_23 ) );
	BOOST_TEST( int_int_comparable::equal( pair23_42, pair23_42 ) );
	BOOST_TEST( int_int_comparable::not_equal( pair42_23, pair23_42 ) );
	BOOST_TEST( int_int_comparable::not_equal( pair23_42, pair42_23 ) );
}
