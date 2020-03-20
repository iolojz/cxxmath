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
template<class Types, class Visit>
struct is_variant<variant<Types, Visit>>: std::true_type {};

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
	template<std::size_t N, class F, class VariantOrResolveArg, class ...NextVariantsAndResolvedArgs>
	constexpr decltype(auto) apply( F &&f, VariantOrResolveArg &&varg, NextVariantsAndResolvedArgs &&...nvargs ) const {
		if constexpr( N == sizeof...(VariantsAndResolvedArgs) + 1 )
			return std::forward<F>( f )(
				std::forward<VariantOrResolveArg>( varg ),
				std::forward<VariantsAndResolvedArgs>( vargs )...
			);
		else {
			constexpr iterate = [&f, &args...] ( auto &&x ) {
				return apply<N + 1>(
					std::forward<F>( f ),
					std::forward<NextVariantsAndResolvedArgs>( args )...,
					std::forward<decltype(x)>( x )
				);
			};
			
			using dispatch_tag = tag_of_t<VariantOrResolveArg>;
			return default_variant_t<dispatch_tag>::visit(
				iterate,
				std::forward<VariantOrResolveArg>( varg )
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
static constexpr dispatch_visit visit;

template<class NodeData> struct dispatch_get_alternative {
	template<class Variant>
	constexpr decltype(auto) apply( Variant &&v ) const {
		using dispatch_tag = tag_of_t<Variant>;
		return default_variant_v<dispatch_tag>::get<NodeData>( std::forward<Variant>( v ) );
	}
};
static constexpr dispatch_get_alternative get_alternative;
}

#endif //CXXMATH_CONCEPTS_VARIANT_HPP
