//
// Created by jayz on 04.09.19.
//

#define BOOST_TEST_MODULE Tag Dispatch - Free Polynomial

#include <boost/test/included/unit_test.hpp>

#include "cxxmath.hpp"

namespace
{
template<class Monomial>
void print_monomial( std::ostream &os, const Monomial &m );

template<class Tag> struct is_free_algebra_tag : std::false_type {};

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
struct is_free_algebra_tag<free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>>
: std::true_type {};
}

namespace cxxmath
{
template<class FRA, class = std::enable_if_t<is_free_algebra_tag < tag_of_t < Polynomial>> ::value>>
std::ostream &operator<<( std::ostream &os, const Polynomial &p )
{
	const auto &monomials = p.monomials();
	if( monomials.empty())
		return os << "0";
	
	auto it = monomials.begin();
	print_monomial( os, *it );
	for( ++it; it != monomials.end(); ++it ) {
		os << " + ";
		print_monomial( os, *it );
	}
	return os;
}
}

namespace
{
template<class Monomial>
void print_monomial( std::ostream &os, const Monomial &m )
{
	os << std::get<1>( m );
	for( const auto &v : std::get<0>( m ))
		os << " " << v;
}
}

BOOST_AUTO_TEST_CASE( test_free_r_algebra )
{
	using namespace cxxmath;
	using polynomial_tag = free_r_algebra_tag<int, std::string_view>;
	using polynomial_ring = default_ring_t<polynomial_tag>;
	
	static_assert( polynomial_ring::is_abelian_ring() == false );
	
	const auto zero_ = make_polynomial<polynomial_tag>( 0 );
	const auto one_ = make_polynomial<polynomial_tag>( 1 );
	
	BOOST_TEST( equal( zero_, polynomial_ring::zero()));
	BOOST_TEST( not_equal( one_, polynomial_ring::zero()));
	
	BOOST_TEST( equal( one_, polynomial_ring::one()));
	BOOST_TEST( not_equal( zero_, polynomial_ring::one()));
	
	const auto ab42 = make_polynomial<polynomial_tag>( 42, "a", "b" );
	const auto ba23 = make_polynomial<polynomial_tag>( 23, "b", "a" );
	const auto abm42 = make_polynomial<polynomial_tag>( -42, "a", "b" );
	
	const std::array<std::string_view, 2> ab = { "a", "b" };
	const std::array<std::string_view, 2> ba = { "b", "a" };
	
	const std::array<std::string_view, 4> abba = { "a", "b", "b", "a" };
	const std::array<std::string_view, 4> baab = { "b", "a", "a", "b" };
	const std::array<std::string_view, 4> abab = { "a", "b", "a", "b" };
	const std::array<std::string_view, 4> baba = { "b", "a", "b", "a" };
	
	const auto ab84 = make_polynomial<polynomial_tag>( 84, "a", "b" );
	const auto abab1764 = make_polynomial<polynomial_tag>( 42 * 42, "a", "b", "a", "b" );
	
	BOOST_TEST( add( ab42, ab42 ) == ab84 );
	BOOST_TEST( multiply( ab42, ab42 ) == abab1764 );
	
	const auto ab42_ba23 = make_polynomial<polynomial_tag>( std::make_pair( ab, 42 ), std::make_pair( ba, 23 ));
	const auto abab1764_abba966_baab966_baba529 = make_polynomial<polynomial_tag>( std::make_pair( abab, 1764 ),
																				   std::make_pair( abba, 966 ),
																				   std::make_pair( baab, 966 ),
																				   std::make_pair( baba, 529 ));
	
	BOOST_TEST( add( ab42, ba23 ) == ab42_ba23 );
	BOOST_TEST( subtract( ab42_ba23, ba23 ) == ab42 );
	BOOST_TEST( negate( ab42 ) == abm42 );
	BOOST_TEST( multiply( ab42_ba23, ab42_ba23 ) == abab1764_abba966_baab966_baba529 );
	
	auto p = ab42;
	BOOST_TEST( add_assign( p, ba23 ) == ab42_ba23 );
	BOOST_TEST( p == ab42_ba23 );
	
	BOOST_TEST( subtract_assign( p, ba23 ) == ab42 );
	BOOST_TEST( p == ab42 );
	
	BOOST_TEST( negate_in_place( p ) == abm42 );
	BOOST_TEST( p == abm42 );
	BOOST_TEST( negate_in_place( p ) == ab42 );
	BOOST_TEST( p == ab42 );
	
	BOOST_TEST( add_assign( p, ba23 ) == ab42_ba23 );
	BOOST_TEST( multiply_assign( p, p ) == abab1764_abba966_baab966_baba529 );
	BOOST_TEST( p == abab1764_abba966_baab966_baba529 );
	
	BOOST_TEST( scalar_multiply( 2, ab42 ) == ab84 );
	
	p = ab42;
	BOOST_TEST( polynomial_r_algebra<polynomial_tag>::scalar_multiply_assign( 2, p ) == ab84 );
	BOOST_TEST( p == ab84 );
}
