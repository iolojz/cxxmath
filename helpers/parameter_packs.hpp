//
// Created by jayz on 18.03.20.
//

#ifndef CXXMATH_HELPERS_PARAMETER_PACKS_HPP
#define CXXMATH_HELPERS_PARAMETER_PACKS_HPP

namespace cxxmath {
template<std::size_t N, class T, class ...Types> struct template_at_c {
	static constexpr decltype(auto) apply( T &&t, Types &&...types ) {
		return template_at_c<N - 1, Types...>( std::forward<Types>( types )... );
	}
	using type = typename at_c<N - 1, Types...>::type;
};

template<class T, class ...Types> struct template_at_c<0, T, Types...> {
	static constexpr decltype(auto) apply( T &&t, Types &&...types ) {
		return std::forward<T>( t );
	}
	using type = T;
};
CXXMATH_DEFINE_TYPE_ALIAS_TEMPLATE(template_at_c)

template<std::size_t N, class T, class ...Types> decltype(auto) function_at_c( T &&t, Types &&...types ) {
	return template_at_c<N, T, Types...>::apply( std::forward<T>( t ), std::forward<Types>( types )... );
}
}

#endif //CXXMATH_HELPERS_PARAMETER_PACKS_HPP
