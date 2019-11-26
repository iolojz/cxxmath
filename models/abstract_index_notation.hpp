//
// Created by jayz on 10.10.19.
//

#ifndef CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP
#define CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP

#include <variant>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/join.hpp>
#include <boost/range/sub_range.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include "quotient_r_algebra.hpp"

#include "../helpers/std_variant.hpp"

namespace cxxmath
{
namespace detail
{
template<class Type1, class Type2>
struct extend_variant
{
	using type = std::variant<Type1, Type2>;
	
	template<class T>
	static constexpr type convert( T &&t )
	{
		return std::forward<T>( t );
	}
};

template<class Type, class ...Args>
struct extend_variant<std::variant<Args...>, Type>
{
	using type = std::variant<Args..., Type>;
private:
	struct converter
	{
		template<class Arg>
		constexpr type operator()( Arg &&arg ) const
		{
			return std::forward<Arg>( arg );
		}
	};

public:
	static constexpr type convert( const std::variant<Args...> &v )
	{
		return std::visit( converter{}, v );
	}
	
	static constexpr type convert( std::variant<Args...> &&v )
	{
		return std::visit( converter{}, std::move( v ));
	}
	
	static constexpr type convert( const Type &t )
	{
		return t;
	}
	
	static constexpr type convert( Type &&t )
	{
		return std::move( t );
	}
};

struct coefficient_decomposer
{
	template<class Arg>
	static auto apply( const Arg &arg ) {
		if constexpr( is_quotient_r_algebra_tag_v<tag_of_t<Arg>> )
			return apply( arg.representative() );
		else if constexpr( is_free_r_algebra_tag_v<tag_of_t<Arg>> ) {
			using coefficient = typename tag_of_t<Arg>::coefficient;
			using symbol = typename tag_of_t<Arg>::symbol;
			
			using atom = typename decltype(apply(std::declval<coefficient>()))::value_type::value_type;
			using value_type = typename extend_variant<atom, symbol>::type;
			std::vector<std::vector<value_type>> result;
			
			for( const auto &monomial : arg.monomials()) {
				auto decomposed_coefficient = apply( monomial.second );
				for( auto &&part : decomposed_coefficient ) {
					std::vector<value_type> new_part;
					new_part.reserve( std::size( part ) + std::size( monomial.first ) );
					
					for( auto &&element : part )
						new_part.push_back( extend_variant<atom, symbol>::convert( std::move( element )));
					new_part.insert( new_part.end(), std::begin( monomial.first ), std::end( monomial.first ));
					
					result.push_back( std::move( new_part ));
				}
			}
			
			return result;
		} else {
			using value_type = std::decay_t<Arg>;
			return std::array<std::array<value_type, 1>, 1>{ std::array<value_type, 1>{ arg } };
		}
	}
};

template<class CoefficientTag>
struct coefficient_composer
{
public:
	template<class Range>
	static constexpr auto apply( Range &&parts )
	{
		if( std::size( parts ) != 1 )
			throw std::runtime_error( "coefficient_composer: given range does not have precisely one part" );
		
		auto &&cpart = *std::begin( parts );
		using cpart_t = std::decay_t<decltype(cpart)>;
		
		if constexpr( is_std_variant_v<cpart_t> ) {
			return std::get<select_type_with_tag_t<CoefficientTag, tag_of_t<cpart_t>>>( cpart );
		} else {
			if( std::is_same_v<CoefficientTag, tag_of_t<cpart_t>> == false )
				throw std::runtime_error( "coefficient_composer: tag mismatch in given range" );
			
			return cpart;
		}
	}
};

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
class coefficient_composer<free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>>
{
	using r_algebra_tag = free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>;
	
	template<class Part> static constexpr bool is_symbol( const Part &part ) {
		if constexpr( is_std_variant_v<std::decay_t<Part>> )
			return std::visit( []( const auto &p ) { return is_symbol( p ); }, part );
		else
			return std::is_same_v<tag_of_t<Part>, tag_of_t<Symbol>>;
	}
	
