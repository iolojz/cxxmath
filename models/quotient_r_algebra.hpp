//
// Created by jayz on 08.10.19.
//

#ifndef CXXMATH_CONCEPTS_QUOTIENT_HPP
#define CXXMATH_CONCEPTS_QUOTIENT_HPP

#include "core/make.hpp"
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
	using r_algebra_object = detail::free_r_algebra<Coefficient, Symbol, CoefficientSet, CoefficientRing, SymbolTotalOrder>;
	using r_algebra_tag = tag_of_t<r_algebra_object>;
	using dispatch_tag = quotient_r_alebra_tag<tag_of_t<r_algebra_object>, RAlgebraQuotientSpec>;
private:
	r_algebra_object rep;
	
	struct zero {
		static quotient_r_algebra apply() {
			return zero<r_algebra_tag>();
		}
	};
	
	struct negate_in_place {
		template<class QRA>
		static QRA &apply( QRA &qra ) {
			RAlgebraQuotientSpec::negate_in_place::apply( qra.rep );
		}
	};
	
	struct scalar_multiply_assign {
		template<class C, class QRA>
		static QRA &apply( C &&c, QRA &qra ) {
			RAlgebraQuotientSpec::scalar_multiply_assign::apply( std::forward<C>( c ), qra.rep );
		}
	};
	
	struct add_assign {
		template<class QRA1, class QRA2>
		static QRA1 &apply( QRA1 &qra1, QRA2 &&qra2 ) {
			RAlgebraQuotientSpec::add_assign::apply( qra1.rep, std::forward<QRA2>( qra2 ).rep );
		}
	};
	
	struct multiply_assign {
		template<class QRA1, class QRA2>
		static QRA1 &apply( QRA1 &qra1, QRA2 &&qra2 ) {
			RAlgebraQuotientSpec::multiply_assign::apply( qra1.rep, std::forward<QRA2>( qra2 ).rep );
		}
	};
	
	struct make {
		template<class ...Args>
		static decltype(auto) apply( Args &&...args ) {
			quotient_r_algebra result = make<tag_of_t<r_algebra_object>>( std::forward<Args>( args )... );
			RAlgebraQuotientSpec::quotient_map_in_place::apply( result.rep );
		}
	};
	
	struct one {
		static r_algebra_object apply() {
			return one<r_algebra_tag>();
		}
	};
	
	struct equal {
		template<class QRA1, class QRA2>
		static decltype(auto) apply( QRA1 &&qra1, QRA2 &&qra2 )
		{
			return RAlgebraQuotientSpec::equal::apply( std::forward<QRA1>( qra1 ).rep, std::forward<QRA2>( qra2 ).rep );
		}
	};
};
}

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct quotient_r_algebra_monoid {
private:
	using value_type = detail::quotient_r_algebra<FreeRAlgebraTag, RAlgebraQuotientSpec>;
public:
	using type = concepts::assignable_monoid<typename value_type::multiply_assign, typename RAlgebraQuotientSpec::multiplication_is_commutative, typename value_type::one>;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct quotient_r_algebra_group {
private:
	using value_type = detail::quotient_r_algebra<FreeRAlgebraTag, RAlgebraQuotientSpec>;
public:
	using type = concepts::assignable_group<
	concepts::assignable_monoid<typename value_type::add_assign, impl::true_implementation, typename value_type::zero>,
	typename value_type::negate_in_place
	>;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct quotient_r_algebra_ring {
	using type = concepts::ring<
	quotient_r_algebra_group<FreeRAlgebraTag, RAlgebraQuotientSpec>,
	quotient_r_algebra_monoid<FreeRAlgebraTag, RAlgebraQuotientSpec>
	>;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
using quotient_r_module = concepts::assignable_r_module<
typename quotient_r_algebra_group<FreeRAlgebraTag, RAlgebraQuotientSpec>::type,
typename detail::quotient_r_algebra<FreeRAlgebraTag, RAlgebraQuotientSpec>::scalar_multiply_assign
>;

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
using quotient_r_algebra = concepts::r_algebra<
typename quotient_r_module<FreeRAlgebraTag, RAlgebraQuotientSpec>::type,
typename quotient_r_algebra_monoid<FreeRAlgebraTag, RAlgebraQuotientSpec>::type
>;

namespace impl {
template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_monoid<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_algebra_monoid<FreeRAlgebraTag, RAlgebraQuotientSpec>::type;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_group<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_algebra_group<FreeRAlgebraTag, RAlgebraQuotientSpec>::type;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_ring<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_algebra_ring<FreeRAlgebraTag, RAlgebraQuotientSpec>::type;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_r_module<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_module<FreeRAlgebraTag, RAlgebraQuotientSpec>::type;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct default_r_algebra<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	using type = typename quotient_r_algebra<FreeRAlgebraTag, RAlgebraQuotientSpec>::type;
};

template<class FreeRAlgebraTag, class RAlgebraQuotientSpec>
struct make<quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>> {
	template<class ...Args> static constexpr decltype(auto) apply( Args &&...args ) {
		return detail::quotient_r_algebra_tag<FreeRAlgebraTag, RAlgebraQuotientSpec>::make::apply(
			std::forward<Args>( args )...
		)
	}
};
}
}

#endif //CXXMATH_CONCEPTS_QUOTIENT_HPP
