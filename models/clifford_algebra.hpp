//
// Created by jayz on 08.10.19.
//

#ifndef CXXMATH_CLIFFORD_ALGEBRA_HPP
#define CXXMATH_CLIFFORD_ALGEBRA_HPP

#include "quotient_r_algebra.hpp"

namespace cxxmath {
namespace detail {
template<class FRATag> struct total_symbol_order_for_free_r_algebra_tag;

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
struct total_symbol_order_for_free_r_algebra_tag<
	free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>
> {
	using type = SymbolTotalOrder;
};

template<class TotalOrder>
struct less_from_total_order
{
	template<class V1, class V2>
	decltype( auto ) operator()( V1 &&v1, V2 &&v2 ) const
	{
		return TotalOrder::less( std::forward<V1>( v1 ), std::forward<V2>( v2 ));
	}
};
}

template<class CoefficientRing, class BilinearForm> struct clifford_quotient_spec {
	using multiplication_is_commutative = impl::false_implementation;
	
	struct negate_in_place {
		template<class FRA> static constexpr decltype(auto) apply( FRA &&fra ) {
			return default_r_algebra_t<tag_of_t<FRA>>::negate_in_place( std::forward<FRA>( fra ) );
		}
	};
	
	struct scalar_multiply_assign
	{
		template<class Scalar, class FRA>
		static constexpr decltype(auto) apply( Scalar &&s, FRA &&fra )
		{
			return default_r_algebra_t< tag_of_t < FRA >>::scalar_multiply_assign(
				std::forward<Scalar>( s ), std::forward<FRA>( fra )
			);
		}
	};
	
	struct add_assign {
		template<class FRA1, class FRA2>
		static constexpr decltype(auto) apply( FRA1 &fra1, FRA2 &&fra2 )
		{
			default_r_algebra_t< tag_of_t < FRA1 >>::add_assign( fra1, std::forward<FRA2>( fra2 ) );
			return quotient_map_in_place( fra1 );
		}
	};

	struct multiply_assign
	{
		template<class FRA1, class FRA2>
		static constexpr decltype( auto ) apply( FRA1 &fra1, FRA2 &&fra2 )
		{
			default_r_algebra_t<tag_of_t<FRA1>>::multiply_assign( fra1, std::forward<FRA2>( fra2 ));
			return quotient_map_in_place::apply( fra1 );
		}
	};
	
	class quotient_map_in_place {
		template<class FRATag, class Monomial>
		static constexpr decltype(default_r_algebra_t<FRATag>::zero())	canonicalize_monomial( Monomial &&monomial ) {
			using total_symbol_order = typename detail::total_symbol_order_for_free_r_algebra_tag<FRATag>::type;
			using less_symbols = detail::less_from_total_order<total_symbol_order>;
			
			auto symbols = std::move( std_get_product::first( monomial ) );
			auto last_ordered = std::adjacent_find( symbols.begin(), symbols.end(), std::not_fn( less_symbols{} ));
			
			if( last_ordered == symbols.end())
				return make<FRATag>( std::move( std_get_product::second( monomial ) ), boost::make_iterator_range(
					std::make_move_iterator( symbols.begin() ),
					std::make_move_iterator( symbols.end() )
				) );
			
			auto clifford_coefficient = BilinearForm::apply( *last_ordered, *( last_ordered + 1 ));
			if( total_symbol_order::equal( *last_ordered, *( last_ordered + 1 ))) {
				auto shortened_coefficient = std::move( std_get_product::second( monomial ) ) * std::move( clifford_coefficient );
				symbols.erase( last_ordered, last_ordered + 2 );
				
				return canonicalize_monomial<FRATag>( std::make_pair(
					std::move( symbols ), std::move( shortened_coefficient )
				) );
			}
			
			auto permuted_coefficient = - std_get_product::second( monomial );
			std::swap( *last_ordered, *( last_ordered + 1 ));
			auto canonicalized_polynomial = canonicalize_monomial<FRATag>( std::make_pair(
				symbols, std::move( permuted_coefficient )
			) );
			
			auto one_ = CoefficientRing::one();
			auto two = one_ + one_;
			auto shortened_coefficient = two * std::move( std_get_product::second( monomial ) ) * std::move( clifford_coefficient );
			symbols.erase( last_ordered, last_ordered + 2 );
			default_r_algebra_t<FRATag>::add_assign( canonicalized_polynomial, canonicalize_monomial<FRATag>( std::make_pair(
				std::move( symbols ), std::move( shortened_coefficient )
			) ) );
			
			return canonicalized_polynomial;
		}
	public:
		template<class FRA> static constexpr FRA &apply( FRA &fra ) {
			auto canonicalized = default_r_algebra_t<tag_of_t<FRA>>::zero();
			for( auto &&monomial : fra.monomials())
				default_r_algebra_t<tag_of_t<FRA>>::add_assign( canonicalized, canonicalize_monomial<tag_of_t<FRA>>( std::move( monomial ) ) );
			fra = std::move( canonicalized );
			return fra;
		}
	};
	
	struct equal {
		template<class FRA1, class FRA2>
		static constexpr decltype( auto ) apply( FRA1 &&fra1, FRA2 &&fra2 ) {
			return ::cxxmath::equal( std::forward<FRA1>( fra1 ), std::forward<FRA2>( fra2 ) );
		}
	};
};
}

#endif //CXXMATH_CLIFFORD_ALGEBRA_HPP
