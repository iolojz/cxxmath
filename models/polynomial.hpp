//
// Created by jayz on 03.10.19.
//

#ifndef CXXMATH_MODELS_POLYNOMIAL_HPP
#define CXXMATH_MODELS_POLYNOMIAL_HPP

#include "std_get.hpp"
#include "std_vector.hpp"
#include "product_monoid.hpp"

#include "core/erase_if.hpp"
#include "core/unpack_template.hpp"
#include "core/move_aware_range_helpers.hpp"

#include <boost/range/combine.hpp>
#include <boost/container/flat_map.hpp>

namespace cxxmath
{
template<class Coefficient, class Variable, class CoefficientSet = default_set_t <tag_of_t<Coefficient>>, class CoefficientRing = default_ring_t <tag_of_t<Coefficient>>, class TotalVariableOrder = default_total_order_t <tag_of_t<Variable>>>
struct polynomial_tag
{
	using coefficient = Coefficient;
	using coefficient_set = CoefficientSet;
	using coefficient_ring = CoefficientRing;
	using variable = Variable;
	using total_variable_order = TotalVariableOrder;
};

template<class Tag>
struct is_polynomial_tag : std::false_type
{
};

template<class Coefficient, class Variable, class CoefficientSet, class CoefficientRing, class TotalVariableOrder>
struct is_polynomial_tag<polynomial_tag<Coefficient, Variable, CoefficientSet, CoefficientRing, TotalVariableOrder>>
: std::true_type
{
};

template<class Coefficient, class Variable, class CoefficientSet = default_set_t <tag_of_t<Coefficient>>, class CoefficientRing = default_ring_t <tag_of_t<Coefficient>>, class TotalVariableOrder = default_total_order_t <tag_of_t<Variable>>>
struct polynomial
{
	using dispatch_tag = polynomial_tag<Coefficient, Variable, CoefficientSet, CoefficientRing, TotalVariableOrder>;
	
	using coefficient = Coefficient;
	using coefficient_set = CoefficientSet;
	using coefficient_ring = CoefficientRing;
	using variable = Variable;
	using variable_monoid = typename vector_monoid<variable>::type;
private:
	using monomial_product = std_get_product;
	using monomial_monoid = product_monoid<monomial_product, variable_monoid, typename coefficient_ring::monoid_>;
	
	static constexpr auto add_assign_coefficients = coefficient_ring::add_assign;
	static constexpr auto equal_coefficients = coefficient_set::equal;
	static constexpr auto zero_coefficient = coefficient_ring::zero;
	static constexpr auto negate_coefficient_in_place = coefficient_ring::negate_in_place;
	
	struct less_variables
	{
		template<class V1, class V2>
		decltype( auto ) operator()( V1 &&v1, V2 &&v2 ) const
		{
			return TotalVariableOrder::less( std::forward<V1>( v1 ), std::forward<V2>( v2 ));
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
			return equal_coefficients( monomial_product::second( monomial ), zero_coefficient());
		} );
	}
	
	// FIXME: Should we enable runtime switching of this type?
	using monomial_container = boost::container::flat_map<std::vector<variable>, coefficient, less_variable_ranges>;
	//using monomial_container = std::map<decltype(std::declval<monomial>().variables), coefficient, less_variable_ranges>;
	monomial_container monomial_map;
public:
	polynomial( void ) = default;
	
	polynomial( const polynomial & ) = default;
	
	polynomial( polynomial && ) = default;
	
	template<class ...Variables>
	explicit polynomial( const coefficient &c, Variables &&...variables )
	: monomial_map{
	typename monomial_container::value_type{ std::vector<variable>{ std::forward<Variables>( variables )... }, c }}
	{ strip_zeros(); }
	
	template<class ...Variables>
	explicit polynomial( coefficient &&c, Variables &&...variables )
	: monomial_map{
	typename monomial_container::value_type{ std::vector<variable>{ std::forward<Variables>( variables )... },
											 std::move( c ) }}
	{ strip_zeros(); }
	
