//
// Created by jayz on 08.11.19.
//

#ifndef CXXMATH_CORE_HAS_MEMBER_TYPE_HPP
#define CXXMATH_CORE_HAS_MEMBER_TYPE_HPP

namespace cxxmath
{
namespace detail
{
template<class, class = void>
struct has_member_type : std::false_type
{
};

template<class T>
struct has_member_type<T, std::void_t<typename T::type>> : std::true_type
{
};
}
}

#endif //CXXMATH_CORE_HAS_MEMBER_TYPE_HPP
