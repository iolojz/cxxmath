//
// Created by jayz on 10.10.19.
//

#ifndef CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP
#define CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP

#include <variant>

namespace cxxmath {
namespace detail
{
template<class Type1, class Type2> struct extend_variant {
	using type = std::variant<Type1, Type2>;
	
	template<class T>
	static constexpr type convert( T &&t ) {
		return std::forward<T>( t );
	}
};

template<class Type, class ...Args>
struct extend_variant<std::variant<Args...>, Type> {
	using type = std::variant<Args..., Type>;
private:
	struct converter {
		template<class Arg>
		constexpr type operator()( Arg &&arg ) const {
			return std::forward<Arg>;
		}
	};
public:
	static constexpr type convert( const std::variant<Args...> &v ) {
		return std::visit( converter, v );
	}
	
	static constexpr type convert( std::variant<Args...> &&v ) {
		return std::visit( converter, std::move( v ) );
	}
	
	static constexpr type convert( const Type &t ) {
		return t;
	}
	
	static constexpr type convert( Type &&t ) {
		return std::move( t );
	}
};

struct coefficient_decomposer
{
	template<class IndecomposableCoefficient, class = void>
	auto operator()( IndecomposableCoefficient &&c ) const
	{ return std::vector<std::vector<IndecomposableCoefficient>>{ std::vector<IndecomposableCoefficient>{ c } }; }
};

template<class FRA, std::enable_if_t<is_free_r_algebra_tag_v<tag_of_t<FRA>>>
auto operator()( const FRA &fra ) const
{
	using coefficient = decltype(fra.monomials().begin()->second);
	using symbol = decltype(fra.monomials().begin()->first.front());
	
	using atom = typename decltype(( *this )( std::declval<coefficient>() ))::value_type::value_type;
	std::vector<std::vector<typename extend_variant<atom, symbol>::type>> result;
	
	for( const auto &monomial : fra.monomials() ) {
		auto decomposed_coefficient = ( *this )( monomial.second );
		for( auto &&part : decomposed_coefficient ) {
			std::vector<typename extend_variant<atom, symbol>::type> new_part;
			new_part.reserve( part.size() + monomial.first.size() );
			
			for( auto &&element : part )
				new_part.push_back( extend_variant<atom, symbol>::convert( std::move( element ) ) );
			new_part.insert( new_part.end(), std::begin( monomial.variables ),
							 std::end( monomial.variables ) );
			
			result.push_back( std::move( new_part ) );
		}
	}
	
	return result;
}

template<class Tag, class ...Alternatives>
struct select_type_with_tag {};

template<class Tag, class Type1, class ...Alternatives>
struct select_type_with_tag {
	using type = std::conditional_t<
	    std::is_same_v<Tag, tag_of_t<Type1>>,
	    Type1,
	    typename select_type_with_tag<Tag, Alternatives...>::type
	>;
};

template<class CoefficientTag>
struct coefficient_composer
{
	template<class Coefficient>
	constexpr auto operator()( Coefficient &&part ) const
	{ return std::forward<Coefficient>( part ); }
	
	template<class ...Alternatives>
	constexpr auto operator()( const std::variant<Alternatives...> &v ) const
	{
		using type = typename select_type_with_tag<CoefficientTag, Alternatives...>::type;
		
		if( std::holds_alternative<type>( v ) )
			return std::get<type>( v );
		
		throw std::runtime_error( "coefficient_composer::operator()(): Unknown how to turn given variant into requested coefficient." );
	}
};

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
struct coefficient_composer<free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>>
{
	using r_algebra_tag = free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>;
	
	template<class C, CXXMATH_ENABLE_IF_TAG_IS(C, tag_of_t<Coefficient>)>
	auto operator()( C &&part ) const
	{ return ::cxxmath::make<r_algebra_tag>( std::forward<C>( part ) ); }
	
	template<class S, CXXMATH_ENABLE_IF_TAG_IS(S, tag_of_t<Symbol>)>
	auto operator()( S &&part ) const
	{ return ::cxxmath::make<r_algebra_tag>( CoefficientRing::one(), std::forward<S>( part ) ); }