	// FIXME: Get rid of move_aware_xxx(). It violates the usual rvalue semantics
	template<class Monomial1, class ...MonomialTail, class = std::enable_if_t<!is_polynomial_tag<
	tag_of_t < Monomial1>>::value>,
	CXXMATH_DISABLE_IF_TAG_IS(
	Monomial1, tag_of_t<coefficient>
	) >
	
	explicit polynomial( Monomial1 &&m1, MonomialTail &&...tail ) : monomial_map{
	typename monomial_container::value_type{
	{ move_aware_begin( monomial_product::first( std::forward<Monomial1>( m1 ))),
	  move_aware_end( monomial_product::first( std::forward<Monomial1>( m1 ))) },
	monomial_product::second( std::forward<Monomial1>( m1 )) }}
	{
		*this += polynomial{ std::forward<MonomialTail>( tail )... };
		strip_zeros();
	}
	
	polynomial &operator=( const polynomial & ) = default;
	
	polynomial &operator=( polynomial && ) = default;
	
	const monomial_container &monomials( void ) const
	{ return monomial_map; }
	
	polynomial &operator+=( const polynomial &p )
	{
		if( this == &p ) {
			for( auto &term : monomial_map )
				add_assign_coefficients( term.second, term.second );
			
			return *this;
		}
		
		for( const auto &term : p.monomials()) {
			auto insertion_result = monomial_map.insert( term );
			if( insertion_result.second == false ) {
				add_assign_coefficients( insertion_result.first->second, term.second );
				if( equal_coefficients( insertion_result.first->second, zero_coefficient()))
					monomial_map.erase( insertion_result.first );
			}
		}
		
		return *this;
	}
	
	polynomial &operator+=( polynomial &&p )
	{
		if( this == &p ) {
			for( auto &term : monomial_map )
				add_assign_coefficients( term.second, term.second );
			return *this;
		}
		
		for( auto &&term : p.monomials()) {
			auto insertion_result = monomial_map.insert( term );
			if( insertion_result.second == false ) {
				add_assign_coefficients( insertion_result.first->second, std::move( term.second ));
				if( equal_coefficients( insertion_result.first->second, zero_coefficient()))
					monomial_map.erase( insertion_result.first );
			}
		}
		
		return *this;
	}
	
	template<class Polynomial>
	polynomial &operator-=( Polynomial &&p )
	{
		return *this += -std::forward<Polynomial>( p );
	}
	
	CXXMATH_COMMUTATIVE_BINARY_OPERATOR_OVERLOAD( polynomial,
	+)
	
	CXXMATH_BINARY_OPERATOR_OVERLOAD( polynomial,
	- )
	
	polynomial &negate_in_place( void )
	{
		for( auto &term : monomial_map )
			negate_coefficient_in_place( term.second );
		
		return *this;
	}
	
	polynomial operator-( void ) &&
	{
		negate_in_place();
		return std::move( *this );
	}
	
	polynomial operator-( void ) const &
	{ return -polynomial{ *this }; }
	
	polynomial &operator*=( const polynomial &p )
	{
		// FIXME: do not take cartesian product for simple multiplications!
		
		std::vector<typename monomial_container::value_type> monomials;
		for( const auto &factor1 : monomial_map ) {
			for( const auto &factor2 : p.monomial_map ) {
				monomials.push_back( monomial_monoid::compose( factor1, factor2 ));
			}
		}
		
		monomial_container result;
		for( auto &&monomial : monomials ) {
			if( equal_coefficients( monomial.second, zero_coefficient()))
				continue;
			
			auto insertion_result = result.insert( monomial );
			if( insertion_result.second == false ) {
				add_assign_coefficients( insertion_result.first->second, std::move( monomial.second ));
				if( equal_coefficients( insertion_result.first->second, zero_coefficient()))
					result.erase( insertion_result.first );
			}
		}
		
		monomial_map = std::move( result );
		return *this;
	}
	
	CXXMATH_BINARY_OPERATOR_OVERLOAD( polynomial, *
	)
	
