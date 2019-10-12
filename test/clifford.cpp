//
// Created by jayz on 28.08.19.
//

#define BOOST_TEST_MODULE Clifford Algebra

#include <boost/test/included/unit_test.hpp>

#include "cxxmath.hpp"

namespace {
struct gamma_matrix {
	std::variant<int, std::string_view> index;
};
struct formal_metric_entry {
	std::variant<int, std::string_view> index1, index2;
};
struct manifold_dimension {};
}

namespace cxxmath {
namespace impl {
struct less_manifold_indices {
	static constexpr bool apply( const std::variant<int, std::string_view> &i1, const std::variant<int, std::string_view> &i2 ) {
		if( std::holds_alternative<int>( i1) ) {
			if( std::holds_alternative<int>( i2 ) )
				return std::get<int>( i1 ) < std::get<int>( i2 );
			return true;
		}
		
		if( std::holds_alternative<int>( i2) )
			return false;
		
		return std::get<std::string_view>( i1 ) < std::get<std::string_view>( i2 );
	}
};

struct less_gamma_matrix {
	static constexpr bool apply( const gamma_matrix &g1, const gamma_matrix &g2 ) {
		return less_manifold_indices::apply( g1.index, g2.index );
	}
};

struct less_formal_metric_entry {
	static constexpr bool apply( const formal_metric_entry &fme1, const formal_metric_entry &fme2 ) {
		if( fme1.index1 < fme2.index1 )
			return true;
		if( fme1.index1 > fme2.index1 )
			return false;
		
		return fme1.index2 < fme2.index2;
	}
};
}

template<> struct default_total_order<gamma_matrix> {
	using type = concepts::total_order<impl::less_gamma_matrix>;
};
template<> struct default_total_order<formal_metric_entry> {
	using type = concepts::total_order<impl::less_formal_metric_entry>;
};
template<> struct default_total_order<manifold_dimension> {
	using type = concepts::total_order<impl::false_implementation>;
};
}

namespace {
using d_int_polynomial_tag = cxxmath::free_r_algebra_tag<int, manifold_dimension>;
using d_int_polynomial = decltype(make<d_int_polynomial_tag>());

using coefficient_tag = cxxmath::free_r_algebra_tag<d_int_polynomial, formal_metric_entry>;
using coefficient = decltype(make<coefficient_tag>());

using gamma_polynomial_tag = cxxmath::free_r_algebra_tag<coefficient_tag, gamma_matrix>;
using gamma_polynomial = decltype(make<gamma_polynomial_tag>());

struct euclidean_form_int
{
	static constexpr coefficient apply( const gamma_matrix &g1, const gamma_matrix &g2 )
	{
		if( std::holds_alternative<int>( g1.index ) ) {
			if( std::holds_alternative<int>( g2.index ) ) {
				if( std::get<int>( g1.index ) == std::get<int>( g2.index ) )
					return make<coefficient_tag>( make<d_int_polynomial_tag>( 1 ) );
				return make<coefficient_tag>( make<d_int_polynomial_tag>( 0 ) );
			}
			
			return make<coefficient_tag>( make<d_int_polynomial_tag>( 1 ), formal_metric_entry{ g1.index, g2.index } );
		}
		
		if( std::holds_alternative<int>( g2.index ) )
			return make<coefficient_tag>( make<d_int_polynomial_tag>( 1 ), formal_metric_entry{ g1.index, g2.index } );
		
		if( std::get<std::string_view>( g1.index ) == std::get<std::string_view>( g2.index ) )
			return make<coefficient_tag>( make<d_int_polynomial_tag>( 1, manifold_dimension ) );
		
		return make<coefficient_tag>( make<d_int_polynomial_tag>( 1 ), formal_metric_entry{ g1.index, g2.index } );
	}
};

template<class T> struct is_variant : std::false_type {};
template<class T> static constexpr bool is_variant_v = is_variant<T>::value;

template<class ...Alternatives> struct is_variant<std::variant<Alternatives...>> : std::true_type {};

struct gamma_index_handler {
	using less_indices = impl::less_manifold_indices;
	
	struct extract_indices {
		static constexpr std::array<std::variant<int, std::string_view>, 1> apply( const gamma_matrix &gm ) {
			return { gm.index };
		}
		
		static constexpr std::array<std::variant<int, std::string_view>, 2> apply( const formal_metric_entry &fme ) {
			return { fm.index1, fm.index2 };
		}
		
		static constexpr std::array<std::variant<int, std::string_view>, 0> apply( const manifold_dimension & ) {
			return {};
		}
	};
	