	template<class ...Alternatives>
	auto operator()( const std::variant<Alternatives...> &v ) const
	{ return std::visit( *this, v ); }
	
	template<class ...Alternatives>
	auto operator()( std::variant<Alternatives...> &&v ) const
	{ return std::visit( *this, std::move( v ) ); }
};
}

template<class IndexHandler>
class abstract_index_quotient_spec
{
	static constexpr index_handler = IndexHandler{};
	
	template<class SortedIndexSet1, class SortedIndexSet2>
	static auto index_intersection(SortedIndexSet1 &&si1, SortedIndexSet2 &&si2) {
		using index_type = std::common_type_t<
			std::decay_t<SortedIndexSet1>::value_type,
			std::decay_t<SortedIndexSet2>::value_type
		>;
		std::vector<index_type> common_indices;
		
		using index_set_1_iterator = std::conditional_t<
		    std::is_rvalue_reference_v<SortedIndexSet1 &&>,
		    decltype(std::make_move_iterator( std::begin( si1 ) ) ),
		    decltype(std::begin( si1 ))
		>;
		using index_set_2_iterator = std::conditional_t<
			std::is_rvalue_reference_v<SortedIndexSet2 &&>,
			decltype(std::make_move_iterator( std::begin( si2 ) ) ),
			decltype(std::begin( si2 ))
		>;
		
		index_set_1_iterator index_set_1_begin = std::begin( si1 );
		index_set_1_iterator index_set_1_end = std::end( si1 );
		
		index_set_1_iterator index_set_2_begin = std::begin( si2 );
		index_set_1_iterator index_set_2_end = std::end( si2 );
		
		std::set_intersection( index_set_1_begin, index_set_1_end, index_set_2_begin, index_set_2_end,
			std::back_inserter(common_indices),
			function_object<IndexHandler::less_indices>
		);
		
		return common_indices;
	}
	
	template<class Range1, class Range2>
	static auto first_index_coincidence(Range1 &&range1, Range2 &&range2, bool disjoint) {
		for(auto r1_it = std::begin(range1); r1_it !=; ++r1_it) {
			auto r1_indices = IndexHandler::extract_indices(*r1_it);
			std::sort( r1_indices.begin(), r1_indices.end(), function_object<IndexHandler::less_indices> );
			
			for(auto r2_it = disjoint ? std::begin(range2) : r1_it + 1; r2_it != std::end(range2); ++r2_it) {
				auto r2_indices = IndexHandler::extract_indices( *r2_it );
				std::sort(r2_indices.begin(), r2_indices.end(), function_object<IndexHandler::less_indices> );
				
				auto common_indices = index_intersection( std::move(r1_indices), std::move(r2_indices) );
				
				if( std::size(common_indices) != 0 )
					return std::make_tuple(r1_it, r2_it, std::move(common_indices));
			}
		}
		
		using indices_type = decltype(IndexHandler::extract_indices(*std::begin(range1)));
		using common_indices_type = std::decay_t<decltype(index_intersection(std::declval<indices_type>(),
																			 std::declval<indices_type>()))>;
		return std::make_tuple(std::end(range1), std::end(range2), common_indices_type{});
	}
	
	// TODO: rework everything further below!
	
// FIXME: Due to the gcc bug #91609, these things have to be public...
public:
	using abstract_index_polynomial;
	using underlying_polynomial = UnderlyingPolynomial;
	using coefficient_ring = typename underlying_polynomial::coefficient_ring;
	
	
	
	
	template<class Function, class CPart, class Variables>
	static underlying_polynomial
	apply_to_cpartv_contractions(Function &&function, CPart &&cpart, Variables &&variables) {
		static_assert(std::is_rvalue_reference_v<CPart>, "cpart is not an rvalue reference.");
		static_assert(std::is_rvalue_reference_v<Variables>, "variables is not an rvalue reference.");
		
		auto index_coincidence = find_index_coincidence(cpart, variables, true);
		auto common_indices = std::get<2>(index_coincidence);
		
		if(std::size(common_indices) == 0)
			return underlying_polynomial{index_handler.compose_coefficient_part(std::move(cpart)),
										 std::move(variables)};
		
		auto c_it = std::get<0>(index_coincidence);
		auto v_it = std::get<1>(index_coincidence);
		
		auto c_pre = boost::make_iterator_range(std::begin(cpart), c_it);
		auto c_post = boost::make_iterator_range(c_it + 1), std::end(cpart));
		