	polynomial &scalar_multiply_assign( const coefficient &c )
	{
		for( auto &monomial : monomial_map )
			coefficient_ring::multiply_assign( monomial.second, c );
		return *this;
	}
	
	polynomial scalar_multiply( const coefficient &c )
	{
		return polynomial{ *this }.scalar_multiply_assign( c );
	}
	
	bool operator==( const polynomial &p ) const
	{
		if( monomial_map.size() != p.monomial_map.size())
			return false;
		
		for( const auto &monomials : boost::combine( monomial_map, p.monomial_map )) {
			const auto &variables1 = boost::get<0>( monomials ).first;
			const auto &variables2 = boost::get<1>( monomials ).first;
			
			if( variables1.size() != variables2.size())
				return false;
			
			for( const auto &variables : boost::combine( variables1, variables2 )) {
				if( TotalVariableOrder::not_equal( boost::get<0>( variables ), boost::get<1>( variables )))
					return false;
			}
			
			if( !equal_coefficients( boost::get<0>( monomials ).second, boost::get<1>( monomials ).second ))
				return false;
		}
		
		return true;
	}
	
	bool operator!=( const polynomial &p ) const
	{ return !( *this == p ); }
};

namespace impl
{
template<class Coefficient, class Variable, class CoefficientSet, class CoefficientRing, class TotalVariableOrder>
struct common_tag<
tag_of_t < Coefficient>, polynomial_tag<Coefficient, Varable, CoefficientSet, CoefficientRing, TotalVariableOrder>>
{
using type = polynomial_tag<Coefficient, Varable, CoefficientSet, CoefficientRing, TotalVariableOrder>;
};

template<class Coefficient, class Variable, class CoefficientSet, class CoefficientRing, class TotalVariableOrder>
struct common_tag<polynomial_tag<Coefficient, Varable, CoefficientSet, CoefficientRing, TotalVariableOrder>,
tag_of_t < Coefficient>>
{
using type = polynomial_tag<Coefficient, Varable, CoefficientSet, CoefficientRing, TotalVariableOrder>;
};

struct supports_polynomial_tag
{
	template<class Tag>
	static constexpr bool supports_tag( void )
	{
		return is_polynomial_tag<Tag>::value;
	}
};

struct polynomial_equal : supports_polynomial_tag
{
	template<class Polynomial1, class Polynomial2>
	static constexpr decltype( auto ) apply( Polynomial1 &&p1, Polynomial2 &&p2 )
	{
		return std::forward<Polynomial1>( p1 ) == std::forward<Polynomial2>( p2 );
	}
};

struct polynomial_add_assign : supports_polynomial_tag
{
	template<class Polynomial1, class Polynomial2>
	static constexpr decltype( auto ) apply( Polynomial1 &&p1, Polynomial2 &&p2 )
	{
		return std::forward<Polynomial1>( p1 ) += std::forward<Polynomial2>( p2 );
	}
};

struct polynomial_addition_is_abelian
{
	static constexpr bool apply( void )
	{
		return true;
	}
};

template<class Coefficient, class Variable, class CoefficientSet, class CoefficientRing, class TotalVariableOrder>
struct polynomial_zero
{
	static constexpr auto apply( void )
	{
		return polynomial < Coefficient, Variable, CoefficientSet, CoefficientRing, TotalVariableOrder > {};
	}
};

struct polynomial_negate_in_place : supports_polynomial_tag
{
	template<class Polynomial>
	static constexpr decltype( auto ) apply( Polynomial &&p )
	{
		return std::forward<Polynomial>( p ).negate_in_place();
	}
};

struct polynomial_multiply_assign : supports_polynomial_tag
{
	template<class Polynomial1, class Polynomial2>
	static constexpr decltype( auto ) apply( Polynomial1 &&p1, Polynomial2 &&p2 )
	{
		return std::forward<Polynomial1>( p1 ) *= std::forward<Polynomial2>( p2 );
	}
};

struct polynomial_multiplication_is_abelian
{
	static constexpr bool apply( void )
	{
		return false;
	}
};

template<class Coefficient, class Variable, class CoefficientSet, class CoefficientRing, class TotalVariableOrder>
struct polynomial_one
{
	static constexpr auto apply( void )
	{
		return polynomial < Coefficient, Variable, CoefficientSet, CoefficientRing, TotalVariableOrder >
																					{ CoefficientRing::one() };
	}
};

struct polynomial_scalar_multiply_assign : supports_polynomial_tag
{
	template<class C, class Polynomial>
	static constexpr auto apply( C &&c, Polynomial &&p )
	{
		return std::forward<Polynomial>( p ).scalar_multiply_assign( std::forward<C>( c ));
	}
};

template<class DispatchTag>
struct default_set<DispatchTag, std::enable_if_t<is_polynomial_tag<DispatchTag>::value>>
{
	using type = concepts::set<polynomial_equal>;
};

template<class DispatchTag>
struct default_monoid<DispatchTag, std::enable_if_t<is_polynomial_tag<DispatchTag>::value>>
{
private:
	using polynomial_one_ = unpack_template_t<DispatchTag, polynomial_one>;
public:
	using type = concepts::assignable_monoid<polynomial_multiply_assign, polynomial_one_, polynomial_multiplication_is_abelian>;
};

template<class DispatchTag>
struct default_group<DispatchTag, std::enable_if_t<is_polynomial_tag<DispatchTag>::value>>
{
private:
	using polynomial_zero_ = unpack_template_t<DispatchTag, polynomial_zero>;
public:
	using type = concepts::assignable_group <concepts::assignable_monoid<polynomial_add_assign, polynomial_zero_, polynomial_addition_is_abelian>, polynomial_negate_in_place>;
};

template<class DispatchTag>
struct default_r_module<DispatchTag, std::enable_if_t<is_polynomial_tag<DispatchTag>::value>>
{
	using type = concepts::assignable_r_module <default_group_t<DispatchTag>, polynomial_scalar_multiply_assign>;
};

template<class DispatchTag>
struct default_r_algebra<DispatchTag, std::enable_if_t<is_polynomial_tag<DispatchTag>::value>>
{
	using type = concepts::r_algebra <default_r_module_t<DispatchTag>, default_monoid_t<DispatchTag>>;
};

template<class PolynomialTag>
struct make_polynomial
{
	static_assert( is_polynomial_tag<PolynomialTag>::value, "The given tag is not a polynomial_tag." );
private:
	using coefficient = typename PolynomialTag::coefficient;
	using coefficient_set = typename PolynomialTag::coefficient_set;
	using coefficient_ring = typename PolynomialTag::coefficient_ring;
	using variable = typename PolynomialTag::variable;
	using total_variable_order = typename PolynomialTag::total_variable_order;
	