	template<class FRATag> class contract_indices {
		static_assert( std::is_same_v<FRATag, gamma_polynomial_tag> );
		
		template<class Range, class IndexRange>
		static gamma_polynomial apply_impl( const gamma_matrix &gm1, const Range &parts, const gamma_matrix &gm2, const IndexRange &indices ) {
			if( parts.size() != 0 )
				throw std::rundtime_error( "part size should be zero" );
			if( indices.size != 1 )
				throw std::rundtime_error( "indices size should be one" );
			if( gm1.index != indices[0] )
				throw std::rundtime_error( "gm1 index mismatch" );
			if( gm2.index != indices[0] )
				throw std::rundtime_error( "gm2 index mismatch" );
			
			throw std::rundtime_error( "This should never be called." );
		}
		
		template<class Range, class IndexRange>
		static gamma_polynomial apply_impl( const gamma_matrix &gm1, const Range &parts, const formal_metric_entry &fme2, const IndexRange &indices ) {
		
		}
		
		template<class Range, class IndexRange>
		static gamma_polynomial apply_impl( const formal_metric_entry &fme1, const Range &parts, const gamma_matrix &gm2, const IndexRange &indices ) {
		
		}
		
		template<class Range, class IndexRange>
		static gamma_polynomial apply_impl( const formal_metric_entry &fme1, const Range &parts, const formal_metric_entry &fme2, const IndexRange &indices ) {
		
		}
	public:
		template<class IndexedObject1, class Range, class IndexedObject2, class IndexRange>
		static gamma_polynomial apply( const IndexedObject1 &io1, const Range &parts, const IndexedObject2 &io2, const IndexRange &indices ) {
			if constexpr( is_variant_v<IndexedObject1> )
				return std::visit( [&] ( const auto &io ) { return apply( io, parts, io2, indices ); }, io1 );
			else if constexpr( is_variant_v<IndexedObject1> )
				return std::visit( [&] ( const auto &io ) { return apply( io1, parts, io, indices ); }, io2 );
			else
				return apply_impl( io1, parts, io2, indices );
		}
	};
};

using clifford_quotient_spec = cxxmath::composed_quotients<
	cxxmath::abstract_index_quotient_spec<gamma_index_handler>,
	cxxmath::clifford_quotient_spec<
		default_ring_t<coefficient_tag>, euclidean_form_int
	>
>;

using clifford_int_spec = cxxmath::clifford_quotient_spec<cxxmath::default_ring_t<int>, euclidean_form_int>;
using free_gamma_algebra_tag = cxxmath::free_r_algebra_tag<int, gamma_matrix>;
using clifford_tag = cxxmath::quotient_r_algebra_tag<free_gamma_algebra_tag, clifford_int_spec>;

std::ostream &operator<<( std::ostream &os, const gamma_matrix &g )
{
	return os << "gamma_" << g.index;
}

template<class Monomial>
void print_monomial( std::ostream &os, const Monomial &m )
{
	os << std::get<1>( m );
	for( const auto &v : std::get<0>( m ))
		os << " " << v;
}

template<class Clifford, CXXMATH_ENABLE_IF_TAG_IS(Clifford, clifford_tag)>
std::ostream &operator<<( std::ostream &os, const Clifford &cl ) {
	os << "[";
	const auto &monomials = cl.representative().monomials();
	if( monomials.empty())
		return os << "0";
	
	auto it = monomials.begin();
	print_monomial( os, *it );
	for( ++it; it != monomials.end(); ++it ) {
		os << " + ";
		print_monomial( os, *it );
	}
	return os << "]";
}
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
	
	BOOST_TEST( gamma_1 * gamma_1 == one );
	BOOST_TEST( gamma_2 * gamma_2 == one );
	BOOST_TEST( gamma_3 * gamma_3 == one );
	
	BOOST_TEST( gamma_1 * gamma_2 != - gamma_1 * gamma_2 );
	BOOST_TEST( gamma_1 * gamma_2 != gamma_2 * gamma_1 );
	
	BOOST_TEST( gamma_1 * gamma_2 == - gamma_2 * gamma_1 );
	BOOST_TEST( gamma_1 * gamma_4 == - gamma_4 * gamma_1 );
	
	BOOST_TEST( gamma_1 * gamma_2 * gamma_3 == gamma_2 * gamma_3 * gamma_1 );
	BOOST_TEST( gamma_2 * gamma_1 * gamma_3 == - gamma_2 * gamma_3 * gamma_1 );
	BOOST_TEST( gamma_1 * gamma_2 * gamma_1 == - gamma_2 );
}
