//
// Created by jayz on 08.10.19.
//

#ifndef CXXMATH_MODELS_QUOTIENT_R_ALGEBRA_HPP
#define CXXMATH_MODELS_QUOTIENT_R_ALGEBRA_HPP

#include "free_r_algebra.hpp"

namespace cxxmath {
template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct quotient_r_algebra_tag {
	static_assert( is_free_r_algebra_tag_v<FreeRAlgebraTag> );
};

namespace detail {
template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct quotient_r_algebra;

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder, class RAlgebraQuotientSpec>
struct quotient_r_algebra<free_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>, RAlgebraQuotientSpec> {
	using r_algebra = ::cxxmath::free_r_algebra<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>;
	using r_algebra_object = detail::free_r_algebra<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>;
	using r_algebra_tag = tag_of_t<r_algebra_object>;
	using dispatch_tag = quotient_r_algebra_tag<tag_of_t<r_algebra_object>, RAlgebraQuotientSpec>;
private:
	r_algebra_object rep;
public:
	quotient_r_algebra() = default;
	quotient_r_algebra( const quotient_r_algebra & ) = default;
	quotient_r_algebra( quotient_r_algebra && ) = default;
	
	explicit quotient_r_algebra( const r_algebra_object &r ) : rep{ r } {
		RAlgebraQuotientSpec::quotient_map_in_place::apply( rep );
	}
	explicit quotient_r_algebra( r_algebra_object &&r ) : rep{ std::move( r ) } {
		RAlgebraQuotientSpec::quotient_map_in_place::apply( rep );
	}
	
	quotient_r_algebra &operator=( const quotient_r_algebra & ) = default;
	quotient_r_algebra &operator=( quotient_r_algebra && ) = default;
	
	quotient_r_algebra &operator=( const r_algebra_object &r ) {
		rep = r;
		RAlgebraQuotientSpec::quotient_map_in_place::apply( rep );
	};
	quotient_r_algebra &operator=( r_algebra_object &&r ) {
		rep = std::move( r );
		RAlgebraQuotientSpec::quotient_map_in_place::apply( rep );
	};
	
	const r_algebra_object &representative() const { return rep; }
	r_algebra_object &representative() { return rep; }
	
	struct zero {
		static constexpr auto apply() {
			return quotient_r_algebra{ r_algebra::zero() };
		}
	};
	
	struct negate_in_place : supports_tag_helper<dispatch_tag> {
		template<class QRA>
		static constexpr QRA &apply( QRA &qra ) {
			RAlgebraQuotientSpec::negate_in_place::apply( qra.rep );
			return qra;
		}
	};
	
	struct scalar_multiply_assign : supports_tag_helper<dispatch_tag>  {
		template<class C, class QRA>
		static constexpr QRA &apply( C &&c, QRA &qra ) {
			RAlgebraQuotientSpec::scalar_multiply_assign::apply( std::forward<C>( c ), qra.rep );
			return qra;
		}
	};
	
	struct add_assign : supports_tag_helper<dispatch_tag>  {
		template<class QRA1, class QRA2>
		static constexpr QRA1 &apply( QRA1 &qra1, QRA2 &&qra2 ) {
			RAlgebraQuotientSpec::add_assign::apply( qra1.rep, std::forward<QRA2>( qra2 ).rep );
			return qra1;
		}
	};
	
	struct multiply_assign : supports_tag_helper<dispatch_tag>  {
		template<class QRA1, class QRA2>
		static constexpr QRA1 &apply( QRA1 &qra1, QRA2 &&qra2 ) {
			RAlgebraQuotientSpec::multiply_assign::apply( qra1.rep, std::forward<QRA2>( qra2 ).rep );
			return qra1;
		}
	};
	
	struct make {
		template<class ...Args>
		static constexpr quotient_r_algebra apply( Args &&...args ) {
			return quotient_r_algebra{ ::cxxmath::make<tag_of_t<r_algebra_object>>( std::forward<Args>( args )... ) };
		}
	};
	
	struct one {
		static constexpr auto apply() {
			return quotient_r_algebra{ r_algebra::one() };
		}
	};
	
