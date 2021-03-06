//
// Created by jayz on 28.09.19.
//

#define BOOST_TEST_MODULE cxxmath - product_set

#include <boost/test/included/unit_test.hpp>

#include "../cxxmath.hpp"

BOOST_AUTO_TEST_CASE( test_product_set_int_int )
{
	using namespace cxxmath;
	using default_std_pair_product = default_product_t<std_pair_tag>;
	using int_int_set = product_set<default_std_pair_product>;
	
	auto pair42_23 = make_product<default_std_pair_product>( 42, 23 );
	auto pair23_42 = make_product<default_std_pair_product>( 23, 42 );
	
	BOOST_TEST( int_int_set::equal( pair42_23, pair42_23 ));
	BOOST_TEST( int_int_set::equal( pair23_42, pair23_42 ));
	BOOST_TEST( int_int_set::not_equal( pair42_23, pair23_42 ));
	BOOST_TEST( int_int_set::not_equal( pair23_42, pair42_23 ));
}