	template<class S> static Symbol extract_symbol( S &&part )
	{
		if constexpr( is_std_variant_v<std::decay_t<S>> )
			return std::visit( []( auto &&p ) { return extract_symbol( std::forward<decltype(p)>( p ) ); }, std::forward<S>( part ) );
		else if constexpr( std::is_same_v<tag_of_t<S>, tag_of_t<Symbol>> )
			return std::forward<S>( part );
		else
			throw std::runtime_error( "attempt to extract symbol from coefficient" );
	}
public:
	template<class Range>
	static decltype(auto) apply( Range &&parts )
	{
		auto rend_symbols = std::find_if(
			std::make_reverse_iterator( std::end( parts ) ),
			std::make_reverse_iterator( std::begin( parts ) ),
			[] ( const auto &part ) { return !is_symbol( part ); }
		);
		auto begin_symbols = rend_symbols.base();
		
		auto cparts = boost::sub_range<std::decay_t<Range>>( std::begin( parts ), begin_symbols );
		auto symbols = boost::sub_range<std::decay_t<Range>>( begin_symbols, std::end( parts ) ) |
			boost::adaptors::transformed( []( auto &&p ) {
			return extract_symbol( std::forward<decltype(p)>( p ) );
		} );
		
		if( std::size( cparts ) == 0 )
			return make<r_algebra_tag>( CoefficientRing::one(), std::move( symbols ) );
		
		auto coefficient = coefficient_composer<tag_of_t<Coefficient>>::apply( std::move( cparts ) );
		return make<r_algebra_tag>( std::move( coefficient ), std::move( symbols ) );
	}
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
class coefficient_composer<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>>
{
public:
	template<class Range> static auto apply( Range &&parts )
	{
		return make<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>>(
			coefficient_composer<FreeRAlgebraTag>::apply( std::forward<Range>( parts ) )
		);
	}
};
}

template<class IndexHandler>
class abstract_index_quotient_spec
{
	template<class SortedIndexRange1, class SortedIndexRange2>
	static auto index_intersection( SortedIndexRange1 &&si1, SortedIndexRange2 &&si2 )
	{
		using index_type = std::common_type_t<typename std::decay_t<SortedIndexRange1>::value_type, typename std::decay_t<SortedIndexRange2>::value_type>;
		std::vector<index_type> common_indices;
		
		std::set_intersection( std::begin( si1 ), std::end( si1 ), std::begin( si2 ), std::end( si2 ),
							   std::back_inserter( common_indices ),
							   function_object_v<typename IndexHandler::less_indices> );
		
		return common_indices;
	}
	
	template<class SortedIndexRange>
	static auto index_intersection( SortedIndexRange &&si )
	{
		std::vector<typename decltype(std::begin( si ))::value_type> indices;
		
		auto it = std::begin( si );
		while(( it = std::adjacent_find( it, std::end( si ))) != std::end( si )) {
			indices.push_back( *it );
			it = std::next( it, 2 );
		}
		
		return indices;
	}
	
	template<bool disjoint, class IteratorPair, class Range1, class Range2>
	static IteratorPair &advance_iterator_pair( IteratorPair &iterator_pair, Range1 &&r1, Range2 &&r2 ) {
		if( ++(iterator_pair.second) == std::end( r2 ) ) {
			++(iterator_pair.first);
			
			if constexpr( disjoint ) {
				if( iterator_pair.first == std::end( r1 ) )
					iterator_pair.second = std::end( r2 );
				else
					iterator_pair.second = std::begin( r2 );
			}
			else
				iterator_pair.second = iterator_pair.first;
		}
		
		return iterator_pair;
	};
	