	using polynomial_ = polynomial<coefficient, variable, coefficient_set, coefficient_ring, total_variable_order>;
public:
	template<class ...Args>
	static constexpr polynomial_ apply( Args &&... args )
	{
		return polynomial_{ std::forward<Args>( args )... };
	}
};

}

template<class PolynomialTag>
struct default_make_polynomial_dispatch
{
	static_assert( is_polynomial_tag<PolynomialTag>::value, "The given tag is not a polynomial_tag." );
	
	template<class ...Args>
	constexpr decltype( auto ) operator()( Args &&... args ) const
	{
		return impl::make_polynomial<PolynomialTag>::apply( std::forward<Args>( args )... );
	}
};

template<class PolynomialTag> static constexpr default_make_polynomial_dispatch<PolynomialTag> make_polynomial;

template<class PolynomialTag> using polynomial_ring = concepts::ring <default_group_t<PolynomialTag>, default_monoid_t<PolynomialTag>>;
template<class PolynomialTag> using polynomial_r_algebra = default_r_algebra_t<PolynomialTag>;

namespace impl
{
template<class PolynomialTag>
struct default_ring<PolynomialTag, std::enable_if_t<is_polynomial_tag<PolynomialTag>::value>>
{
	using type = polynomial_ring<PolynomialTag>;
};
}
}

#endif //CXXMATH_MODELS_POLYNOMIAL_HPP
