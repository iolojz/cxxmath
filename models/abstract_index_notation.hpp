//
// Created by jayz on 10.10.19.
//

#ifndef CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP
#define CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP

#include <variant>
#include <boost/range/adaptor/transformed.hpp>

#include "quotient_r_algebra.hpp"

// TODO: Make everything independent on default_...<>

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
			return std::forward<Arg>;
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
	template<class IndecomposableCoefficient, class = std::enable_if_t<
	is_free_r_algebra_tag_v<tag_of_t<IndecomposableCoefficient>> == false>>
	static auto apply( IndecomposableCoefficient &&c )
	{
		return std::vector<std::vector<IndecomposableCoefficient>>{
		std::vector<IndecomposableCoefficient>{ std::forward<IndecomposableCoefficient>( c ) }};
	}
	
	template<class FRA, class = std::enable_if_t<is_free_r_algebra_tag_v<tag_of_t<FRA>>>>
	static auto apply( const FRA &fra )
	{
		using coefficient = decltype( fra.monomials().begin()->second );
		using symbol = decltype( fra.monomials().begin()->first.front());
		
		using atom = typename decltype(apply( std::declval<coefficient>()))::value_type::value_type;
		std::vector<std::vector<typename extend_variant<atom, symbol>::type>> result;
		
		for( const auto &monomial : fra.monomials()) {
			auto decomposed_coefficient = apply( monomial.second );
			for( auto &&part : decomposed_coefficient ) {
				std::vector<typename extend_variant<atom, symbol>::type> new_part;
				new_part.reserve( part.size() + monomial.first.size());
				
				for( auto &&element : part )
					new_part.push_back( extend_variant<atom, symbol>::convert( std::move( element )));
				new_part.insert( new_part.end(), std::begin( monomial.variables ), std::end( monomial.variables ));
				
				result.push_back( std::move( new_part ));
			}
		}
		
		return result;
	}
};

template<class Tag, class ...Alternatives>
struct select_type_with_tag
{
};

template<class Tag, class Type1, class ...Alternatives>
struct select_type_with_tag<Tag, Type1, Alternatives...>
{
	using type = std::conditional_t<std::is_same_v<Tag, tag_of_t<Type1>>, Type1, typename select_type_with_tag<Tag, Alternatives...>::type>;
};

template<class CoefficientTag>
struct coefficient_composer
{
	template<class C, CXXMATH_ENABLE_IF_TAG_IS( C, tag_of_t<CoefficientTag> ) >
	static constexpr decltype( auto ) extract_coefficient( C &&part )
	{ return std::forward<C>( part ); }
	
	template<class ...Alternatives>
	static constexpr decltype( auto ) extract_coefficient( const std::variant<Alternatives...> &part )
	{
		using type = typename select_type_with_tag<CoefficientTag, Alternatives...>::type;
		return std::get<type>( part );
	}
	
	template<class ...Alternatives>
	static constexpr decltype( auto ) extract_coefficient( std::variant<Alternatives...> &&part )
	{
		using type = typename select_type_with_tag<CoefficientTag, Alternatives...>::type;
		return std::get<type>( std::move( part ));
	}

public:
	template<class Range>
	constexpr auto apply( Range &&parts ) const
	{
		auto coefficient = default_ring_t<CoefficientTag>::zero();
		for( auto cit = std::begin( parts ); cit != std::end( parts ); ++cit ) {
			coefficient *= extract_coefficient( *cit );
		}
		return coefficient;
	}
};

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
class coefficient_composer<free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>>
{
	using r_algebra_tag = free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>;
	
	template<class Part>
	static constexpr bool is_symbol( const Part &part )
	{
		if constexpr( std::is_same_v<tag_of_t<Part>, tag_of_t<Symbol>> )
			return true;
		else
			return false;
	}
	
	template<class ...Alternatives>
	static constexpr bool is_symbol( const std::variant<Alternatives...> &v )
	{ return std::visit( []( const auto &part ) { return is_symbol( part ); }, v ); }
	
	template<class C, CXXMATH_ENABLE_IF_TAG_IS( C, tag_of_t<Coefficient> ) >
	static constexpr decltype( auto ) extract_coefficient( C &&part )
	{ return std::forward<C>( part ); }
	
	template<class ...Alternatives>
	static constexpr decltype( auto ) extract_coefficient( const std::variant<Alternatives...> &part )
	{
		using type = typename select_type_with_tag<tag_of_t<Coefficient>, Alternatives...>::type;
		return std::get<type>( part );
	}
	
	template<class ...Alternatives>
	static constexpr decltype( auto ) extract_coefficient( std::variant<Alternatives...> &&part )
	{
		using type = typename select_type_with_tag<tag_of_t<Coefficient>, Alternatives...>::type;
		return std::get<type>( std::move( part ));
	}
	
	template<class S, CXXMATH_ENABLE_IF_TAG_IS( S, tag_of_t<Symbol> ) >
	static constexpr decltype( auto ) extract_symbol( S &&part )
	{ return std::forward<S>( part ); }
	