	template<bool disjoint, class IteratorPair, class Range1, class Range2>
	static auto find_index_coincidence( IteratorPair begin, Range1 &&range1, Range2 &&range2 )
	{
		using indices_type = decltype( IndexHandler::extract_indices::apply( *std::begin( range1 )));
		using common_indices_type = std::decay_t<decltype( index_intersection( std::declval<indices_type>(),
																			   std::declval<indices_type>()))>;
		
		if( begin.first == std::end( range1 ) || begin.second == std::end( range2 ) )
			return std::make_tuple( std::end( range1 ), std::end( range2 ), common_indices_type{} );
		
		auto end = std::make_pair( std::end( range1 ), std::end( range2 ) );
		do {
			auto r1_indices = IndexHandler::extract_indices::apply( *(begin.first) );
			std::sort( std::begin( r1_indices ), std::end( r1_indices ),
					   function_object_v<typename IndexHandler::less_indices> );
			auto r1_indices_moved = boost::make_iterator_range( std::make_move_iterator( std::begin( r1_indices )),
																std::make_move_iterator( std::end( r1_indices )));
			
			if constexpr( disjoint == false ) {
				if( begin.first == begin.second ) {
					auto self_intersection = index_intersection( r1_indices_moved );
					if( self_intersection.size() != 0 )
						return std::make_tuple( begin.first, begin.first, std::move( self_intersection ));
					
					continue;
				}
			}
			
			auto r2_indices = IndexHandler::extract_indices::apply( *(begin.second) );
			std::sort( r2_indices.begin(), r2_indices.end(),
					   function_object_v<typename IndexHandler::less_indices> );
			auto r2_indices_moved = boost::make_iterator_range( std::make_move_iterator( std::begin( r2_indices )),
																std::make_move_iterator( std::end( r2_indices )));
			
			auto common_indices = index_intersection( r1_indices_moved, r2_indices_moved );
			
			if( std::size( common_indices ) != 0 )
				return std::make_tuple( begin.first, begin.second, std::move( common_indices ));
		} while( advance_iterator_pair<disjoint>( begin, std::forward<Range1>( range1 ), std::forward<Range2>( range2 ) ) != end );
		
		return std::make_tuple( std::end( range1 ), std::end( range2 ), common_indices_type{} );
	}
	
	template<class FRA, class SymbolRange>
	static std::optional<FRA> perform_ss_contractions( SymbolRange &&symbols )
	{
		auto begin = std::make_pair( std::begin( symbols ), std::begin( symbols ) );
		auto end = std::make_pair( std::end( symbols ), std::end( symbols ) );
		
		auto advance = [&symbols] ( auto &iterator_pair ) {
			return advance_iterator_pair<false>( iterator_pair, symbols, symbols );
		};
		
		for( auto current = begin; current.first != end.first; advance( current ) ) {
			auto index_coincidence = find_index_coincidence<false>( current, symbols, symbols );
			auto common_indices = std::get<2>( index_coincidence );
			
			if( std::size( common_indices ) == 0 )
				break;
			
			current = std::make_pair( std::get<0>( index_coincidence ), std::get<1>( index_coincidence ) );
			
			auto s1_it = current.first;
			auto s2_it = current.second;
			
			std::optional<FRA> contracted;
			if( s1_it == s2_it ) {
				contracted = IndexHandler::template contract_indices<tag_of_t<FRA>>::apply( *s1_it, std::move( common_indices ) );
			} else {
				auto middle = boost::make_iterator_range( std::next( s1_it ), s2_it );
				contracted = IndexHandler::template contract_indices<tag_of_t<FRA>>::apply( *s1_it, std::move( middle ), *s2_it, std::move( common_indices ) );
			}
			
			if( contracted == std::nullopt )
				continue;
			
			auto head = make<tag_of_t<FRA>>( FRA::coefficient_ring::one(),
											 boost::make_iterator_range( std::begin( symbols ), s1_it ));
			auto tail = make<tag_of_t<FRA>>( FRA::coefficient_ring::one(),
											 boost::make_iterator_range( ++s2_it, std::end( symbols ) ) );
			
			cxxmath::multiply_assign( head, std::move( *contracted ) );
			cxxmath::multiply_assign( head, std::move( tail ) );
			return quotient_map_in_place::apply( head );
		}
		
		return std::nullopt;
	}
	
