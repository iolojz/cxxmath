//
// Created by jayz on 28.08.19.
//

#define BOOST_TEST_MODULE Clifford Algebra

#include <boost/test/included/unit_test.hpp>

#include "cxxmath.hpp"

namespace {
struct gamma_matrix {
	int index;
};
}

namespace cxxmath {
namespace impl {
struct less_gamma_matrix {
	static constexpr bool apply( const gamma_matrix &g1, const gamma_matrix &g2 ) {
		return g1.index < g2.index;
	}
};
}

template<> struct default_total_order<gamma_matrix> {
	using type = concepts::total_order<impl::less_gamma_matrix>;
};
}

namespace {
struct euclidean_form_int
{
	static constexpr int apply( const gamma_matrix &g1, const gamma_matrix &g2 )
	{
		if(  g1.index == g2.index )
			return 1;
		
		return 0;
	}
};

using clifford_int_spec = cxxmath::clifford_quotient_spec<cxxmath::default_ring_t<int>, euclidean_form_int>;
using free_gamma_algebra_tag = cxxmath::free_r_algebra_tag<int, gamma_matrix>;
using clifford_tag = cxxmath::quotient_r_algebra_tag<free_gamma_algebra_tag, clifford_int_spec>;
}

BOOST_AUTO_TEST_CASE( clifford_algebra_int )
{
	using namespace cxxmath;
	
	static_assert( models_concept_v<int, std_get_product> == false );
	
	const auto gamma_1_symbol = gamma_matrix{ 1 };
	const auto gamma_2_symbol = gamma_matrix{ 2 };
	const auto gamma_3_symbol = gamma_matrix{ 3 };
	const auto gamma_4_symbol = gamma_matrix{ 4 };
	
	const auto one = default_ring_t<clifford_tag>::one();
	
	const auto gamma_1 = make<clifford_tag>( 1, gamma_1_symbol );
	const auto gamma_2 = make<clifford_tag>( 1, gamma_2_symbol );
	const auto gamma_3 = make<clifford_tag>( 1, gamma_3_symbol );
	const auto gamma_4 = make<clifford_tag>( 1, gamma_4_symbol );
	
	BOOST_TEST( equal( multiply( gamma_1, gamma_1 ), one ) );
	BOOST_TEST( equal( multiply( gamma_2, gamma_2 ), one ) );
	BOOST_TEST( equal( multiply( gamma_3, gamma_3 ), one ) );
	
	BOOST_TEST( not_equal( multiply( gamma_1, gamma_2 ), negate( multiply( gamma_1, gamma_2 ) ) ) );
	BOOST_TEST( not_equal( multiply( gamma_1, gamma_2 ), multiply( gamma_2, gamma_1 ) ) );
	
	BOOST_TEST( equal( multiply( gamma_1, gamma_2 ), negate( multiply( gamma_2, gamma_1 ) ) ) );
	BOOST_TEST( equal( multiply( gamma_1, gamma_4 ), negate( multiply( gamma_4, gamma_1 ) ) ) );
	
	BOOST_TEST( equal( multiply( gamma_1, multiply( gamma_2, gamma_3 ) ), multiply( gamma_2, multiply( gamma_3, gamma_1 ) ) ) );
	BOOST_TEST( equal( multiply( gamma_2, multiply( gamma_1, gamma_3 ) ), negate( multiply( gamma_2, multiply( gamma_3, gamma_1 ) ) ) ) );
	BOOST_TEST( equal( multiply( gamma_1, multiply( gamma_2, gamma_1 ) ), negate( gamma_2 ) ) );
}
