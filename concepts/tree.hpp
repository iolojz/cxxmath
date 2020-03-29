//
// Created by jayz on 17.03.20.
//

#ifndef CXXMATH_CONCEPTS_TREE_HPP
#define CXXMATH_CONCEPTS_TREE_HPP

#include "variant.hpp"

#include <boost/range/adaptors.hpp>

namespace cxxmath {
namespace concepts {
namespace detail {
template<class F>
struct invoke_result_impl {
	template<class ...Args>
	constexpr auto operator()( Args &&...args ) const {
		return boost::hana::type_c<
			std::invoke_result_t<F, typename decltype(+std::forward<Args>( args ))::type...>
		>;
	}
};
template<class F>
static constexpr invoke_result_impl<F> invoke_result;
}

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
	static_assert( is_tree_node_v<TreeNode>, "Given TreeNode is not a tree node" );
	using variant = Variant;
	using tree_node = TreeNode;
private:
	struct is_terminal_type {
		template<class T>
		constexpr decltype(auto) operator()( boost::hana::basic_type<T> ) const {
			return decltype(tree_node::is_terminal( std::declval<T>() )){};
		}
	};
	struct data_type {
		template<class T>
		constexpr auto operator()( boost::hana::basic_type<T> ) const {
			return boost::hana::type_c<decltype(tree_node::data( std::declval<T>() )) >;
		}
	};
	
	template<class NodeData>
	struct get_node_impl {
		template<class Var>
		static constexpr decltype(auto) apply( Var &&v ) {
			constexpr auto nodes_for_node_data = boost::hana::filter(
				variant::types( v ),
				[] ( auto &&node_type ) {
					using data_type = std::decay_t<decltype(TreeNode::data(
						std::declval<typename decltype(+node_type)::type>() )
					)>;
					if constexpr( std::is_same_v<data_type, NodeData> )
						return boost::hana::true_c;
					else
						return boost::hana::false_c;
				}
			);
			static_assert( boost::hana::value( boost::hana::length( nodes_for_node_data ) ) == 1 );
			using type = typename decltype(+boost::hana::front( nodes_for_node_data ))::type;
			
			return Variant::template get_alternative<type>( std::forward<Var>( v ) );
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
	struct recursive_tree_transform_impl {
		template<class Tree, class F>
		static constexpr auto apply( Tree &&tree, F &&f )
		-> typename decltype(
			+boost::hana::fold_left(
				boost::hana::transform(
					boost::hana::transform(
						boost::hana::filter(
							Variant::types( tree ),
							is_terminal_type{}
						),
						data_type{}
					),
					detail::invoke_result<F>
				),
				boost::hana::metafunction<boost::hana::common>
			)
		)::type {
			auto visitor = [f] ( auto &&node ) {
				if constexpr( tree_node::is_terminal( node ) )
					return f( tree_node::data( node ) );
				else {
					auto recurse = [f] ( auto &&child_tree ) {
						return apply( std::forward<decltype(child_tree)>( child_tree ), f );
					};
					
					return f(
						tree_node::data( node ),
						boost::adaptors::transform( tree_node::children( node ), recurse )
					);
				}
			};
			return visit( visitor, std::forward<Tree>( tree ) );
		}
	};
public:
	static constexpr auto visit = Variant::visit;
	
	template<class NodeData>
	static constexpr auto get_node = static_function_object<get_node_impl<NodeData>>;
	template<class NodeData>
	static constexpr auto holds_node = static_function_object<holds_node_impl<NodeData>>;
	static constexpr auto recursive_tree_transform = static_function_object<recursive_tree_transform_impl>;
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
	constexpr decltype(auto) operator()( Variant &&v ) const {
		using dispatch_tag = tag_of_t<Variant>;
		return default_tree_t<dispatch_tag>::template get_node<NodeData>( std::forward<Variant>( v ) );
	}
};
template<class NodeData>
static constexpr dispatch_get_node<NodeData> get_node;

template<class NodeData> struct dispatch_holds_node {
	template<class Variant>
	constexpr decltype(auto) operator()( Variant &&v ) const {
		using dispatch_tag = tag_of_t<Variant>;
		return default_tree_t<dispatch_tag>::template holds_node<NodeData>( std::forward<Variant>( v ) );
	}
};
template<class NodeData>
static constexpr dispatch_holds_node<NodeData> holds_node;

struct dispatch_recursive_tree_transform {
	template<class Tree, class F>
	constexpr decltype(auto) operator()( Tree &&tree, F &&f ) const {
		using dispatch_tag = tag_of_t<Tree>;
		return default_tree_t<dispatch_tag>::recursive_tree_transform(
			std::forward<Tree>( tree ),
			std::forward<F>( f )
		);
	}
};
static constexpr dispatch_recursive_tree_transform recursive_tree_transform;
}

#endif //CXXMATH_CONCEPTS_TREE_HPP
