//
// Created by jayz on 17.03.20.
//

#ifndef CXXMATH_CONCEPTS_TREE_HPP
#define CXXMATH_CONCEPTS_TREE_HPP

namespace cxxmath {
namespace concepts {
template<class Data, class IsTerminal, class Arity, class Children>
struct tree_node {
	static constexpr auto data = function_object_v<Data>;
	static constexpr auto is_terminal = function_object_v<IsTerminal>;
	static constexpr auto arity = function_object_v<Arity>;
	static constexpr auto children = function_object_v<Children>;
};

template<class> struct is_tree_node: std::false_type {};
template<class Data, class IsTerminal, class Arity, class Children>
struct is_tree_node<tree_node<Data, IsTerminal, Arity, Children>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_tree_node)

template<class Variant, class TreeNode>
struct tree {
	static_assert( is_variant_v<Variant>, "Given Variant is not a variant" );
	using tree_node_for_node_data = TreeNodeForNodeData;
private:
	struct get_node_impl {
		template<class NodeData, class Var>
		static constexpr decltype(auto) apply( Var &&v ) {
			constexpr auto visitor = [] ( auto &&node ) {
				using data_type = std::decay_t<decltype(TreeNode::data( std::forward<decltype(node)>( node ) ))>;
				if constexpr( std::is_same_v<data_type, NodeData> )
					return node;
				else
					throw std::bad_variant_access{};
			};
			return visit( visitor, std::forward<Var>( v ) );
			
			return Variant::get<typename tree_node_for_node_data<NodeData>::type>( std::forward<Var>( v ) );
		}
	};
public:
	using variant = Variant;
	static constexpr auto visit = Variant::visit;
	static constexpr auto get_node = function_object_v<get_node_impl>;
};

template<class> struct is_tree: std::false_type {};
template<class Variant, class TreeNode>
struct is_tree<tree<Variant, TreeNode>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_tree)
}

template<class Type, class TreeNode>
struct type_models_concept<Type, TreeNode, std::enable_if_t<concepts::is_tree_node_v<TreeNode>>> {
	static constexpr bool value = (
		std::is_invocable_v<decltype(TreeNode::data), Type> &&
		std::is_invocable_v<decltype(TreeNode::is_terminal), Type>
		// FIXME: How do we conditionally enable these for constexpr non-terminals?
		/*CXXMATH_IS_VALID( TreeNode::arity, std::declval<Type>() ),
		CXXMATH_IS_VALID( TreeNode::children, std::declval<Type>() )*/
	);
};

template<class Type, class Tree>
struct type_models_concept<Type, Tree, std::enable_if_t<concepts::is_tree_v<Tree>>> {
private:
	struct is_tree_node_visitor {
		template<class T> constexpr std::true_type operator()( T && ) const {
			return std::bool_constant<type_models_concept_v<T, Tree::TreeNode>>{};
		}
	};
public:
	static constexpr bool value = (
		std::is_invocable_v<decltype(Tree::visit), is_tree_node_visitor, Type>
	);
};

CXXMATH_DEFINE_CONCEPT( tree_node )
CXXMATH_DEFINE_CONCEPT( tree )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION(is_terminal, tree_node)
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION(arity, tree_node)
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION(children, tree_node)

template<class NodeData> struct dispatch_get_node {
	template<class Variant>
	constexpr decltype(auto) apply( Variant &&v ) const {
		using dispatch_tag = tag_of_t<Variant>;
		return default_variant_v<dispatch_tag>::get_node<NodeData>( std::forward<Variant>( v ) );
	}
};
static constexpr dispatch_get_node get_node;
}

#endif //CXXMATH_CONCEPTS_TREE_HPP
