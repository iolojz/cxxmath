//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CORE_CONCEPTS_HPP
#define CXXMATH_CORE_CONCEPTS_HPP

#include "helpers/wrap_template_helpers.hpp"

namespace cxxmath
{
template<class DispatchTag, class Concept>
struct models_concept : std::false_type
{
};
DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(models_concept)

namespace impl
{
struct no_default_concept
{
};
}
}

#ifdef CXXMATH_DEFINE_CONCEPT
#error "CXXMATH_DEFINE_CONCEPT is already defined"
#endif
#define CXXMATH_DEFINE_CONCEPT( concept ) \
namespace impl { \
template<class Tag, class = void> struct default_ ## concept : no_default_concept {}; \
} \
\
template<class Tag> static constexpr bool has_default_ ## concept ## _v = \
    !std::is_base_of_v<impl::no_default_concept, impl::default_ ## concept<Tag>>; \
\
template<class Tag> \
struct default_ ## concept \
{ \
    static_assert( has_default_ ## concept ## _v<Tag>, "No default " #concept " for 'Tag' implemented." ); \
    using type = typename impl::default_ ## concept<Tag>::type; \
}; \
DEFINE_TYPE_ALIAS_TEMPLATE(default_ ## concept ## _t)

#endif //CXXMATH_CORE_MODELS_CONCEPT_HPP
