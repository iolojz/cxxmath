//
// Created by jayz on 07.10.19.
//

#ifndef CXXMATH_MODELS_FREE_R_MODULE_HPP
#define CXXMATH_MODELS_FREE_R_MODULE_HPP

#include "free_monoid.hpp"
#include "product_monoid.hpp"
#include "std_get.hpp"
#include "function_object.hpp"

#include "core/is_range.hpp"

#include "concepts/r_module.hpp"

#include <boost/range/combine.hpp>
#include <boost/container/flat_map.hpp>

namespace cxxmath {
template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
struct free_r_algebra_tag {};

namespace detail {
template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
struct free_r_algebra {
	using dispatch_tag = free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>;
private:
	using symbol_monoid = free_monoid<Symbol>;
	using monomial_monoid = product_monoid<std_get_product, symbol_monoid, typename CoefficientRing::monoid_>;
	
	struct less_variables
	{
		template<class V1, class V2>
		decltype( auto ) operator()( V1 &&v1, V2 &&v2 ) const
		{
			return SymbolTotalOrder::less( std::forward<V1>( v1 ), std::forward<V2>( v2 ));
		}
	};
	
	struct less_variable_ranges
	{
		template<class Range1, class Range2>
		bool operator()( Range1 &&r1, Range2 &&r2 ) const
		{
			return std::lexicographical_compare( std::begin( r1 ), std::end( r1 ), std::begin( r2 ), std::end( r2 ),
												 less_variables{} );
		}
	};
	
	void strip_zeros( void )
	{
		erase_if( monomial_map, []( const auto &monomial ) {
			return equal_Coefficients( std_get_product::second( monomial ), CoefficientRing::zero() );
		} );
	}
	
	// FIXME: Should we enable runtime switching of this type?
	using monomial_container = boost::container::flat_map<std::vector<Symbol>, Coefficient, less_variable_ranges>;
	//using monomial_container = std::map<std::vector<variable>, Coefficient, less_variable_ranges>;
	monomial_container monomial_map;
	
	free_r_algebra( monomial_container &&mm )
	: monomial_map{ std::move( mm ) }
	{};
	
	free_r_algebra &operator=( monomial_container &&mm )
	{ monomial_map = std::move( mm ); }
	
	monomial_container &monomials() & { return monomial_map; }
	monomial_container monomials() && { return std::move( monomial_map ); }
public:
	const monomial_container &monomials() const & { return monomial_map; }
	
	free_r_algebra() = default;
	
	free_r_algebra( const free_r_algebra & ) = default;
	
	free_r_algebra( free_r_algebra && ) = default;
	
	free_r_algebra &operator=( const free_r_algebra & ) = default;
	
	free_r_algebra &operator=( free_r_algebra && ) = default;
	
	struct zero {
		static decltype(auto) apply() {
			return free_r_algebra{};
		}
	};
	
	struct negate_in_place {
		template<class FRA>
		static FRA &apply( FRA &fra ) {
			auto &monomial_map = fra.monomials();
			for( auto &monomial : monomial_map )
				CoefficientRing::negate_in_place( monomial.second );
			return fra;
		}
	};
	
	struct scalar_multiply_assign {
		template<class C, class FRA>
		static FRA &apply( C &&c, FRA &fra ) {
			auto &monomial_map = fra.monomials();
			for( auto &monomial : monomial_map )
				CoefficientRing::multiply_assign( monomial.second, c );
			fra.strip_zeros();
			return fra;
		}
	};
	
	struct add_assign {
		template<class FRA1, class FRA2>
		static FRA1 &apply( FRA1 &fra1, FRA2 &&fra2 ) {
			auto &monomial_map1 = fra1.monomials();
			
			if( std::addressof(fra1) == std::addressof(fra2) ) {
				for( auto &term : monomial_map1 )
					CoefficientRing::add_assign( term.second, term.second );
				
				return fra1;
			}
			
			const auto &monomial_map2 = fra2.monomials();
			
			for( const auto &term : monomial_map2 ) {
				auto insertion_result = monomial_map1.insert( term );
				if( insertion_result.second == false ) {
					CoefficientRing::add_assign( insertion_result.first->second, term.second );
					if( CoefficientSet::equal( insertion_result.first->second, CoefficientRing::zero() ) )
						monomial_map1.erase( insertion_result.first );
				}
			}
			
			return fra1;
		}
	};
	
