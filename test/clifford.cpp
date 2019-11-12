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

std::ostream &operator<<( std::ostream &os, const gamma_matrix &g )
{
	return os << "gamma_" << g.index;
}

std::ostream &operator<<( std::ostream &os, const formal_metric_entry &fme )
{
	return os << "g_{" << fme.index1 << ", " << fme.index2 << "}";
}

std::ostream &operator<<( std::ostream &os, const manifold_dimension & )
{
	return os << "D";
}

template<class Monomial>
void print_monomial( std::ostream &os, const Monomial &m )
{
	os << std::get<1>( m );
	for( const auto &v : std::get<0>( m ))
		os << " " << v;
}
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
		if( less_manifold_indices::apply( fme1.index1, fme2.index1 )
			return true;
		if( less_manifold_indices::apply( fme2.index1, fme1.index1 )
			return false;
		
		return less_manifold_indices::apply( fme1.index2, fme2.index2 );
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

template<class FRA, class = std::enable_if_t<is_free_r_algebra_tag_v<tag_of_t<FRA>>>>
std::ostream &operator<<( std::ostream &os, const FRA &fra )
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

template<class QRA, class = std::enable_if_t<is_quotient_r_algebra_tag_v<tag_of_t<QRA>>>>
std::ostream &operator<<( std::ostream &os, const QRA &qra )
{
	return os << "[" << qra.representative() << "]";
}
}

namespace {
struct formal_metric_index_handler {
	using less_indices = cxxmath::impl::less_manifold_indices;
	
	struct extract_indices {
		static constexpr auto apply(const formal_metric_entry &fme) {
			std::vector<std::string_view> open_indices;
			
			if(std::holds_alternative<std::string_view>(fme.index1))
				open_indices.push_back(std::get<std::string_view>(fme.index1));
			if(std::holds_alternative<std::string_view>(fme.index2))
				open_indices.push_back(std::get<std::string_view>(fme.index2));
			
			return open_indices;
		}
		
		template<class T>
		static constexpr std::array<std::string_view, 0> apply(T &&) { return {}; }
	};
	
	template<class AlgebraTag>
	class contract_indices;
};

struct gamma_index_handler {
	using less_indices = cxxmath::impl::less_manifold_indices;
	
	struct extract_indices {
		static constexpr std::vector<std::string_view> apply(const gamma_matrix &gm) {
			if(std::holds_alternative<std::string_view>(gm.index))
				return { std::get<std::string_view>( gm.index ) };
			
			return {};
		}
		
		template<class T>
		static constexpr auto apply(T &&t) {
			return formal_metric_index_handler::extract_indices( std::forward<T>( t ) );
		}
	};
	
	template<class AlgebraTag>
	class contract_indices;
};

namespace type_helpers {
using d_int_polynomial = cxxmath::free_r_algebra<int, manifold_dimension>;

using free_metric_entry_algebra = cxxmath::free_r_algebra<d_int_polynomial, formal_metric_entry>;
using metric_entry_quotient_spec = cxxmath::abstract_index_quotient_spec<formal_metric_index_handler>;
using metric_entry_algebra = cxxmath::quotient_r_algebra<free_metric_entry_algebra, coefficient_quotient_spec>;

using free_gamma_algebra = cxxmath::free_r_algebra_tag<metric_entry_algebra, gamma_matrix>;
using clifford_quotient_spec = cxxmath::composed_quotients<
	cxxmath::abstract_index_quotient_spec<gamma_index_handler>,
	cxxmath::clifford_quotient_spec<
		default_ring_t<tag_of_t<metric_entry_algebra>>,
		euclidean_form
	>
>;
}

using clifford = cxxmath::quotient_r_algebra<type_helpers::free_gamma_algebra, type_helpers::clifford_quotient_spec>;

template<> class formal_metric_index_handler::contract_indices<tag_of_t<type_helpers::metric_entry_algebra>> {
	using d_int_polynomial_tag = tag_of_t<type_helpers::d_int_polynomial>;
	using metric_entry_algebra_tag = tag_of_t<type_helpers::metric_entry_algebra>;
	
	template<class Range, class IndexRange>
	static auto apply_impl( const formal_metric_entry &fme, const IndexRange &indices ) {
		if( std::size( indices ) != 1 )
			throw std::runtime_error( "self-contraction of formal metric entry with size( indices ) != 1" );
		
		const auto &index = *std::begin( indices );
		if( fme.index1 != index )
			throw std::runtime_error( "self-contraction of formal metric entry with mismatch in first index" );
		else if( fme.index2 != index )
			throw std::runtime_error( "self-contraction of formal metric entry with mismatch in second index" );
		
		auto result_coefficient = cxxmath::make<d_int_polynomial_tag>( 1, manifold_dimension );
		return cxxmath::make<metric_entry_algebra_tag>( std::move( result_coefficient ) );
	}
	
	template<class Range, class IndexRange>
	static auto apply_impl( const formal_metric_entry &fme1, const Range &parts, const formal_metric_entry &fme2, const IndexRange &indices ) {
		auto coefficient_one = cxxmath::make<d_int_polynomial_tag>( 1 );
		
		if( std::size( indices ) == 1 ) {
			auto result_coefficient = cxxmath::make<d_int_polynomial_tag>( 1 );
			formal_metric_entry resulting_metric_entry;
			
			if( fme1.index1 == fme2.index1 )
				resulting_metric_entry = formal_metric_entry{ fme1.index2, fme2.index2 };
			else if( fme1.index1 == fme2.index2 )
				resulting_metric_entry = formal_metric_entry{ fme1.index2, fme2.index1 };
			else if( fme1.index2 == fme2.index1 )
				resulting_metric_entry = formal_metric_entry{ fme1.index1, fme2.index2 };
			else if( fme1.index2 == fme2.index2 )
				resulting_metric_entry = formal_metric_entry{ fme1.index1, fme2.index1 };
			else
				throw std::runtime_error( "cross-contraction(1) of formal metric entries with mismatching indices" );
			
			auto metric_entries = boost::range::join(
				boost::make_iterator_range( &resulting_metric_entry, (&resulting_metric_entry) + 1 ),
				parts
			);
			
			return cxxmath::make<tag_of_t<coefficient>>( std::move( coefficient_one ), std::move( metric_entries ) );
		} else if( std::size( indices ) == 2 ) {
			if( !(fme1.index1 == fme2.index1 && fme1.index2 == fme2.index2) && !(fme1.index1 == fme2.index2 && fme1.index2 == fme2.index1) )
				throw std::runtime_error( "cross-contraction(2) of formal metric entries with mismatching indices" );
			
			auto result_coefficient = cxxmath::make<tag_of_t<d_int_polynomial>>( 1, manifold_dimension );
			return cxxmath::make<tag_of_t<coefficient>>( std::move( result_coefficient ), parts );
		}
		
		throw std::runtime_error( "cross-contraction of formal metric entries with size( indices ) != 1 and != 2" );
	}
};

template<> class gamma_index_handler::contract_indices<tag_of_t<clifford>> {
	template<class Range> decltype(auto) compose_coefficient( Range &&r ) {
		return cxxmath::detail::coefficient_composer<
		    type_helpers::free_gamma_algebra,
		    cxxmath::default_ring_t<type_helpers::free_gamma_algebra>
		>( std::forward<Range>( r ) );
	}
	
	template<class Range> auto to_gamma_range( Range &&r ) {
		std::vector<gamma_matrix> gammas;
		std::for_each( std::begin( r ), std::end( r ), [&] ( auto &&gm ) {
			if constexpr( std::is_same_v<std::decay_t<decltype(gm)>, gamma_matrix> )
				gammas.push_back( gm );
			else
				gammas.push_back( std::get<gamma_matrix>( gm ) );
		} );
		return gammas;
	}
	
	template<class Range, class IndexRange>
	static clifford apply_impl( const gamma_matrix &, const Range &, const gamma_matrix &, const IndexRange & ) {
		throw std::rundtime_error( "Clifford quotient algebra should eliminate this possibility." );
	}
	
	template<class Range, class IndexRange>
	static clifford apply_impl( const gamma_matrix &, const Range &, const formal_metric_entry &, const IndexRange & ) {
		throw std::rundtime_error( "Coefficients should always precede the symbols in index contractions." );
	}
	
	template<class Range, class IndexRange>
	static clifford apply_impl( const formal_metric_entry &fme1, const Range &parts, const gamma_matrix &gm2, const IndexRange &indices ) {
		gamma_matrix resulting_gamma;
		
		if( indices.size != 1 )
			throw std::rundtime_error( "indices size should be one" );
		if( gm2.index == fme1.index1 )
			resulting_gamma.index = fme1.index2;
		else if( gm2.index == fme1.index2 )
			resulting_gamma.index = fme1.index1;
		else
			throw std::rundtime_error( "gm2/fme1 index mismatch" );
		
		auto it = std::find_if( std::begin( parts ), std::end( parts ), [] ( auto &&part ) {
			return std::is_same_v<std::decay_t<decltype(part)>, gamma_matrix>;
		} );
		
		auto coefficient = compose_coefficient( boost::make_iterator_range( std::begin( parts ), it ) );
		auto gammas = to_gamma_range(
			boost::join(
				boost::make_iterator_range( it, std::end( parts ) ),
				boost::make_iterator_range( &resulting_gamma, (&resulting_gamma) + 1 )
			)
		);
		
		return cxxmath::make<tag_of_t<clifford>>( std::move( coefficient ), std::move( gammas ) );
	}
	
	template<class Range, class IndexRange>
	static clifford apply_impl( const formal_metric_entry &, const Range &, const formal_metric_entry &, const IndexRange & ) {
		throw std::rundtime_error( "This contraction should be handled in formal_metric_index_handler::contract_indices." );
	}
public:
	template<class IndexedObject, class IndexRange>
	static clifford apply( const IndexedObject &, const IndexRange & ) {
		throw std::rundtime_error( "A self-contraction of a gamma matrix is impossible." );
	}
	
	template<class IndexedObject1, class Range, class IndexedObject2, class IndexRange>
	static clifford apply( const IndexedObject1 &io1, const Range &parts, const IndexedObject2 &io2, const IndexRange &indices ) {
		if constexpr( is_variant_v<IndexedObject1> )
			return std::visit( [&] ( const auto &io ) { return apply( io, parts, io2, indices ); }, io1 );
		else if constexpr( is_variant_v<IndexedObject1> )
			return std::visit( [&] ( const auto &io ) { return apply( io1, parts, io, indices ); }, io2 );
		else
			return apply_impl( io1, parts, io2, indices );
	}
};
}

BOOST_AUTO_TEST_CASE( clifford_algebra_int )
{
	using namespace cxxmath;
	using clifford_tag = tag_of_t<clifford>;
	
	const auto coefficient_one = clifford_tag::coefficient_ring::one();
	const auto one = default_ring_t<clifford_tag>::one();
	
	const auto gamma_1 = make<clifford_tag>( coefficient_one, gamma_matrix{ 1 } );
	const auto gamma_2 = make<clifford_tag>( coefficient_one, gamma_matrix{ 2 } );
	const auto gamma_3 = make<clifford_tag>( coefficient_one, gamma_matrix{ 3 } );
	const auto gamma_4 = make<clifford_tag>( coefficient_one, gamma_matrix{ 4 } );
	
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
