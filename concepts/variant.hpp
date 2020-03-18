//
// Created by jayz on 18.03.20.
//

#ifndef CXXMATH_CONCEPTS_VARIANT_HPP
#define CXXMATH_CONCEPTS_VARIANT_HPP

namespace cxxmath {
namespace concepts {
template<class Visit> struct variant {
	static constexpr auto visit = function_object_v<Visit>;
};

template<class> struct is_variant: std::false_type {};
template<class Types, class Visit>
struct is_variant<variant<Types, Visit>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_variant)
}

template<class Type, class Variant>
struct type_models_concept<Type, Variant, std::enable_if_t<concepts::is_variant_v<Variant>>> {
	static constexpr bool value = CXXMATH_IS_VALID( Variant::visit, [] ( auto && ) {}, std::declval<Type>() );
};

CXXMATH_DEFINE_CONCEPT( variant )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( visit, variant )
)

class dispatch_visit {
	template<std::size_t N, class F, class Arg, class ...Args>
	constexpr decltype(auto) apply_n( F &&f, Arg &&arg, Args &&...args ) const {
		if constexpr( sizeof...(Args) + 1 == N )
			return std::forward<F>( f )( std::forward<Arg>( arg ), std::forward<Args>( args )... );
		else
			return apply_n<N>( std::forward<F>( f ), std::forward<Args>( args )... );
	}
	
	template<std::size_t N, class F, class ...VariantsAndResolvedArgs>
	constexpr decltype(auto) apply( F &&f, VariantsAndResolvedArgs &&...vargs ) const {
		if constexpr( 2 * N == sizeof...(VariantsAndResolvedArgs) )
			return apply_n<N>( std::forward<F>( f ), std::forward<VariantsAndResolvedArgs>( vargs )... );
		else {
			using variant = template_at_c_t<N, ResolvedArgsAndVariants...>;
			constexpr iterate = [&f, &args...] ( auto &&x ) {
				return apply<N + 1>(
					std::forward<F>( f ),
					std::forward<VariantsAndResolvedArgs>( args )...,
					std::forward<decltype(x)>( x )
				);
			};
			
			using dispatch_tag = tag_of_t<>;
			return default_variant_t<dispatch_tag>::visit(
				iterate,
				std::forward<variant>( function_at_c<N>( std::forward<ResolvedArgsAndVariants>( args )... ) )
			);
		}
		
	}
public:
	template<class F, class ...Variants, class = std::enable_if_t<sizeof...(Variants) != 0>>
	constexpr decltype( auto ) operator()( F &&f, Variants &&...variants ) const {
		constexpr auto iterate = [&f, &other_variants...] ( auto &&x ) {
			return apply<0>(
				std::forward<F>( f ),
				std::forward<Variants>( variants )...,
			);
		};
		
		using dispatch_tag = tag_of_t<Variants>;
		return default_variant_t<dispatch_tag>::visit( iterate, std::forward<Variant>( v ) );
	}
};

static constexpr default_visit_dispatch visit;
}

#endif //CXXMATH_CONCEPTS_VARIANT_HPP
