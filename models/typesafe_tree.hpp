//
// Created by jayz on 17.03.20.
//

#ifndef CXXMATH_MODELS_TYPESAFE_TREE_HPP
#define CXXMATH_MODELS_TYPESAFE_TREE_HPP

#include "../concepts/mapping_prescription.hpp"

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>
#include <boost/hana/ext/std/integral_constant.hpp>

#include "boost_variant.hpp"
#include "../helpers/boost_variant.hpp"

namespace cxxmath {
struct runtime_arity_t {};
static constexpr auto runtime_arity = runtime_arity_t{};

template<class NodeDataTypes, class Arities> class typesafe_tree;

template<class T> struct is_typesafe_tree: std::false_type {};
template<class NodeDataTypes, class Arities>
struct is_typesafe_tree<typesafe_tree<NodeDataTypes, Arities>>: std::true_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_typesafe_tree)

template<class NodeDataTypes, class Arities> struct typesafe_tree_tag {
	using tree_type = typesafe_tree<NodeDataTypes, Arities>;
};
template<class TypesafeTree> struct typesafe_tree_node_tag {
	static_assert( is_typesafe_tree_v<TypesafeTree> );
	using tree_type = TypesafeTree;
};

template<class T> struct is_typesafe_tree_tag: std::false_type {};
template<class NodeDataTypes, class Arities>
struct is_typesafe_tree_tag<typesafe_tree_tag<NodeDataTypes, Arities>>: std::true_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_typesafe_tree_tag)

template<class NodeData, class TypesafeTree, class = void> class typesafe_tree_node;

template<class T> struct is_typesafe_tree_node : std::false_type {};
template<class NodeDataTypes, class Arities>
struct is_typesafe_tree_node<typesafe_tree_node<NodeDataTypes, Arities>> : std::true_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_typesafe_tree_node)

template<class NodeData, class TypesafeTree>
class typesafe_tree_node<
	NodeData,
	TypesafeTree,
	std::enable_if_t<TypesafeTree::is_terminal_node_type( boost::hana::type_c<NodeData> )>
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
	>
	typesafe_tree_node( Data &&d ) : data{ std::forward<Data>( d ) } {}
	
	bool operator==( const typesafe_tree_node &other ) const {
		return data == other.data;
	}
	bool operator!=( const typesafe_tree_node &other ) const {
		return data != other.data;
	}
	
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
	static_assert( boost::hana::length( node_data_types ) == boost::hana::length( arities ) );
	
	static constexpr auto arity_map = boost::hana::unpack(
		boost::hana::zip_with(
			boost::hana::make_pair,
			node_data_types,
			arities
		),
		boost::hana::make_map
	);
public:
	template<class NodeData>
	static constexpr auto arity_of_node_type( boost::hana::basic_type<NodeData> ) {
		return boost::hana::at_key( arity_map, boost::hana::type_c<NodeData> );
	}
	
	template<class NodeData>
	static constexpr bool is_terminal_node_type( boost::hana::basic_type<NodeData> ) {
		constexpr auto arity_ = arity_of_node_type( boost::hana::type_c<NodeData> );
		if constexpr( boost::hana::typeid_( arity_ ) == boost::hana::type_c<runtime_arity_t> ) {
			return false;
		} else if constexpr( arity_ != 0 ) {
			return false;
		} else {
			return true;
		}
	}
