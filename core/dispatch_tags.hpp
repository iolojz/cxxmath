//
// Created by jayz on 24.09.19.
//

#ifndef CXXMATH_CORE_DISPATCH_TAGS_HPP
#define CXXMATH_CORE_DISPATCH_TAGS_HPP

#include "../helpers/has_member_type.hpp"
#include "../helpers/wrap_template_members.hpp"

namespace cxxmath
{
namespace detail
{
template<class, class = void>
struct has_dispatch_tag : std::false_type
{
};

template<class T>
struct has_dispatch_tag<T, std::void_t<typename T::cxxmath_dispatch_tag>> : std::true_type
{
};

template<class T, bool>
struct tag_of;

template<class T>
struct tag_of<T, true>
{
	using type = typename T::cxxmath_dispatch_tag;
};

template<class T>
struct tag_of<T, false>
{
	using type = std::decay_t<T>;
};
}

namespace impl
{
template<class T>
struct tag_of
{
	using type = typename detail::tag_of<T, detail::has_dispatch_tag<T>::value>::type;
};

template<class Tag1, class Tag2, class = void>
struct common_tag {};
template<class Tag1, class Tag2>
struct common_tag<Tag1, Tag2, std::enable_if_t<std::is_same_v<Tag1, Tag2>>>
{
	using type = Tag1;
};
}

template<class T>
struct tag_of
{
	using type = typename impl::tag_of<std::decay_t<T>>::type;
};
CXXMATH_DEFINE_TYPE_ALIAS_TEMPLATE(tag_of)

template<class ...Tags>
struct common_tag;
CXXMATH_DEFINE_TYPE_ALIAS_TEMPLATE(common_tag)

template<class Tag>
struct common_tag<Tag>
{
	using type = Tag;
};

namespace detail {
template<class Tag1, class Tag2, class TagTuple, class = void> struct common_tag {};

template<class Tag1, class Tag2, class ...Tags>
struct common_tag<Tag1, Tag2, std::tuple<Tags...>, std::enable_if_t<has_member_type<impl::common_tag<Tag1, Tag2>>::value>>
{
	using type = ::cxxmath::common_tag_t<typename impl::common_tag<Tag1, Tag2>::type, Tags...>;
};
}

template<class Tag1, class Tag2, class ...Tags>
struct common_tag<Tag1, Tag2, Tags...> : detail::common_tag<Tag1, Tag2, std::tuple<Tags...>> {};

template<class ...Args> struct have_common_tag {
	static constexpr bool value = detail::has_member_type<common_tag<tag_of_t<Args>...>>::value;
};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(have_common_tag)

template<class DispatchTag>
struct supports_tag_helper
{
	template<class Tag>
	static constexpr bool supports_tag( void )
	{
		return std::is_same_v<DispatchTag, Tag>;
	}
};

template<class ...Implementations>
struct forward_supported_tags
{
	template<class Tag>
	static constexpr bool supports_tag( void )
	{
		return (Implementations::template supports_tag<Tag>() && ... && true);
	}
};
}

#ifdef CXXMATH_ENABLE_IF_TAG_IS
#error "CXXMATH_ENABLE_IF_TAG_IS is already defined."
#endif
#define CXXMATH_ENABLE_IF_TAG_IS( type, tag ) \
class = std::enable_if_t<std::is_same_v<::cxxmath::tag_of_t<type>, tag>>

#ifdef CXXMATH_DISABLE_IF_TAG_IS
#error "CXXMATH_DISABLE_IF_TAG_IS is already defined."
#endif
#define CXXMATH_DISABLE_IF_TAG_IS( type, tag ) \
class = std::enable_if_t<!std::is_same_v<::cxxmath::tag_of_t<type>, tag>>

#endif //CXXMATH_CORE_DISPATCH_TAGS_HPP
