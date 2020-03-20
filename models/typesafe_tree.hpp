//
// Created by jayz on 17.03.20.
//

#ifndef CXXMATH_MODELS_TYPESAFE_TREE_HPP
#define CXXMATH_MODELS_TYPESAFE_TREE_HPP

#include "../concepts/mapping_prescription.hpp"

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>
#include <boost/hana/ext/std/integral_constant.hpp>

#include <boost/variant.hpp>

namespace cxxmath {
struct runtime_arity_t {};
static constexpr auto runtime_arity = runtime_arity_t{};

template<class NodeDataTypes, class Arities> class typesafe_tree;
template<class T, class TypesafeTree, class = void> class typesafe_tree_node;

template<class NodeDataTypes, class Arities> struct typesafe_tree_tag {
	using tree_type = typesafe_tree<NodeDataTypes, Arities>;
};
template<class TypesafeTree> struct typesafe_tree_node_tag {
	using tree_type = TypesafeTree;
};

template<class T> struct is_typesafe_tree: std::false_type {};
template<class NodeDataTypes, class Arities>
struct is_typesafe_tree<typesafe_tree<NodeDataTypes, Arities>>: std::true_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_typesafe_tree)

template<class NodeData, class TypesafeTree>
class typesafe_tree_node<
	NodeData,
	TypesafeTree,
	std::enable_if<TypesafeTree::is_terminal_node_type( boost::hana::type_c<NodeData> )>
> {
	static_assert( is_typesafe_tree_v<TypesafeTree> , "Internal error" );
public:
	using cxxmath_dispatch_tag = typesafe_tree_node_tag<TypesafeTree>;
	using tree = TypesafeTree;
	using node_data = NodeData;
	
	typesafe_tree_node( void ) = default;
	typesafe_tree_node( const typesafe_tree_node & ) = default;
	typesafe_tree_node( typesafe_tree_node && ) = default;
	
	typesafe_tree_node &operator=( const typesafe_tree_node & ) = default;
	typesafe_tree_node &operator=( typesafe_tree_node && ) = default;
	
	template<
		class Data,
		class = std::enable_if_t<std::is_constructible_v<node_data, Data &&>>
	> typesafe_tree_node( Data &&d ) : data{ std::forward<Data>( d ) } {}
	
	constexpr bool is_terminal( void ) const { return true; }
	constexpr std::size_t arity( void ) const { return 0; }
	
	node_data data;
};

template<class NodeDataTypes, class Arities> class typesafe_tree {
	static constexpr auto node_data_types = boost::hana::transform(
		NodeDataTypes{},
		boost::hana::typeid_
	);
	static constexpr auto arities = Arities{};
	
	static_assert( boost::hana::length( node_data_types ) != boost::hana::size_c<0>, "No node types were provided." );
	static_assert( boost::hana::length( node_data_types ) != boost::hana::length( arities ) );
	
	static constexpr auto arity_map = boost::hana::make_map(
		boost::hana::zip(
			node_data_types,
			arities
		)
	);
	
	static constexpr auto node_in_tree = []( auto &&t ) {
		return boost::hana::type_c<typesafe_tree_node<typename decltype(+t)::type, typesafe_tree>>;
	};
	static constexpr auto wrap_if_nonterminal = []( auto &&t ) {
		if constexpr( is_terminal( boost::hana::type_c<typename decltype(+t)::type> ) ) {
			return node_in_tree( boost::hana::type_c<typename decltype(+t)::type> );
		} else {
			return boost::hana::type_c<
				boost::recursive_wrapper<typename decltype(+node_in_tree( t ))::type>
			>;
		}
	};
	
	static constexpr auto wrapped_nodes = boost::hana::transform(
		node_data_types,
		wrap_if_nonterminal
	);
	
	using node_variant = typename decltype(+boost::hana::unpack(
		boost::hana::prepend(
			wrapped_nodes,
			boost::hana::type_c<boost::blank> // Make sure the variant is default-constructible
		),
		boost::hana::template_<boost::variant>
	))::type;
	node_variant node;
public:
	using cxxmath_dispatch_tag = typesafe_tree_tag<NodeDataTypes, Arities>;
	
	typesafe_tree( void ) = default;
	typesafe_tree( typesafe_tree &&other ) = default;
	typesafe_tree( const typesafe_tree &other ) = default;
	
	template<
	    class ...Args,
		class = std::enable_if_t<std::is_constructible_v<node_variant, Args &&...>>
	>
	typesafe_tree( Args &&...args ) : node{ std::forward<Args>( args )... } {}
	
	typesafe_tree &operator=( typesafe_tree &&other ) = default;
	typesafe_tree &operator=( const typesafe_tree &other ) = default;
	
	template<class NodeData>
	static constexpr auto arity_of_node_type( boost::hana::basic_type<NodeData> ) {
		return boost::hana::at_key( arity_map, boost::hana::type_c<NodeData> );
	}
	
	template<class NodeData>
	static constexpr bool is_terminal_node_type( boost::hana::basic_type<NodeData> ) {
		constexpr auto arity_ = arity( boost::hana::type_c<NodeData> );
		if constexpr( boost::hana::typeid_( arity_ ) == boost::hana::type_c<runtime_arity_t> ) {
			return false;
		} else if constexpr( arity_ != 0 ) {
			return false;
		} else {
			return true;
		}
	}
	
	template<class F> decltype( auto ) visit( F &&f ) && {
		return boost::apply_visitor( std::forward<F>( f ), std::move( node ) );
	}
	
	template<class F> decltype( auto ) visit( F &&f ) & {
		return boost::apply_visitor( std::forward<F>( f ), node );
	}
	
	template<class F> decltype( auto ) visit( F &&f ) const & {
		return boost::apply_visitor( std::forward<F>( f ), node );
	}
};

