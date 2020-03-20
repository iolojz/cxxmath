//
// Created by jayz on 17.03.20.
//

#ifndef CXXMATH_CONCEPTS_MAPPING_PRESCRIPTION_HPP
#define CXXMATH_CONCEPTS_MAPPING_PRESCRIPTION_HPP

#include "comparable.hpp"
#include "tree.hpp"

namespace cxxmath {
namespace concepts {
template<class Tree, class TerminalNodeDataIsArgument>
struct mapping_prescription {
	static_assert( is_tree_v<Tree>, "template parameter 'Tree' is not a tree." );
	using tree = Tree;
	
	static constexpr auto visit = Tree::visit;
	static constexpr auto get_node = Tree::get_node;
	static constexpr auto terminal_node_data_is_argument = function_object_v<TerminalNodeDataIsArgument>;
};

template<class> struct is_mapping_prescription: std::false_type {};
template<class Tree, class TerminalNodeDataIsArgument>
struct is_mapping_prescription<mapping_prescription<Tree, TerminalNodeDataIsArgument>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE( is_mapping_prescription )
}

template<class Type, class MappingPrescription>
struct type_models_concept<
	Type,
	MappingPrescription,
	std::enable_if_t<concepts::is_mapping_prescription_v<MappingPrescription>>
> {
private:
	struct node_is_argument_visitor {
		template<class Node>
		constexpr void operator()( Node &&node ) const {
			[[maybe_unused]] auto x = MappingPrescription::terminal_node_data_is_argument(
				MappingPrescription::tree::tree_node::data( std::forward<Node>( node ) )
			);
		}
	};
public:
	static constexpr bool value = (
		models_concept_v<Type, typename MappingPrescription::tree> &&
		std::is_invocable_v<decltype(MappingPrescription::tree::visit), node_is_argument_visitor, Type>
	);
};

CXXMATH_DEFINE_CONCEPT( mapping_prescription )

template<class MappingPrescriptionTree> using mapping_prescription_tree = typename MappingPrescriptionTree::tree;

namespace impl {
template<class DispatchTag>
struct default_tree<DispatchTag, std::enable_if_t<has_default_mapping_prescription_v<DispatchTag>>> {
	using type = mapping_prescription_tree<default_mapping_prescription_t<DispatchTag>>;
};
}
}

#endif //CXXMATH_CONCEPTS_MAPPING_PRESCRIPTION_HPP
