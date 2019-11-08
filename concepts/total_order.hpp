//
// Created by jayz on 03.10.19.
//

#ifndef CXXMATH_CONCEPTS_TOTAL_ORDER_HPP
#define CXXMATH_CONCEPTS_TOTAL_ORDER_HPP

#include "core/concepts.hpp"
#include "models/function_object.hpp"

namespace cxxmath
{
namespace impl
{
namespace concepts {
template<class Less>
class total_order
{
	template<class Less>
	struct less_equal_impl : forward_supported_tags<Less>
	{
		template<class Arg1, class Arg2>
		static constexpr decltype( auto ) apply( const Arg1 &arg1, const Arg2 &arg2 )
		{
			return not_( Less::apply( arg2, arg1 ));
		}
	};
	
	template<class Less>
	struct equal_impl : forward_supported_tags<Less>
	{
		template<class Arg1, class Arg2>
		static constexpr decltype( auto ) apply( const Arg1 &arg1, const Arg2 &arg2 )
		{
			return and_( not_( Less::apply( arg1, arg2 )), not_( Less::apply( arg2, arg1 )));
		}
	};
	
	template<class Less>
	struct greater_equal_impl : forward_supported_tags<Less>
	{
		template<class Arg1, class Arg2>
		static constexpr decltype( auto ) apply( const Arg1 &arg1, const Arg2 &arg2 )
		{
			return not_( Less::apply( arg1, arg2 ));
		}
	};
	
	template<class Less>
	struct greater_impl : forward_supported_tags<Less>
	{
		template<class Arg1, class Arg2>
		static constexpr decltype( auto ) apply( const Arg1 &arg1, const Arg2 &arg2 )
		{
			return Less::apply( arg2, arg1 );
		}
	};
public:
	static constexpr auto less = function_object_v<Less>;
	
	static constexpr auto less_equal = function_object_v<less_equal_impl<Less>>;
	static constexpr auto equal = function_object_v<equal_impl<Less>>;
	static constexpr auto greater_equal = function_object_v<greater_equal_impl<Less>>;
	static constexpr auto greater = function_object_v<greater_impl<Less>>;
	
	static constexpr auto not_equal = compose( not_, equal );
};
}

template<class DispatchTag, class Less>
struct models_concept<DispatchTag, concepts::total_order<Less>>
{
	using total_order = concepts::total_order<Less>;
	static constexpr bool value = total_order::less.template supports_tag<DispatchTag>();
};

CXXMATH_DEFINE_CONCEPT( total_order )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( less, total_order )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( less_equal, total_order )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( greater, total_order )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( greater_equal, total_order )

template<class Less> using totally_ordered_set = concepts::set<concepts::detail::equal_total_order<Less>>;

namespace impl
{
template<class DispatchTag>
struct default_set<DispatchTag, std::enable_if_t<has_default_total_order<DispatchTag>>>
{
	using type = totally_ordered_set<default_total_order_t<DispatchTag>>;
};
}
}

#endif //CXXMATH_CONCEPTS_TOTAL_ORDER_HPP
