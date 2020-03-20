//
// Created by jayz on 08.10.19.
//

#ifndef CXXMATH_CLIFFORD_ALGEBRA_HPP
#define CXXMATH_CLIFFORD_ALGEBRA_HPP

#include "quotient_r_algebra.hpp"

namespace cxxmath {
namespace detail {
template<class TotalOrder>
struct less_from_total_order {
	template<class V1, class V2>
	decltype( auto ) operator()( V1 &&v1, V2 &&v2 ) const {
		return TotalOrder::less( std::forward<V1>( v1 ), std::forward<V2>( v2 ) );
	}
};
}

template<class CoefficientRing, class BilinearForm> struct clifford_quotient_spec {
	using multiplication_is_commutative = impl::false_implementation;
	
	struct negate_in_place: commutes_with_quotient_map_helper<true> {
		template<class FRA> static constexpr decltype( auto ) apply( FRA &&fra ) {
			static_assert( is_free_r_algebra_tag_v < tag_of_t < FRA >> );
			using algebra = typename model_free_r_algebra::free_r_algebra_concepts<tag_of_t < FRA>>::algebra;
			
			return algebra::negate_in_place( std::forward<FRA>( fra ) );
		}
	};
	
	struct scalar_multiply_assign: commutes_with_quotient_map_helper<true> {
		template<class Scalar, class FRA>
		static constexpr decltype( auto ) apply( Scalar &&s, FRA &&fra ) {
			static_assert( is_free_r_algebra_tag_v < tag_of_t < FRA >> );
			using algebra = typename model_free_r_algebra::free_r_algebra_concepts<tag_of_t < FRA>>::algebra;
			
			return algebra::scalar_multiply_assign(
				std::forward<Scalar>( s ), std::forward<FRA>( fra )
			);
		}
	};
	
	struct add_assign: commutes_with_quotient_map_helper<false> {
		template<class FRA1, class FRA2>
		static constexpr decltype( auto ) apply( FRA1 &fra1, FRA2 &&fra2 ) {
			static_assert( std::is_same_v<tag_of_t < FRA1>, tag_of_t < FRA2 >> );
			static_assert( is_free_r_algebra_tag_v < tag_of_t < FRA1 >> );
			using algebra = typename model_free_r_algebra::free_r_algebra_concepts<tag_of_t < FRA1>>::algebra;
			
			algebra::add_assign( fra1, std::forward<FRA2>( fra2 ) );
			return quotient_map_in_place::apply( fra1 );
		}
	};
	
	struct multiply_assign: commutes_with_quotient_map_helper<false> {
		template<class FRA1, class FRA2>
		static constexpr decltype( auto ) apply( FRA1 &fra1, FRA2 &&fra2 ) {
			static_assert( std::is_same_v<tag_of_t < FRA1>, tag_of_t < FRA2 >> );
			static_assert( is_free_r_algebra_tag_v < tag_of_t < FRA1 >> );
			using algebra = typename model_free_r_algebra::free_r_algebra_concepts<tag_of_t < FRA1>>::algebra;
			
			algebra::multiply_assign( fra1, std::forward<FRA2>( fra2 ) );
			return quotient_map_in_place::apply( fra1 );
		}
	};
	
	class quotient_map_in_place {
		template<class FRATag, class Monomial>
		static constexpr auto canonicalize_monomial( Monomial &&monomial ) {
			using algebra = typename model_free_r_algebra::free_r_algebra_concepts<FRATag>::algebra;
			
			using total_symbol_order = typename FRATag::symbol_total_order;
			using less_symbols = detail::less_from_total_order<total_symbol_order>;
			
			auto symbols = std::move( std_get_product::first( monomial ) );
			auto last_ordered = std::adjacent_find( symbols.begin(), symbols.end(), std::not_fn( less_symbols{} ) );
			
			if( last_ordered == symbols.end() ) {
				return make<FRATag>(
					std::move( std_get_product::second( monomial ) ), boost::make_iterator_range(
						std::make_move_iterator( symbols.begin() ),
						std::make_move_iterator( symbols.end() )
					)
				);
			}
			
			auto clifford_coefficient = BilinearForm::apply( *last_ordered, *( last_ordered + 1 ) );
			if( total_symbol_order::equal( *last_ordered, *( last_ordered + 1 ) ) ) {
				auto shortened_coefficient =
					CoefficientRing::multiply( std_get_product::second( monomial ), std::move( clifford_coefficient ) );
				symbols.erase( last_ordered, last_ordered + 2 );
				
				return canonicalize_monomial<FRATag>(
					std::make_pair(
						std::move( symbols ), std::move( shortened_coefficient )
					)
				);
			}
			
			auto permuted_coefficient = CoefficientRing::negate( std_get_product::second( monomial ) );
			std::swap( *last_ordered, *( last_ordered + 1 ) );
			auto canonicalized_polynomial = canonicalize_monomial<FRATag>(
				std::make_pair(
					symbols, std::move( permuted_coefficient )
				)
			);
			
			auto two = CoefficientRing::one();
			CoefficientRing::add_assign( two, two );
			
			auto shortened_coefficient = CoefficientRing::multiply(
				two, CoefficientRing::multiply(
					std::move( std_get_product::second( monomial ) ), std::move( clifford_coefficient )
				)
			);
			symbols.erase( last_ordered, last_ordered + 2 );
			algebra::add_assign(
				canonicalized_polynomial, canonicalize_monomial<FRATag>(
					std::make_pair(
						std::move( symbols ), std::move( shortened_coefficient )
					)
				)
			);
			
			return canonicalized_polynomial;
		}
	public:
		template<class FRA> static constexpr FRA &apply( FRA &fra ) {
			static_assert( is_free_r_algebra_tag_v<tag_of_t<FRA>> );
			using algebra = typename model_free_r_algebra::free_r_algebra_concepts<tag_of_t<FRA>>::algebra;
			
			auto canonicalized = default_r_algebra_t<tag_of_t<FRA>>::zero();
			for( auto &&monomial : fra.monomials() ) {
				algebra::add_assign( canonicalized, canonicalize_monomial<tag_of_t<FRA>>( std::move( monomial ) ) );
			}
			fra = std::move( canonicalized );
			return fra;
		}
	};
	
	struct equal {
		template<class FRA1, class FRA2>
		static constexpr decltype( auto ) apply( FRA1 &&fra1, FRA2 &&fra2 ) {
			static_assert( std::is_same_v<tag_of_t<FRA1>, tag_of_t<FRA2>> );
			static_assert( is_free_r_algebra_tag_v<tag_of_t<FRA1>> );
			using comparable = typename model_free_r_algebra::free_r_algebra_concepts<tag_of_t<FRA1>>::comparable;
			
			return comparable::equal( std::forward<FRA1>( fra1 ), std::forward<FRA2>( fra2 ) );
		}
	};
};
}

#endif //CXXMATH_CLIFFORD_ALGEBRA_HPP