	template<class FRA, class CPartRange, class SymbolRange>
	static std::optional<FRA> perform_cparts_contractions( CPartRange &&cparts, SymbolRange &&symbols )
	{
		auto begin = std::make_pair( std::begin( cparts ), std::begin( symbols ) );
		
		auto advance = [&symbols, &cparts] ( auto &iterator_pair ) {
			return advance_iterator_pair<true>( iterator_pair, cparts, symbols );
		};
		
		for( auto current = begin; current.first != std::end( cparts ); advance( current ) ) {
			auto index_coincidence = find_index_coincidence<true>( current, cparts, symbols );
			auto common_indices = std::get<2>( index_coincidence );
			
			if( std::size( common_indices ) == 0 )
				break;
			
			current = std::make_pair( std::get<0>( index_coincidence ), std::get<1>( index_coincidence ) );
			
			auto cpart_it = current.first;
			auto s_it = current.second;
			
			auto middle = boost::make_iterator_range( std::next( cpart_it ), std::end( cparts ) );
			auto preceeding_symbols = boost::make_iterator_range( std::begin( symbols ), s_it );
			
			auto contracted = IndexHandler::template contract_indices<tag_of_t<FRA>>::apply( *cpart_it, std::move( middle ), *s_it, std::move( common_indices ));
			if( contracted == std::nullopt )
				continue;
			
			auto coefficient = detail::coefficient_composer<tag_of_t<typename FRA::coefficient>>::apply(
				boost::make_iterator_range( std::begin( cparts ), cpart_it )
			);
			auto head = make<tag_of_t<FRA>>( std::move( coefficient ), std::move( preceeding_symbols ) );
			auto tail = make<tag_of_t<FRA>>( FRA::coefficient_ring::one(),
											 boost::make_iterator_range( ++s_it, std::end( symbols ) ) );
			
			cxxmath::multiply_assign( head, std::move( *contracted ) );
			cxxmath::multiply_assign( head, std::move( tail ) );
			
			return quotient_map_in_place::apply( head );
		}
		
		return std::nullopt;
	}
	
	template<class FRA, class Coefficient, class SymbolRange>
	static std::optional<FRA> perform_cs_contractions( Coefficient &&c, SymbolRange &&symbols )
	{
		static_assert( std::is_rvalue_reference_v<Coefficient &&> );
		auto c_decomposed = detail::coefficient_decomposer::apply( c );
		
		std::vector<std::decay_t<decltype(*std::begin(symbols))>> symbol_copies;
		boost::range::copy( symbols, std::back_inserter( symbol_copies ) );
		
		std::optional<FRA> contraction_result;
		auto contraction_it = std::find_if( std::begin( c_decomposed ), std::end( c_decomposed ),
				[&] ( auto &&cpart_range ) {
			contraction_result = perform_cparts_contractions<FRA>( std::move( cpart_range ), symbol_copies );
			return contraction_result.has_value();
		} );
		
		if( contraction_it == std::end( c_decomposed ) )
			return std::nullopt;
		
		using monomial = typename FRA::monomial_container::value_type;
		auto monomial_from_cpart = [&] ( auto &&cpart_range ) {
			return monomial{symbol_copies, detail::coefficient_composer<tag_of_t<typename FRA::coefficient>>::apply(
					boost::make_iterator_range(std::make_move_iterator(std::begin(cpart_range)),
											   std::make_move_iterator(std::end(cpart_range))))};
		};
		
		std::vector<monomial> new_monomials;
		std::transform( std::begin( c_decomposed ), contraction_it,
				std::back_inserter( new_monomials ), monomial_from_cpart
		);
		
		{
			const auto &contracted_monomials = contraction_result->monomials();
			new_monomials.insert(new_monomials.end(), std::make_move_iterator(std::begin(contracted_monomials)),
								 std::make_move_iterator(std::end(contracted_monomials)));
		}
		
		while( ++contraction_it != std::end( c_decomposed ) ) {
			contraction_result = perform_cparts_contractions<FRA>( std::move( *contraction_it ), symbol_copies );
			
			if( contraction_result ) {
				const auto &contracted_monomials = contraction_result->monomials();
				new_monomials.insert(new_monomials.end(), std::make_move_iterator(std::begin(contracted_monomials)),
									 std::make_move_iterator(std::end(contracted_monomials)));
			} else
				new_monomials.push_back( monomial_from_cpart( std::move( *contraction_it ) ) );
		}
		
		return make<tag_of_t<FRA>>( std::make_move_iterator( new_monomials.begin() ), std::make_move_iterator( new_monomials.end() ) );
	}
public:
	using multiplication_is_commutative = impl::false_implementation;
	
