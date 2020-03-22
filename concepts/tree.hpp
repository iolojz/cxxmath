//
// Created by jayz on 17.03.20.
//

#ifndef CXXMATH_CONCEPTS_TREE_HPP
#define CXXMATH_CONCEPTS_TREE_HPP

#include "variant.hpp"

namespace cxxmath {
namespace concepts {
template<class Data, class IsTerminal, class Arity, class Children>
struct tree_node {
	static constexpr auto data = static_function_object<Data>;
	static constexpr auto is_terminal = static_function_object<IsTerminal>;
	static constexpr auto arity = static_function_object<Arity>;
	static constexpr auto children = static_function_object<Children>;
};

template<class> struct is_tree_node: std::false_type {};
template<class Data, class IsTerminal, class Arity, class Children>
struct is_tree_node<tree_node<Data, IsTerminal, Arity, Children>>: std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_tree_node)

template<class Variant, class TreeNode>
struct tree {
	static_assert( is_variant_v<Variant>, "Given Variant is not a variant" );
	using tree_node = TreeNode;
private:
	template<class NodeData>
	struct get_node_impl {
		template<class Var>
		static constexpr decltype(auto) apply( Var &&v ) {
			constexpr auto predicate = [] ( auto &&node ) {
				using data_type = std::decay_t<decltype(TreeNode::data( std::forward<decltype(node)>( node ) ))>;
				if constexpr( std::is_same_v<data_type, NodeData> )
					return boost::hana::true_c;
				else
					return boost::hana::false_c;
			};
			return Variant::get_alternative_with_predicate( predicate, std::forward<Var>( v ) );
		}
	};
	template<class NodeData>
	struct holds_node_impl {
		template<class Var>
		static constexpr decltype(auto) apply( Var &&v ) {
			constexpr auto visitor = [] ( auto &&node ) {
				using data_type = std::decay_t<decltype(TreeNode::data( std::forward<decltype(node)>( node ) ))>;
				if constexpr( std::is_same_v<data_type, NodeData> )
					return true;
				else
					return false;
			};
			return visit( visitor, std::forward<Var>( v ) );
		}
	};
public:
	using variant = Variant;
	static constexpr auto visit = Variant::visit;
	
	template<class NodeData>
	static constexpr auto get_node = static_function_object<get_node_impl<NodeData>>;
	template<class NodeData>
	static constexpr auto holds_node = static_function_object<holds_node_impl<NodeData>>;
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
		return default_tree_t<dispatch_tag>::template get_node<NodeData>( std::forward<Variant>( v ) );
	}
};
template<class NodeData>
static constexpr dispatch_get_node<NodeData> get_node;

template<class NodeData> struct dispatch_holds_node {
	template<class Variant>
	constexpr decltype(auto) apply( Variant &&v ) const {
		using dispatch_tag = tag_of_t<Variant>;
		return default_tree_t<dispatch_tag>::template holds_node<NodeData>( std::forward<Variant>( v ) );
	}
};
template<class NodeData>
static constexpr dispatch_holds_node<NodeData> holds_node;
}

#endif //CXXMATH_CONCEPTS_TREE_HPP
