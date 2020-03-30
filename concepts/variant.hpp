//
// Created by jayz on 18.03.20.
//

#ifndef CXXMATH_CONCEPTS_VARIANT_HPP
#define CXXMATH_CONCEPTS_VARIANT_HPP

namespace cxxmath {
namespace concepts {
template<class Types, class Visit> class variant {
	template<class T>
	struct get_alternative_impl {
		template<class Variant>
		static constexpr decltype(auto) apply( Variant &&v ) {
			constexpr auto visitor = [] ( auto &&x ) -> T {
				if constexpr ( std::is_same_v<std::decay_t<decltype(x)>, T> )
					return x;
				
				throw std::bad_variant_access();
			};
			return visit( visitor, std::forward<Variant>( v ) );
		}
	};
	template<class T>
	struct holds_alternative_impl {
		template<class Variant>
		static constexpr decltype(auto) apply( Variant &&v ) {
			constexpr auto predicate = [] ( auto &&x ) {
				if( std::is_same_v<std::decay_t<decltype(x)>, T> )
					return true;
				else
					return false;
			};
			return visit( predicate, std::forward<Variant>( v ) );
		}
	};
public:
	static constexpr auto types = static_function_object<Types>;
	static constexpr auto visit = static_function_object<Visit>;
	
	template<class T>
	static constexpr auto get_alternative = static_function_object<get_alternative_impl<T>>;
	template<class T>
	static constexpr auto holds_alternative = static_function_object<holds_alternative_impl<T>>;
};

template<class> struct is_variant: std::false_type {};
template<class Visit, class GetAlternativeWithPredicate>
struct is_variant<variant<Visit, GetAlternativeWithPredicate>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_variant)
}

template<class Type, class Variant>
struct type_models_concept<Type, Variant, std::enable_if_t<concepts::is_variant_v<Variant>>> {
private:
	struct void_action {
		template<class T> constexpr void operator()( T && ) const {}
	};
	struct true_predicate {
		template<class T> constexpr std::true_type operator()( T && ) const { return std::true_type{}; }
	};
public:
	static constexpr bool value = (
		std::is_invocable_v<decltype(Variant::visit), void_action, Type>
		// FIXME: We cannot check get_alternative_with_predicate()
	);
};

CXXMATH_DEFINE_CONCEPT( variant )

class dispatch_visit {
	template<std::size_t N, class F, class VariantOrResolvedArg, class ...NextVariantsAndResolvedArgs>
	static constexpr decltype(auto)
	apply( F &&f, VariantOrResolvedArg &&varg, NextVariantsAndResolvedArgs &&...nvargs ) {
		if constexpr( N == sizeof...(NextVariantsAndResolvedArgs) + 1 )
			return std::forward<F>( f )(
				std::forward<VariantOrResolvedArg>( varg ),
				std::forward<NextVariantsAndResolvedArgs>( nvargs )...
			);
		else {
			constexpr auto iterate = [&f, &nvargs...] ( auto &&x ) {
				return apply<N + 1>(
					std::forward<F>( f ),
					std::forward<NextVariantsAndResolvedArgs>( nvargs )...,
					std::forward<decltype(x)>( x )
				);
			};
			
			using dispatch_tag = tag_of_t<VariantOrResolvedArg>;
			return default_variant_t<dispatch_tag>::visit(
				iterate,
				std::forward<VariantOrResolvedArg>( varg )
			);
		}
	}
public:
	template<class F, class Variant, class ...OtherVariants>
	constexpr decltype( auto ) operator()( F &&f, Variant &&variant, OtherVariants &&...other_variants ) const {
		constexpr auto iterate = [&f, &other_variants...] ( auto &&x ) {
			return apply<0>(
				std::forward<F>( f ),
				std::forward<OtherVariants>( other_variants )...,
				std::forward<decltype(x)>( x )
			);
		};
		return default_variant_t<tag_of_t<Variant>>::visit( iterate, std::forward<Variant>( variant ) );
	}
};
static constexpr dispatch_visit visit;

struct dispatch_get_alternative_with_predicate {
	template<class UnaryPredicate, class Variant>
	constexpr decltype(auto) operator()( UnaryPredicate &&p, Variant &&v ) const {
		return default_variant_t<tag_of_t<Variant>>::get_with_predicate(
			std::forward<UnaryPredicate>( p ),
			std::forward<Variant>( v )
		);
	}
};
static constexpr dispatch_get_alternative_with_predicate get_alternative_with_predicate;

template<class NodeData> struct dispatch_holds_alternative {
	template<class Variant>
	constexpr decltype(auto) operator()( Variant &&v ) const {
		return default_variant_t<tag_of_t<Variant>>::template holds_alternative<NodeData>( std::forward<Variant>( v ) );
	}
};
template<class NodeData>
static constexpr dispatch_holds_alternative<NodeData> holds_alternative;

template<class NodeData> struct dispatch_get_alternative {
	template<class Variant>
	constexpr decltype(auto) operator()( Variant &&v ) const {
		return default_variant_t<tag_of_t<Variant>>::template get_alternative<NodeData>( std::forward<Variant>( v ) );
	}
};
template<class NodeData>
static constexpr dispatch_get_alternative<NodeData> get_alternative;
}

#endif //CXXMATH_CONCEPTS_VARIANT_HPP