	struct multiply_assign {
		template<class FRA1, class FRA2>
		static FRA1 &apply( FRA1 &fra1, FRA2 &&fra2 ) {
			// FIXME: do not take cartesian product for simple multiplications!
			auto &monomial_map1 = fra1.monomials();
			auto &&monomial_map2 = fra2.monomials();
			
			std::vector<typename monomial_container::value_type> monomials;
			for( const auto &factor1 : monomial_map1 ) {
				for( const auto &factor2 : monomial_map2 ) {
					monomials.push_back( monomial_monoid::compose( factor1, factor2 ));
				}
			}
			
			monomial_container result;
			for( auto &&monomial : monomials ) {
				if( equal_Coefficients( monomial.second, CoefficientRing::zero()))
					continue;
				
				auto insertion_result = result.insert( monomial );
				if( insertion_result.second == false ) {
					add_assign_Coefficients( insertion_result.first->second, std::move( monomial.second ));
					if( equal_Coefficients( insertion_result.first->second, CoefficientRing::zero()))
						result.erase( insertion_result.first );
				}
			}
			
			fra1 = std::move( result );
			return fra1;
		}
	};
	
	class make {
		template<class MonoidElement, class C>
		static constexpr void make_element( MonoidElement &&m, C &&c ) {
			monomial_container monomial_map = { std::forward<MonoidElement>( m ), CoefficientRing::one() };
			
			using scalar_multiply = concepts::detail::scalar_multiply<scalar_multiply_assign>;
			return scalar_multiply::apply( std::forward<C>( c ), std::move( monomial_map ) );
		}
		
		template<class MonoidElement>
		static constexpr void multiply_assign_( MonoidElement &m ) {}
		
		template<class MonoidElement1, class MonoidElement2, class ...Tail>
		static constexpr void multiply_assign_( MonoidElement1 &m1, MonoidElement2 &&m2, Tail &&...tail ) {
			multiply_assign_( symbol_monoid::multiply_assign( m1, std::forward<MonoidElement2>( m2 ) ), std::forward<Tail>( tail )... );
		}
		
		template<class Product, class FRA>
		static constexpr void add_assign_( FRA &fra ) {}
		
		template<class Product, class FRA1, class ProductElement, class ...Tail>
		static constexpr void multiply_assign_( FRA1 &fra1, ProductElement &&p, Tail &&...tail ) {
			auto next_summand = make_element(
				Product::first( std::forward<ProductElement>( p ) ),
				Product::second( std::forward<ProductElement>( p ) )
			);
			add_assign_( add_assign::apply( fra1, std::move( next_summand ) ), std::forward<Tail>( tail )... );
		}
	public:
		template<class C, class ...Symbols, CXXMATH_ENABLE_IF_TAG_IS(C, tag_of_t<Coefficient>),
		class = std::enable_if_t<!is_range_v<Symbols>>>>
		static decltype(auto) apply( C &&c, Symbols &&...symbols ) {
			auto monoid_element = symbol_monoid::one();
			multiply_assign_( monoid_element, std::forward<Symbols>( symbols )... );
			return make_element( std::move( monoid_element, std::forward<C>( c ) ) );
		}
		
		template<class C, class Range, CXXMATH_ENABLE_IF_TAG_IS(C, tag_of_t<Coefficient>),
		class = std::enable_if_t<is_range_v<Range>>>
		static decltype(auto) apply( C &&c, Range &&r ) {
			auto first = std::begin( r );
			auto last = std::end( r );
			
			auto monoid_element = symbol_monoid::one();
			while( first != last ) {
				multiply_assign_( monoid_element, *first );
				++first;
			}
			return make_element( std::move( monoid_element, std::forward<C>( c ) ) );
		}
		
		template<class FreeRAlgebra, CXXMATH_ENABLE_IF_TAG_IS(FreeRAlgebra, dispatch_tag)>
		static decltype(auto) apply( FreeRAlgebra &&fra ) {
			return free_r_algebra{ std::forward<FreeRAlgebra>( fra ) };
		}
		
		template<class Product = std_get_product, class Product1, class ...Products,
				class = std::enable_if_t<models_concept_v<Product1, Product>>>
		static decltype(auto) apply( Product1 &&p1, Products &&...products ) {
			auto result = make_element(
				Product::first( std::forward<Product1>( p1 ) ),
				Product::second( std::forward<Product1>( p1 ) )
			);
			add_assign_<Product>( result, std::forward<Products>( products )... );
			return result;
		}
	};
	
	struct one {
		static decltype(auto) apply() {
			return make::apply( CoefficientRing::one() );
		}
	};
	