	struct quotient_map_in_place
	{
		template<class FRA>
		static FRA &apply( FRA &fra )
		{
			static_assert( is_std_variant_v<typename tag_of_t<FRA>::symbol> == false, "Abstract index notation cannot work with std::variants" );
			static_assert( is_std_variant_v<typename tag_of_t<FRA>::coefficient> == false, "Abstract index notation cannot work with std::variants" );
			
			std::vector<typename FRA::monomial_container::value_type> new_monomials;
			for( auto monomial = fra.monomials().begin(); monomial != fra.monomials().end(); ++monomial ) {
				auto contraction_result = perform_ss_contractions<FRA>(
					// FIXME: It would be nice, if we could use move_iterators here...
					boost::make_iterator_range( std::begin( monomial->first ), std::end( monomial->first ) )
				);
				
				if( contraction_result ) {
					FRA::scalar_multiply_assign::apply( std::move( monomial->second ), *contraction_result );
					
					const auto &contracted_monomials = contraction_result->monomials();
					new_monomials.insert( new_monomials.end(), std::make_move_iterator( contracted_monomials.begin() ), std::make_move_iterator( contracted_monomials.end() ) );
				} else
					new_monomials.push_back( std::move( *monomial ) );
			}
			
			fra = make<tag_of_t<FRA>>( std::make_move_iterator( new_monomials.begin() ), std::make_move_iterator( new_monomials.end() ) );
			new_monomials.clear();
			
			for( auto monomial = fra.monomials().begin(); monomial != fra.monomials().end(); ++monomial ) {
				auto contraction_result = perform_cs_contractions<FRA>( std::move( monomial->second ), std::move( monomial->first ) );
				
				if( contraction_result ) {
					const auto &contracted_monomials = contraction_result->monomials();
					new_monomials.insert( new_monomials.end(), std::make_move_iterator( contracted_monomials.begin() ), std::make_move_iterator( contracted_monomials.end() ) );
				} else
					new_monomials.push_back( std::move( *monomial ) );
			}
			
			fra = make<tag_of_t<FRA>>( std::make_move_iterator( new_monomials.begin() ), std::make_move_iterator( new_monomials.end() ) );
			return fra;
		}
	};
	
	struct negate_in_place : commutes_with_quotient_map_helper<true>
	{
		template<class FRA>
		static constexpr decltype( auto ) apply( FRA &&fra )
		{
			return ::cxxmath::negate_in_place( std::forward<FRA>( fra ));
		}
	};
	
	struct scalar_multiply_assign : commutes_with_quotient_map_helper<false>
	{
		template<class Scalar, class FRA>
		static constexpr decltype( auto ) apply( Scalar &&s, FRA &fra )
		{
			::cxxmath::scalar_multiply_assign( std::forward<Scalar>( s ), fra );
			return quotient_map_in_place::apply( fra );
		}
	};
	
	struct add_assign : commutes_with_quotient_map_helper<true>
	{
		template<class FRA1, class FRA2>
		static constexpr decltype( auto ) apply( FRA1 &&fra1, FRA2 &&fra2 )
		{
			return ::cxxmath::add_assign( std::forward<FRA1>( fra1 ), std::forward<FRA2>( fra2 ));
		}
	};
	
	struct multiply_assign : commutes_with_quotient_map_helper<false>
	{
		template<class FRA1, class FRA2>
		static constexpr decltype( auto ) apply( FRA1 &fra1, FRA2 &&fra2 )
		{
			::cxxmath::multiply_assign( fra1, std::forward<FRA2>( fra2 ));
			return quotient_map_in_place::apply( fra1 );
		}
	};
	
	struct equal
	{
		template<class FRA1, class FRA2>
		static constexpr decltype( auto ) apply( FRA1 &&fra1, FRA2 &&fra2 )
		{
			return ::cxxmath::equal( std::forward<FRA1>( fra1 ), std::forward<FRA2>( fra2 ));
		}
	};
};
}

#endif //CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP
