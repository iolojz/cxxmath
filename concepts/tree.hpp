//
// Created by jayz on 17.03.20.
//

#ifndef CXXMATH_CONCEPTS_TREE_HPP
#define CXXMATH_CONCEPTS_TREE_HPP

namespace cxxmath {
namespace concepts {
template<class IsTerminal, class Arity, class Children>
struct tree_node {
	static constexpr auto is_terminal = function_object_v<IsTerminal>;
	static constexpr auto arity = function_object_v<Arity>;
	static constexpr auto children = function_object_v<Children>;
};

template<class> struct is_tree_node: std::false_type {};
template<class IsTerminal, class Arity, class Children>
struct is_tree_node<tree_node<IsTerminal, Arity, Children>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_tree_node)

template<class IsTerminal, class Arity, class ApplyToChildren, class MakeNode>
struct tree {
	static constexpr auto visit_node = function_object_v<ApplyToChildren>;
};

template<class> struct is_tree: std::false_type {};
template<class IsTerminal, class Arity, class Children, class MakeNode>
struct is_tree<tree<IsTerminal, Arity, Children, MakeNode>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_tree)
}

template<class DispatchTag, class Tree>
struct models_concept<DispatchTag, Tree, std::enable_if_t < concepts::is_tree_v<Tree>>> {
static constexpr bool value = (
	Tree::is_terminal.template supports_tag<DispatchTag>() &&
		Tree::arity.template supports_tag<DispatchTag>()
);
};

CXXMATH_DEFINE_CONCEPT( tree )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( is_terminal, tree
)
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( arity, tree
)

struct default_apply_to_children_dispatch {
	template<class Tree, class F>
	constexpr decltype( auto ) operator()( Tree &&tree, F &&f ) const {
		using dispatch_tag = tag_of_t<Tree>;
		return default_tree_t<dispatch_tag>::apply_to_children( std::forward<Tree>( tree ), std::forward<F>( f ) );
	}
};

static constexpr default_apply_to_children_dispatch apply_to_children;
}

#endif //CXXMATH_CONCEPTS_TREE_HPP