	struct equal {
		static bool apply( const free_r_algebra &fra1, const free_r_algebra &fra2 )
		{
			const auto &monomial_map1 = fra1.monomials();
			const auto &monomial_map2 = fra2.monomials();
			
			if( monomial_map1.size() != monomial_map2.size())
				return false;
			
			for( const auto &monomials : boost::combine( monomial_map1, monomial_map2 )) {
				const auto &variables1 = boost::get<0>( monomials ).first;
				const auto &variables2 = boost::get<1>( monomials ).first;
				
				if( variables1.size() != variables2.size())
					return false;
				
				for( const auto &variables : boost::combine( variables1, variables2 )) {
					if( SymbolTotalOrder::not_equal( boost::get<0>( variables ), boost::get<1>( variables )))
						return false;
				}
				
				if( CoefficientSet::not_equal( boost::get<0>( monomials ).second, boost::get<1>( monomials ).second ))
					return false;
			}
			
			return true;
		}
	};
};
}

template<class Coefficient, class Symbol,
		class CoefficientSet = default_set_t<tag_of_t<Coefficient>>,
		class CoefficientRing = default_ring_t<tag_of_t<Coefficient>>,
		class SymbolTotalOrder = default_total_order_t<tag_of_t<Symbol>>>
struct free_r_algebra_monoid {
private:
	using value_type = detail::free_r_algebra<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>;
public:
	using type = concepts::assignable_monoid<typename value_type::multiply_assign, impl::false_implementation, typename value_type::one>;
};

template<class Coefficient, class Symbol,
		class CoefficientSet = default_set_t<tag_of_t<Coefficient>>,
		class CoefficientRing = default_ring_t<tag_of_t<Coefficient>>,
		class SymbolTotalOrder = default_total_order_t<tag_of_t<Symbol>>>
struct free_r_algebra_group {
private:
	using value_type = detail::free_r_algebra<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>;
public:
	using type = concepts::assignable_group<
			concepts::assignable_monoid<typename value_type::add_assign, impl::true_implementation, typename value_type::zero>,
			typename value_type::negate_in_place
	>;
};

template<class Coefficient, class Symbol,
		class CoefficientSet = default_set_t<tag_of_t<Coefficient>>,
		class CoefficientRing = default_ring_t<tag_of_t<Coefficient>>,
		class SymbolTotalOrder = default_total_order_t<tag_of_t<Symbol>>>
struct free_r_algebra_ring {
	using type = concepts::ring<
			free_r_algebra_group<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>,
			free_r_algebra_monoid<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>
	>;
};

template<class Coefficient, class Symbol,
		class CoefficientSet = default_set_t<tag_of_t<Coefficient>>,
		class CoefficientRing = default_ring_t<tag_of_t<Coefficient>>,
		class SymbolTotalOrder = default_total_order_t<tag_of_t<Symbol>>>
using free_r_module = concepts::assignable_r_module<
	typename free_r_algebra_group<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>::type,
	typename detail::free_r_algebra<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>::scalar_multiply_assign
>;

template<class Coefficient, class Symbol,
		class CoefficientSet = default_set_t<tag_of_t<Coefficient>>,
		class CoefficientRing = default_ring_t<tag_of_t<Coefficient>>,
		class SymbolTotalOrder = default_total_order_t<tag_of_t<Symbol>>>
using free_r_algebra = concepts::r_algebra<
	typename free_r_module<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>::type,
	typename free_r_algebra_monoid<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>::type
>;

namespace impl {
template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
struct default_monoid<free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>> {
using type = typename free_r_algebra_monoid<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>::type;
};

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
struct default_group<free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>> {
using type = typename free_r_algebra_group<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>::type;
};

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
struct default_ring<free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>> {
using type = typename free_r_algebra_ring<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>::type;
};

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
struct default_r_module<free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>> {
using type = typename free_r_module<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>::type;
};

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
struct default_r_algebra<free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>> {
using type = typename free_r_algebra<Coefficient, CoefficientSet, CoefficientRing, SymbolTotalOrder>::type;
};
}

template<class Coefficient, class Symbol,
		class CoefficientSet = default_set_t<tag_of_t<Coefficient>>,
		class CoefficientRing = default_ring_t<tag_of_t<Coefficient>>,
		class SymbolTotalOrder = default_total_order_t<tag_of_t<Symbol>>>
static constexpr typename detail::free_r_algebra<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>::make make_free_r_algebra;
}

#endif //CXXMATH_MODELS_FREE_R_MODULE_HPP
