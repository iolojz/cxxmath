//
// Created by jayz on 08.11.19.
//

#ifndef CXXMATH_HELPERS_WRAP_TEMPLATE_MEMBERS_HPP
#define CXXMATH_HELPERS_WRAP_TEMPLATE_MEMBERS_HPP

#define CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( template_name ) \
template<class ...Args> static constexpr auto template_name ## _v = template_name<Args...>::value;

#define CXXMATH_DEFINE_TYPE_ALIAS_TEMPLATE( template_name ) \
template<class ...Args> using template_name ## _t = typename template_name<Args...>::type;

#endif //CXXMATH_HELPERS_WRAP_TEMPLATE_MEMBERS_HPP