private:
	static constexpr auto node_in_tree = [] ( auto &&t ) {
		return boost::hana::type_c<typesafe_tree_node<typename decltype(+t)::type, typesafe_tree>>;
	};
	static constexpr auto wrap_if_nonterminal = [] ( auto &&t ) {
		if constexpr( is_terminal_node_type( boost::hana::type_c<typename decltype(+t)::type> ) ) {
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
		wrapped_nodes,
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
		class = std::enable_if_t<has_unique_constructible_alternative_v<node_variant, Args &&...>>
	>
	typesafe_tree( Args &&...args ) : node{
		unique_constructible_alternative_t<node_variant, Args &&...>{ std::forward<Args>( args )... }
	} {}
	
	typesafe_tree &operator=( typesafe_tree &&other ) = default;
	typesafe_tree &operator=( const typesafe_tree &other ) = default;
	
	bool operator==( const typesafe_tree &other ) const {
		return node == other.node;
	}
	bool operator!=( const typesafe_tree &other ) const {
		return node != other.node;
	}
	
	node_variant &variant( void ) & { return node; }
	const node_variant &variant( void ) const & { return node; }
	node_variant &&variant( void ) && { return std::move( node ); }
};

template<class NodeData, class TypesafeTree>
class typesafe_tree_node<
	NodeData,
	TypesafeTree,
	std::enable_if_t<!TypesafeTree::is_terminal_node_type( boost::hana::type_c<NodeData> )>
> {
	static_assert( is_typesafe_tree_v<TypesafeTree> , "Internal error" );
	static constexpr auto child_container_( void ) {
		constexpr auto arity = TypesafeTree::arity_of_node_type( boost::hana::type_c<NodeData> );
		
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
		class = std::enable_if_t<sizeof...(Children) != 0>
	> typesafe_tree_node( Data &&d, Children &&...ch )
	: data{ d }, children{ std::forward<Children>( ch )... } {}
	
	bool operator==( const typesafe_tree_node &other ) const {
		return data == other.data && children == other.children;
	}
	bool operator!=( const typesafe_tree_node &other ) const {
		return data != other.data || children != other.children;
	}
	
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
		return concepts::boost_variant::visit(
			std::forward<F>( f ),
			std::forward<TypesafeTree>( tree ).variant()
		);
	}
};

struct get_alternative_with_predicate {
	template<class UnaryPredicate, class TypesafeTree>
	static constexpr decltype(auto) apply( UnaryPredicate &&p, TypesafeTree &&tree ) {
		return concepts::boost_variant::get_alternative_with_predicate(
			std::forward<UnaryPredicate>( p ),
			std::forward<TypesafeTree>( tree ).variant()
		);
	}
};

struct equal {
	template<class TypesafeTree1, class TypesafeTree2>
	static constexpr decltype(auto) apply( TypesafeTree1 &&tst1, TypesafeTree2 &&tst2 ) {
		return std::forward<TypesafeTree1>( tst1 ) == std::forward<TypesafeTree2>( tst2 );
	}
};
}

namespace concepts {
template<class NodeDataTypes, class Arities>
using typesafe_tree_comparable = concepts::comparable<
	model_typesafe_tree::equal
>;

template<class NodeDataTypes, class Arities>
using typesafe_tree_variant = concepts::variant<
    model_typesafe_tree::visit,
    model_typesafe_tree::get_alternative_with_predicate
>;

template<class TypesafeTree>
using typesafe_tree_node_comparable = concepts::comparable<
	model_typesafe_tree::equal
>;
}

namespace impl {
template<class NodeDataTypes, class Arities>
struct default_comparable<typesafe_tree_tag<NodeDataTypes, Arities>> {
	using type = concepts::typesafe_tree_comparable<NodeDataTypes, Arities>;
};

template<class NodeDataTypes, class Arities>
struct default_variant<typesafe_tree_tag<NodeDataTypes, Arities>> {
	using type = concepts::typesafe_tree_variant<NodeDataTypes, Arities>;
};

template<class TypesafeTree>
struct default_comparable<typesafe_tree_node_tag<TypesafeTree>> {
	using type = concepts::typesafe_tree_node_comparable<TypesafeTree>;
};

template<class TypesafeTree>
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
	    concepts::typesafe_tree_variant<NodeDataTypes, Arities>,
	    concepts::tree_node<
			model_typesafe_tree::data,
			model_typesafe_tree::is_terminal,
			model_typesafe_tree::arity,
			model_typesafe_tree::children
		>
	>;
};

template<class TypesafeTreeTag>
struct make<TypesafeTreeTag, std::enable_if_t<is_typesafe_tree_tag_v<TypesafeTreeTag>>> {
	template<class ...Args> static constexpr decltype(auto) apply( Args &&...args ) {
		return typename TypesafeTreeTag::tree_type{ std::forward<Args>( args )... };
	}
};
}

template<class TypesafeTreeNode>
std::enable_if_t<is_typesafe_tree_node_v<TypesafeTreeNode>, std::ostream> &
operator<<( std::ostream &os, const TypesafeTreeNode &node ) {
	if constexpr( node.is_terminal() ) {
		return os << node.data;
	} else {
		os << node.data << " [ ";
		auto it = std::begin( node.children );
		
		if( it != std::end( node.children ) ) {
			os << *it;
			for( ++it; it != std::end( node.children ); ++it )
				os << ", " << *it;
		}
		
		return os;
	}
}

template<class TypesafeTree>
std::enable_if_t<is_typesafe_tree_v<TypesafeTree>, std::ostream> &
operator<<( std::ostream &os, const TypesafeTree &tst ) {
	return os << tst.variant();
}
}

#endif //CXXMATH_MODELS_TYPESAFE_TREE_HPP
