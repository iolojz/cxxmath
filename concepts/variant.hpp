//
// Created by jayz on 18.03.20.
//

#ifndef CXXMATH_CONCEPTS_VARIANT_HPP
#define CXXMATH_CONCEPTS_VARIANT_HPP

namespace cxxmath {
namespace concepts {
template<class Visit> class variant {
	struct get_impl {
		template<class T, class Variant>
		static constexpr decltype(auto) apply( Variant &&v ) {
			constexpr auto visitor = [] ( auto &&x ) {
				if constexpr( std::is_same_v<std::decay_t<decltype(x)>, T> )
					return x;
				else
					throw std::bad_variant_access{};
			};
			return visit( visitor, std::forward<Variant>( v ) );
		}
	};
public:
	static constexpr auto visit = function_object_v<Visit>;
	static constexpr auto get = function_object_v<get_impl>;
};

template<class> struct is_variant: std::false_type {};
template<class Visit>
struct is_variant<variant<Visit>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_variant)
}

template<class Type, class Variant>
struct type_models_concept<Type, Variant, std::enable_if_t<concepts::is_variant_v<Variant>>> {
private:
	struct void_action {
		template<class T> constexpr void operator()( T && ) const {}
	};
public:
	static constexpr bool value = std::is_invocable_v<decltype(Variant::visit), void_action, Type>;
};

CXXMATH_DEFINE_CONCEPT( variant )

class dispatch_visit {
	template<std::size_t N, class F, class VariantOrResolvedArg, class ...NextVariantsAndResolvedArgs>
	constexpr decltype(auto) apply( F &&f, VariantOrResolvedArg &&varg, NextVariantsAndResolvedArgs &&...nvargs ) const {
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

template<class NodeData> struct dispatch_get_alternative {
	template<class Variant>
	constexpr decltype(auto) operator()( Variant &&v ) const {
		return default_variant_t<tag_of_t<Variant>>::get.template operator()<NodeData>( std::forward<Variant>( v ) );
	}
};
template<class NodeData>
static constexpr dispatch_get_alternative<NodeData> get_alternative;
}

#endif //CXXMATH_CONCEPTS_VARIANT_HPP
