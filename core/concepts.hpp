//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CORE_CONCEPTS_HPP
#define CXXMATH_CORE_CONCEPTS_HPP

#include "../helpers/wrap_template_members.hpp"

namespace cxxmath {
template<class Type, class Concept, class = void> struct type_models_concept : std::false_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(type_models_concept)

template<class ...Types> struct multitype {};

namespace detail {
struct no_default_concept {};
}
}


#ifdef CXXMATH_DEFINE_CONCEPT
#error "CXXMATH_DEFINE_CONCEPT is already defined"
#endif
#define CXXMATH_DEFINE_CONCEPT( concept_ ) \
namespace impl { \
template<class Tag, class = void> struct default_ ## concept_ : ::cxxmath::detail::no_default_concept {}; \
} \
\
template<class Tag> static constexpr bool has_default_ ## concept_ ## _v = \
    !std::is_base_of_v<::cxxmath::detail::no_default_concept, ::cxxmath::impl::default_ ## concept_<Tag>>; \
\
template<class Tag, class = std::enable_if_t<has_default_ ## concept_ ## _v<Tag>>> struct default_ ## concept_ { \
	using type = typename impl::default_ ## concept_<Tag>::type; \
}; \
CXXMATH_DEFINE_TYPE_ALIAS_TEMPLATE(default_ ## concept_)

#endif //CXXMATH_CORE_MODELS_CONCEPT_HPP
