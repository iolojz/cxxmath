//
// Created by jayz on 03.10.19.
//

#ifndef CXXMATH_MODELS_STD_STRING_VIEW_HPP
#define CXXMATH_MODELS_STD_STRING_VIEW_HPP

namespace cxxmath
{
template<class CharType, class Traits = std::char_traits <CharType>>
struct std_string_view_tag;

namespace impl
{
template<class CharType, class Traits>
struct tag_of<std::basic_string_view<CharType, Traits>>
{
	using type = std_string_view_tag<CharType, Traits>;
};
}

namespace model_std_string_view
{
struct less
{
	template<class StringView1, class StringView2>
	static constexpr bool apply( StringView1 &&s1, StringView2 &&s2 )
	{
		return std::forward<StringView1>( s1 ) < std::forward<StringView2>( s2 );
	}
};
}

namespace impl {
template<class CharType, class Traits>
struct default_total_order<std_string_view_tag<CharType, Traits>>
{
	using type = concepts::total_order<model_std_string_view::less>;
};
}
}

#endif //CXXMATH_MODELS_STD_STRING_VIEW_HPP