	template<class ...Alternatives>
	static constexpr decltype( auto ) extract_symbol( const std::variant<Alternatives...> &part )
	{
		using type = typename select_type_with_tag<tag_of_t<Symbol>, Alternatives...>::type;
		return std::get<type>( part );
	}
	
	template<class ...Alternatives>
	static constexpr decltype( auto ) extract_symbol( std::variant<Alternatives...> &&part )
	{
		using type = typename select_type_with_tag<tag_of_t<Symbol>, Alternatives...>::type;
		return std::get<type>( std::move( part ));
	}

public:
	template<class Range>
	static auto apply( Range &&parts )
	{
		auto symbols_begin = std::find_if( std::begin( parts ), std::end( parts ), is_symbol );
		
		auto coefficient = CoefficientRing::zero();
		for( auto cit = std::begin( parts ); cit != symbols_begin; ++cit ) {
			coefficient *= extract_coefficient( *cit );
		}
		
		auto symbol_range = boost::make_iterator_range( std::make_move_iterator( symbols_begin ),
														std::make_move_iterator( std::end( parts ))) |
							boost::adaptors::transformed( []( auto &&part ) {
								return extract_symbol( std::forward<decltype( part )>( part ));
							} );
		
		return ::cxxmath::make<r_algebra_tag>( std::move( coefficient ), std::move( symbol_range ));
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
	
	template<class Range1, class Range2>
	static auto first_index_coincidence( Range1 &&range1, Range2 &&range2, bool disjoint )
	{
		for( auto r1_it = std::begin( range1 ); r1_it != std::end( range1 ); ++r1_it ) {
			auto r1_indices = IndexHandler::extract_indices( *r1_it );
			std::sort( std::begin( r1_indices ), std::end( r1_indices ),
					   function_object_v<typename IndexHandler::less_indices> );
			auto r1_indices_moved = boost::make_iterator_range( std::make_move_iterator( std::begin( r1_indices )),
																std::make_move_iterator( std::end( r1_indices )));
			
			if( disjoint == false ) {
				auto self_intersection = index_intersection( r1_indices_moved );
				if( self_intersection.size() != 0 )
					return std::make_tuple( r1_it, r1_it, std::move( self_intersection ));
			}
			
			for( auto r2_it = disjoint ? std::begin( range2 ) : r1_it + 1; r2_it != std::end( range2 ); ++r2_it ) {
				auto r2_indices = IndexHandler::extract_indices( *r2_it );
				std::sort( r2_indices.begin(), r2_indices.end(),
						   function_object_v<typename IndexHandler::less_indices> );
				auto r2_indices_moved = boost::make_iterator_range( std::make_move_iterator( std::begin( r2_indices )),
																	std::make_move_iterator( std::end( r2_indices )));
				
				auto common_indices = index_intersection( r1_indices_moved, r2_indices_moved );
				
				if( std::size( common_indices ) != 0 )
					return std::make_tuple( r1_it, r2_it, std::move( common_indices ));
			}
		}
		
		using indices_type = decltype( IndexHandler::extract_indices( *std::begin( range1 )));
		using common_indices_type = std::decay_t<decltype( index_intersection( std::declval<indices_type>(),
																			   std::declval<indices_type>()))>;
		return std::make_tuple( std::end( range1 ), std::end( range2 ), common_indices_type{} );
	}
	
	template<class FRA, class SymbolRange>
	static FRA perform_ss_contractions( SymbolRange &&symbols )
	{
		auto index_coincidence = find_index_coincidence( symbols, symbols, false );
		auto common_indices = std::get<2>( index_coincidence );
		
		if( std::size( common_indices ) == 0 )
			return make<tag_of_t<FRA>>( FRA::coefficient_ring::zero(), std::forward<SymbolRange>( symbols ));
		
		auto s1_it = std::make_move_iterator( std::get<0>( index_coincidence ));
		auto s2_it = std::make_move_iterator( std::get<1>( index_coincidence ));
		
		auto middle = ( s1_it == s2_it ) ? boost::make_iterator_range( s1_it, s1_it ) : boost::make_iterator_range(
		std::next( s1_it ), s2_it );
		
		auto contracted = IndexHandler::template contract_indices<tag_of_t<FRA>>::apply( *s1_it, std::move( middle ),
																						 *s2_it,
																						 std::move( common_indices ));
		
		auto head = make<tag_of_t<FRA>>( FRA::coefficient_ring::one(),
										 boost::make_iterator_range( std::make_move_iterator( symbols.begin()),
																	 s1_it ));
		auto tail = make<tag_of_t<FRA>>( FRA::coefficient_ring::one(),
										 boost::make_iterator_range( s2_it, std::make_move_iterator( symbols.end())));
		
		contracted *= std::move( tail );
		head *= std::move( contracted );
		return quotient_map_in_place( head );
	}
	
	template<class FRA, class Coefficient, class SymbolRange>
	static FRA perform_cs_contractions( Coefficient &&c, SymbolRange &&symbols )
	{
		using coefficient = typename FRA::coefficient;
		auto c_decomposed = detail::coefficient_decomposer::apply( std::forward<Coefficient>( c ));
		bool performed_contraction = false;
		
		auto result = default_ring_t<tag_of_t<FRA>>::zero();
		for( auto &&cpart : c_decomposed ) {
			auto index_coincidence = find_index_coincidence( cpart, symbols, true );
			auto common_indices = std::get<2>( index_coincidence );
			
			if( std::size( common_indices ) == 0 ) {
				result += make<tag_of_t<FRA >>( detail::coefficient_composer<coefficient>::apply( std::move( cpart )),
												symbols );
				continue;
			}
			
			performed_contraction = true;
			auto cpart_it = std::make_move_iterator( std::get<0>( index_coincidence ));
			auto s_it = std::make_move_iterator( std::get<1>( index_coincidence ));
			
			auto head = detail::coefficient_composer<coefficient>::apply(
			boost::make_iterator_range( std::make_move_iterator( std::begin( cpart )), cpart_it ));
			std::vector<typename detail::extend_variant<typename decltype(cpart_it)::value_type, typename decltype(s_it)::value_type>::type> middle_parts = {
			std::advance( cpart_it ), std::make_move_iterator( std::end( cpart )) };
			middle_parts.insert( middle_parts.end(), std::begin( symbols ), s_it );
			auto tail = make<tag_of_t<FRA>>( FRA::coefficient_ring::one(),
											 boost::make_iterator_range( s_it, std::end( symbols )));
			
			auto contracted = IndexHandler::template contract_indices<tag_of_t<FRA>>::apply( *cpart_it,
																							 std::move( middle_parts ),
																							 *s_it, std::move(
			common_indices ));
			contracted *= tail;
			default_r_module<tag_of_t<FRA>>::scalar_multiply_in_place( std::move( head ), contracted );
			
			result += std::move( contracted );
		}
		
		if( performed_contraction )
			quotient_map_in_place( result );
		
		return result;
	}
	
	template<class FRA, class C>
	static FRA perform_cc_contractions( C &&c )
	{
		using coefficient = typename FRA::coefficient;
		auto c_decomposed = detail::coefficient_decomposer::apply( std::forward<C>( c ));
		bool performed_contraction = false;
		
		auto result = default_ring_t<tag_of_t<FRA>>::zero();
		for( auto &&cpart : c_decomposed ) {
			auto index_coincidence = find_index_coincidence( cpart, cpart, false );
			auto common_indices = std::get<2>( index_coincidence );
			
			if( std::size( common_indices ) == 0 ) {
				result += make<tag_of_t<FRA>>( detail::coefficient_composer<coefficient>::apply( std::move( cpart )));
				continue;
			}
			
			performed_contraction = true;
			auto c1_it = std::make_move_iterator( std::get<0>( index_coincidence ));
			auto c2_it = std::make_move_iterator( std::get<1>( index_coincidence ));
			
			auto middle = ( c1_it == c2_it ) ? boost::make_iterator_range( c1_it, c1_it ) : boost::make_iterator_range(
			std::next( c1_it ), c2_it );
			
			auto head = detail::coefficient_composer<coefficient>::apply(
			boost::make_iterator_range( std::make_move_iterator( std::begin( cpart )), c1_it ));
			auto contracted = IndexHandler::template contract_indices<tag_of_t<FRA>>::apply( *c1_it,
																							 std::move( middle ),
																							 *c2_it, std::move(
			common_indices ));
			auto tail = detail::coefficient_composer<coefficient>::apply(
			boost::make_iterator_range( c2_it, std::end( cpart )));
			
			default_r_module<tag_of_t<FRA>>::scalar_multiply_in_place( std::move( head ), contracted );
			default_r_module<tag_of_t<FRA>>::scalar_multiply_in_place( std::move( tail ), contracted );
			result += std::move( contracted );
		}
		
		if( performed_contraction )
			quotient_map_in_place( result );
		
		return result;
	}

public:
	using multiplication_is_commutative = impl::false_implementation;
	
	struct quotient_map_in_place
	{
		template<class FRA>
		static FRA &apply( FRA &fra )
		{
			auto contracted = default_ring_t<tag_of_t<FRA>>::zero();
			for( auto &&monomial : fra.monomials())
				contracted += perform_ss_contractions<FRA>( std::move( monomial.first ));
			
			fra = default_ring_t<tag_of_t<FRA>>::zero();
			for( auto &&monomial : contracted.monomials())
				fra += perform_cs_contractions<FRA>( std::move( monomial.second ), std::move( monomial.first ));
			
			contracted = default_ring_t<tag_of_t<FRA>>::zero();
			for( auto &&monomial : fra.monomials())
				fra += perform_cc_contractions<FRA>( std::move( monomial.second ), std::move( monomial.first ));
			
			fra = std::move( contracted );
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