		underlying_polynomial result = function(std::move(c_pre), std::move(*c_it), std::move(c_post), std::move(*v_it),
												std::move(common_indices));
		return (underlying_polynomial{coefficient_ring::one(), {std::make_move_iterator(std::cbegin(variables)),
																std::make_move_iterator(v_it)}} *
				std::move(result) *
				underlying_polynomial{coefficient_ring::one(), {std::make_move_iterator(v_it + 1),
																std::make_move_iterator(std::end(variables))}});
	}
	
	template<class Function, class Coefficient, class Variables>
	static underlying_polynomial
	canonicalize_cv_contractions(Function &&function, Coefficient &&c, Variables &&variables) {
		auto c_decomposed = index_handler.decompose_coefficient(c);
		
		underlying_polynomial result;
		std::for_each(c_decomposed.begin(), c_decomposed.end(), [&variables](auto &&c_part) {
			result += apply_to_cpartv_contractions(std::forward<Function>(f), std::move(c_part), variables);
			);
		}
		
		template<class Function, class Coefficient, class Variables>
		static underlying_polynomial
		apply_to_vv_contractions(Function &&function, Coefficient &&c, Variables &&variables) {
			static_assert(std::is_rvalue_reference_v<Coefficient>, "c is not an rvalue reference.");
			static_assert(std::is_rvalue_reference_v<Variables>, "variables is not an rvalue reference.");
			
			auto index_coincidence = find_index_coincidence(cpart, variables, true);
			auto common_indices = std::get<2>(index_coincidence);
			
			if(std::size(common_indices) == 0)
				return underlying_polynomial{std::move(c), std::move(variables)};
			
			auto v1_it = std::get<0>(index_coincidence);
			auto v2_it = std::get<1>(index_coincidence);
			
			underlying_polynomial result = function(v1, boost::make_iterator_range(v1_it + 1, v2_it), v2,
													std::move(common_indices));
			return (underlying_polynomial{std::move(c), {std::make_move_iterator(std::begin(variables)),
														 std::make_move_iterator(v1_it)}} * std::move(result) *
					underlying_polynomial{coefficient_ring::one(), {std::make_move_iterator(v1_it + 1),
																	std::make_move_iterator(std::end(variables))}});
		}
		
		void canonicalize(abstract_index_polynomial &p) {
			underlying_polynomial contracted;
			std::for_each(p.monomials.begin(), p.monomials.end(), [](auto &&monomial) {
				contracted += apply_to_vv_contractions(index_handler.contract, std::move(monomial.coefficient),
													   std::move(monomial.variables));
			});
			
			p = ring<abstract_index_polynomial>::zero();
			std::for_each(contracted.monomials.begin(), contracted.monomials.end(), [](auto &&monomial) {
				p += apply_to_cv_contractions(index_handler.contract, std::move(monomial.coefficient),
											  std::move(monomial.variables));
			});
		}
		
		static underlying_polynomial expand_all_summations(const abstract_index_polynomial &p) {
			underlying_polynomial expanded = std::move(p);
			std::for_each(p.monomials.begin(), p.monomials.end(), [](auto &&monomial) {
				expanded += apply_to_vv_contractions(index_handler.expand, std::move(monomial.coefficient),
													 std::move(monomial.variables));
			});
			
			p = ring<abstract_index_polynomial>::zero();
			std::for_each(expanded.monomials.begin(), expanded.monomials.end(), [](auto &&monomial) {
				p += apply_to_cv_contractions(index_handler.expand, std::move(monomial.coefficient),
											  std::move(monomial.variables));
			});
		}
		
		static bool equal(const abstract_index_polynomial &p1, const abstract_index_polynomial &p2) {
			return (expand_all_summations(p1) == expand_all_summations(p2);
		}
		
		public:
		using type = abstract_index_polynomial;
	};
};
}
}

#endif //CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP
