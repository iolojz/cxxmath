//
// Created by jayz on 27.09.19.
//

#define BOOST_TEST_MODULE cxxmath - product_monoid

#include <boost/test/included/unit_test.hpp>

#include "cxxmath.hpp"

namespace std
{
template<class First, class Second>
std::ostream &operator<<( std::ostream &os, const std::pair<First, Second> &p )
{
	return os << "{" << cxxmath::first( p ) << ", " << cxxmath::second( p ) << "}";
}

template<class Symbol, class Allocator>
std::ostream &operator<<( std::ostream &os, const std::vector<Symbol, Allocator> &v )
{
	if( v.empty())
		return os << "{}";
	
	auto it = v.begin();
	os << "{ " << *it;
	while( ++it != v.end())
		os << ", " << *it;
	return os << " }";
}
}

BOOST_AUTO_TEST_CASE( test_product_monoid_int_int )
{
	using namespace cxxmath;
	
	using std_pair_int_int_product = model_std_pair::product<tag_of_t<int>, tag_of_t<int>>;
	using std_pair_int_int_monoid = product_monoid<std_pair_int_int_product>;
	
	using first_monoid = typename default_group_t<int>::monoid_;
	using second_monoid = default_monoid_t<int>;
	using mixed_int_int_monoid = product_monoid<std_pair_int_int_product, first_monoid, second_monoid>;
	
	auto pair0_1 = make_product<std_pair_int_int_product>( 0, 1 );
	auto pair1_1 = make_product<std_pair_int_int_product>( 1, 1 );
	auto pair42_23 = make_product<std_pair_int_int_product>( 42, 23 );
	auto pair1764_529 = make_product<std_pair_int_int_product>( 42 * 42, 23 * 23 );
	auto pair84_529 = make_product<std_pair_int_int_product>( 42 + 42, 23 * 23 );
	
	auto pair = pair42_23;
	
	BOOST_TEST( std_pair_int_int_monoid::compose( pair42_23, pair42_23 ) == pair1764_529 );
	BOOST_TEST( std_pair_int_int_monoid::neutral_element() == pair1_1 );
	BOOST_TEST( std_pair_int_int_monoid::is_abelian_monoid() == true );
	BOOST_TEST( std_pair_int_int_monoid::compose_assign( pair, pair ) == pair1764_529 );
	BOOST_TEST( pair == pair1764_529 );
	
	pair = pair42_23;
	
	BOOST_TEST( mixed_int_int_monoid::compose( pair42_23, pair42_23 ) == pair84_529 );
	BOOST_TEST( mixed_int_int_monoid::neutral_element() == pair0_1 );
	BOOST_TEST( mixed_int_int_monoid::is_abelian_monoid() == true );
	BOOST_TEST( mixed_int_int_monoid::compose_assign( pair, pair ) == pair84_529 );
	BOOST_TEST( pair == pair84_529 );
}

BOOST_AUTO_TEST_CASE( test_product_monoid_int_vector )
{
	using namespace cxxmath;
	using default_std_pair_product = default_product_t<std_pair_tag>;
	using int_vector_monoid = product_monoid<default_std_pair_product>;
	using string_vector = std::vector<std::string_view>;
	
	string_vector empty;
	string_vector a = { std::string_view( "a" ) };
	string_vector aa = { std::string_view( "a" ), std::string_view( "a" ) };
	
	auto pair1_empty = make_product<default_std_pair_product>( 1, empty );
	auto pair42_a = make_product<default_std_pair_product>( 42, a );
	auto pair1764_aa = make_product<default_std_pair_product>( 42 * 42, aa );
	
	auto pair = pair42_a;
	
	BOOST_TEST( int_vector_monoid::compose( pair42_a, pair42_a ) == pair1764_aa );
	BOOST_TEST( int_vector_monoid::compose_assign( pair, pair ) == pair1764_aa );
	BOOST_TEST( pair == pair1764_aa );
}