	struct equal : supports_tag_helper<dispatch_tag>  {
		template<class QRA1, class QRA2>
		static constexpr decltype(auto) apply( QRA1 &&qra1, QRA2 &&qra2 )
		{
			return RAlgebraQuotientSpec::equal::apply( std::forward<QRA1>( qra1 ).rep, std::forward<QRA2>( qra2 ).rep );
		}
	};
};
}

namespace model_quotient_r_algebra {
template<class FRATag, class QSpec> class quotient_r_algebra_concepts {
	using value_type = detail::quotient_r_algebra<FreeRAlgebraTag, RAlgebraQuotientSpec>;
public:
	using set =
};

template<class Coefficient, class Symbol, class CoefficientSet, class CoefficientRing, class SymbolTotalOrder>
class quotient_r_algebra_concepts<
	quotient_r_algebra_tag<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>,
	QSpec
> {
	using value_type = detail::free_r_algebra<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>;
public:
	using set = concepts::set<typename value_type::equal>;
	using monoid = concepts::assignable_monoid<
		typename value_type::multiply_assign,
		typename value_type::one,
		impl::false_implementation
	>;
	using group = concepts::assignable_group<
		concepts::assignable_monoid<
			typename value_type::add_assign,
			typename value_type::zero,
			impl::true_implementation
		>,
		typename value_type::negate_in_place
	>;
	using ring = concepts::ring<group, monoid>;
	using module = concepts::assignable_r_module<group, typename value_type::scalar_multiply_assign>;
	using algebra = concepts::r_algebra<module, monoid>;
	using make = typename value_type::make;
};
}

namespace impl {
template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_set<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_algebra_concepts<FreeRAlgebraTag, RAlgebraQuotientSpec>::set;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_monoid<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_algebra_concepts<FreeRAlgebraTag, RAlgebraQuotientSpec>::monoid;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_group<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_algebra_concepts<FreeRAlgebraTag, RAlgebraQuotientSpec>::group;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_ring<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_algebra_concepts<FreeRAlgebraTag, RAlgebraQuotientSpec>::ring;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_r_module<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_algebra_concepts<FreeRAlgebraTag, RAlgebraQuotientSpec>::module;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_r_algebra<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_algebra_concepts<FreeRAlgebraTag, RAlgebraQuotientSpec>::algebra;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct make<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	template<class ...Args> static constexpr decltype(auto) apply( Args &&...args ) {
		return quotient_r_algebra_concepts<FreeRAlgebraTag, RAlgebraQuotientSpec>::make::apply(
			std::forward<Args>( args )...
		);
	}
};
}

template<bool Flag>
struct commutes_with_quotient_map_helper {
	static constexpr bool commutes_with_quotient_map() {
		return Flag;
	}
};

#ifdef CXXMATH_DEFINE_COMPOSED_QUOTIENTS_FUNCTION
#error "CXXMATH_DEFINE_COMPOSED_QUOTIENTS_FUNCTION is already defined."
#endif
#define CXXMATH_DEFINE_COMPOSED_QUOTIENTS_FUNCTION( function ) \
struct function { \
	template<class ...Args> static constexpr decltype(auto) apply( Args &&...args ) { \
		using first_ ## function = typename FirstQuotientSpec::function; \
		using second_ ## function = typename SecondQuotientSpec::function; \
		\
		if constexpr( first_ ## function::commutes_with_quotient_map() ) \
			return second_ ## function::apply( std::forward<Args>( args )... ); \
		else \
			return SecondQuotientSpec::quotient_map_in_place( first_ ## function::apply( std::forward<Args>( args )... ) ); \
	} \
};

template<class SecondQuotientSpec, class FirstQuotientSpec> struct composed_quotients {
	using multiplication_is_commutative = typename SecondQuotientSpec::multiplication_is_commutative;
	
	CXXMATH_DEFINE_COMPOSED_QUOTIENTS_FUNCTION( negate_in_place )
	CXXMATH_DEFINE_COMPOSED_QUOTIENTS_FUNCTION( scalar_multiply_assign )
	CXXMATH_DEFINE_COMPOSED_QUOTIENTS_FUNCTION( add_assign )
	CXXMATH_DEFINE_COMPOSED_QUOTIENTS_FUNCTION( multiply_assign )
	
	struct quotient_map_in_place {
		template<class FRA> static constexpr decltype(auto) apply( FRA &&fra ) {
			return SecondQuotientSpec::quotient_map_in_place::apply( FirstQuotientSpec::quotient_map_in_place::apply( std::forward<FRA>( fra ) ) );
		}
	};
	
	using equal = typename SecondQuotientSpec::equal;
};

#undef CXXMATH_DEFINE_COMPOSED_QUOTIENTS_FUNCTION
}

#endif //CXXMATH_CONCEPTS_QUOTIENT_HPP
