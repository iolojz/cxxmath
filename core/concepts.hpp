//
// Created by jayz on 27.09.19.
//

#ifndef CXXMATH_CORE_MODELS_CONCEPT_HPP
#define CXXMATH_CORE_MODELS_CONCEPT_HPP

#include "function_dispatch.hpp"

namespace cxxmath
{
template<class Concept, class ...Tags>
struct modelled_concept : std::false_type
{
};
template<class Concept, class ...Tags> static constexpr bool modelled_concept_v = modelled_concept<Concept, Tags...>::value;

namespace impl
{
struct no_default_concept
{
};
}

#ifdef CXXMATH_DEFINE_CONCEPT
#error "CXXMATH_DEFINE_CONCEPT is already defined"
#endif
#define CXXMATH_DEFINE_CONCEPT( concept ) \
namespace impl { \
template<class Tags, class = void> struct default_ ## concept : no_default_concept {}; \
} \
\
template<class Tag> static constexpr bool has_default_ ## concept = \
    !std::is_base_of_v<impl::no_default_concept, impl::default_ ## concept<Tag>>; \
\
template<class Tag> \
struct default_ ## concept \
{ \
    static_assert( has_default_ ## concept<Tag>, "No default " #concept " for 'Tag' implemented." ); \
    using type = typename impl::default_ ## concept<Tag>::type; \
}; \
template<class Tag> using default_ ## concept ## _t = typename default_ ## concept<Tag>::type;
}

#endif //CXXMATH_CORE_MODELS_CONCEPT_HPP
