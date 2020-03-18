//
// Created by jayz on 17.03.20.
//

#ifndef CXXMATH_CONCEPTS_MAPPING_PRESCRIPTION_HPP
#define CXXMATH_CONCEPTS_MAPPING_PRESCRIPTION_HPP

#include "comparable.hpp"
#include "tree.hpp"

namespace cxxmath {
namespace concepts {
template<class Tree, class IsArgument>
struct mapping_prescription {
	static_assert( is_tree_v<Tree>, "template parameter 'Tree' is not a tree." );
	
	using tree = Tree;
	
	static constexpr auto is_terminal = tree::is_terminal;
	static constexpr auto arity = tree::arity;
	static constexpr auto children = tree::children;
	static constexpr auto make_node = tree::make_node;
	
	static constexpr auto is_argument = function_object_v<IsArgument>;
};

template<class> struct is_mapping_prescription: std::false_type {};
template<class Tree, class IsArgument>
struct is_mapping_prescription<mapping_prescription<Tree, IsArgument>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( is_mapping_prescription )
}

template<class DispatchTag, class MappingPrescription>
struct models_concept<
	DispatchTag,
	MappingPrescription,
	std::enable_if_t < concepts::is_mapping_prescription_v<MappingPrescription>>
> {
static constexpr bool value = (
	models_concept_v<DispatchTag, typename MappingPrescription::tree>
);
};

CXXMATH_DEFINE_CONCEPT( mapping_prescription )

template<class MappingPrescriptionTree> using mapping_prescription_tree = typename MappingPrescriptionTree::tree;

namespace impl {
template<class DispatchTag>
struct default_tree<DispatchTag, std::enable_if_t < has_default_mapping_prescription_v<DispatchTag>>> {
using type = mapping_prescription_tree<default_mapping_prescription_t<DispatchTag>>;
};
}
}

#endif //CXXMATH_CONCEPTS_MAPPING_PRESCRIPTION_HPP