template<class NodeData, class TypesafeTree>
class typesafe_tree_node<
	NodeData,
	TypesafeTree,
	std::enable_if<!TypesafeTree::is_terminal_node_data( boost::hana::type_c<NodeData> )>
> {
	static_assert( is_typesafe_tree_v<TypesafeTree> , "Internal error" );
	static constexpr auto child_container_( void ) {
		constexpr auto arity = TypesafeTree::arity_for_node_data( boost::hana::type_c<NodeData> );
		
		if constexpr( boost::hana::typeid_( arity ) == boost::hana::type_c<runtime_arity_t> ) {
			return boost::hana::type_c<std::vector<TypesafeTree>>;
		} else {
			static_assert( arity != 0, "Internal error" );
			return boost::hana::type_c<std::array<TypesafeTree, arity>>;
		}
	}
public:
	using cxxmath_dispatch_tag = typesafe_tree_node_tag<TypesafeTree>;
	
	using tree = TypesafeTree;
	using node_data = NodeData;
	using child_container = typename decltype(+child_container_())::type;
	
	typesafe_tree_node( void ) = default;
	typesafe_tree_node( const typesafe_tree_node & ) = default;
	typesafe_tree_node( typesafe_tree_node && ) = default;
	
	typesafe_tree_node &operator=( const typesafe_tree_node & ) = default;
	typesafe_tree_node &operator=( typesafe_tree_node && ) = default;
	
	template<
		class Data,
		class ...Children,
		class = std::enable_if_t<std::is_constructible_v<node_data, Data &&>>,
		class = std::enable_if_t<std::is_constructible_v<child_container, Children &&...>>
	> typesafe_tree_node( Data &&d, Children &&...ch )
	: data{ d }, children{ std::forward<Children>( ch )... } {}
	
	constexpr bool is_terminal( void ) const { return false; }
	constexpr decltype(auto) arity( void ) const {
		return TypesafeTree::arity_for_node_data( boost::hana::type_c<NodeData> );
	}
	
	node_data data;
	child_container children;
};

namespace model_typesafe_tree {
struct data {
	template<class TypesafeTreeNode>
	static constexpr decltype(auto) apply( TypesafeTreeNode &&node ) {
		return std::forward<TypesafeTreeNode>( node ).data;
	}
};

struct children {
	template<class TypesafeTreeNode>
	static constexpr decltype(auto) apply( TypesafeTreeNode &&node ) {
		return std::forward<TypesafeTreeNode>( node ).children;
	}
};

struct is_terminal {
	template<class TypesafeTreeNode>
	static constexpr decltype(auto) apply( const TypesafeTreeNode &node ) {
		return node.is_terminal();
	}
};

struct arity {
	template<class TypesafeTreeNode>
	static constexpr decltype(auto) apply( const TypesafeTreeNode &node ) {
		return node.arity();
	}
};

struct visit {
	template<class F, class TypesafeTree>
	static constexpr decltype(auto) apply( F &&f, TypesafeTree &&tree ) {
		return std::forward<TypesafeTree>( tree ).visit( std::forward<F>( f ) );
	}
};

template<class TypesafeTreeTag>
struct make {
	template<class ...Args>
	static constexpr decltype(auto) apply( Args &&...args ) {
		return typename TypesafeTreeTag::tree_type{ std::forward<Args>( args )... };
	}
};
}

namespace impl {
template<class NodeDataTypes, class Arities>
struct default_tree<typesafe_tree_tag<NodeDataTypes, Arities>> {
	using type = concepts::variant<model_typesafe_tree::visit>;
};

template<TypesafeTree>
struct default_tree_node<typesafe_tree_node_tag<TypesafeTree>> {
	using type = concepts::tree_node<
		model_typesafe_tree::data,
		model_typesafe_tree::is_terminal,
		model_typesafe_tree::arity,
		model_typesafe_tree::children
	>;
};

template<class NodeDataTypes, class Arities>
struct default_tree<typesafe_tree_tag<NodeDataTypes, Arities>> {
	using type = concepts::tree<
	    concepts::variant<model_typesafe_tree::visit>,
	    concepts::tree_node<
			model_typesafe_tree::data,
			model_typesafe_tree::is_terminal,
			model_typesafe_tree::arity,
			model_typesafe_tree::children
		>
	>;
};
}
}

#endif //CXXMATH_MODELS_TYPESAFE_TREE_HPP
