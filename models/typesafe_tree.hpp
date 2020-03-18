//
// Created by jayz on 17.03.20.
//

#ifndef CXXMATH_MODELS_TYPESAFE_TREE_HPP
#define CXXMATH_MODELS_TYPESAFE_TREE_HPP

#include <boost/hana.hpp>

namespace cxxmath {
struct runtime_arity_t {};
static constexpr auto runtime_arity = runtime_arity_t{};

template<class NodeDataTypes, class Arities> class typesafe_tree;

template<class T, class TypesafeTree, class = void> class typesafe_tree_node;

template<class T> struct is_typesafe_tree: std::false_type {};
template<class NodeDataTypes, class Arities>
struct is_typesafe_tree<typesafe_tree<NodeDataTypes, Arities>>: std::true_type {};
CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_typesafe_tree)

template<class NodeData, class TypesafeTree>
class typesafe_tree_node<
	NodeData,
	TypesafeTree,
	std::enable_if<TypesafeTree::is_terminal( boost::hana::type_c<NodeData> )>
> {
	static_assert( is_typesafe_tree_v < TypesafeTree > , "Internal error" );
public:
	using tree = TypesafeTree;
	using node_data = NodeData;
	
	typesafe_tree_node( void ) = default;
	typesafe_tree_node( const node & ) = default;
	typesafe_tree_node( node && ) = default;
	
	typesafe_tree_node &operator=( const typesafe_tree_node & ) = default;
	typesafe_tree_node &operator=( typesafe_tree_node && ) = default;
	
	template<
		class Data,
		class = std::enable_if_t<std::is_constructible_v<node_data, Data &&>>
	> typesafe_tree_node( Data &&d ) : data{std::forward<Data>( d )} {}
	
	node_data data;
};

template<class NodeDataTypes, class Arities> class typesafe_tree {
	static constexpr auto node_data_types = NodeDataTypes{};
	static constexpr auto arities = Arities{};
	
	static_assert( boost::hana::length( node_data_types ) != boost::hana::size_c<0>, "No node types were provided." );
	static_assert( boost::hana::length( node_data_types ) != boost::hana::length( arities ) );
	
	static constexpr auto node_data_types2 = boost::hana::transform(
		node_data_types,
		boost::hana::typeid_
	);
	static_assert( node_data_types == node_data_types2, "Some node data types are not boost::hana types." );
	
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
	
	template<class NodeData>
	static constexpr auto index_of_node_data( void ) {
		constexpr auto index = boost::hana::index_if(
			node_variant::types{},
			[]( auto &&t ) {
				if constexpr( std::is_same_v<typename decltype(+t)::type, boost::blank> ) {
					return boost::hana::bool_c<std::is_same_v<NodeData, boost::blank>>;
				} else {
					using node_data = typename boost::unwrap_recursive<typename decltype(+t)::type::node_data>::type;
					return boost::hana::bool_c<std::is_same_v<NodeData, node_data>>;
				}
			}
		);
		static_assert(
			boost::hana::is_just( index ),
			"The given node data type does not appear in the given node variant."
		);
		return *index;
	}
public:
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
	
	constexpr decltype( auto ) arity( void ) const {
		constexpr auto arity_visitor = []( const auto &n ) -> std::variant<std::size_t, runtime_arity_t> {
			return arity_of_node_type( boost::hana::type_c<std::decay_t<decltype( n )>> );
		};
		return boost::apply_visitor( arity_visitor, node );
	}
	
	constexpr decltype( auto ) is_terminal( void ) const {
		auto arity_ = arity();
		if( std::holds_alternative<std::size_t>( arity_ ) && std::get<std::size_t>( arity_ ) == 0 ) {
			return true;
		}
		return false;
	}
	
	template<class F> decltype( auto ) visit( F &&f ) const {
		return boost::apply_visitor( std::forward<F>( f ), node );
	}
	
	template<class NodeData>
	constexpr bool holds_node( boost::hana::type_c<NodeData> ) const {
		constexpr auto node_type_index = index_of_node_data<NodeData>();
		return ( node_type_index == node.which() );
	}
	
	template<class NodeData>
	static constexpr auto get_node( boost::hana::type_c<NodeData> ) {
		using nodes = node_variant::types;
		constexpr auto node_index = index_of_node_data<NodeData>();
		using node_type = typename boost::mpl::at_c<nodes, node_type_index>::type;
		return boost::get<node_type>( node );
	}
	
	template<class NodeData>
	static constexpr auto get_node( boost::hana::type_c<NodeData> ) const {
		using nodes = node_variant::types;
		constexpr auto node_index = index_of_node_data<NodeData>();
		using node_type = typename boost::mpl::at_c<nodes, node_type_index>::type;
		return boost::get<node_type>( node );
	}
};

template<class NodeData, class TypesafeTree>
class typesafe_tree_node<
	NodeData,
	TypesafeTree,
	std::enable_if<!TypesafeTree::is_terminal( boost::hana::type_c<NodeData> )>
> {
	static_assert( is_typesafe_tree_v < TypesafeTree > , "Internal error" );
	static constexpr auto child_container_( void ) {
		constexpr auto arity = TypesafeTree::arity( boost::hana::type_c<T> );
		
		if constexpr( boost::hana::typeid_( arity ) == boost::hana::type_c<runtime_arity_t> ) {
			return boost::hana::type_c<std::vector<TypesafeTree>>;
		} else {
			static_assert( arity != 0, "Internal error" );
			return boost::hana::type_c<std::array<TypesafeTree, arity>>;
		}
	}
public:
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
		class = std::enable_if_t<std::is_constructible_v<child_container, Children &&...>>,
	> typesafe_tree_node( Data &&d, Children &&...ch )
		: data{d}, children{std::forward<Children>( ch )...} {}
	
	node_data data;
	child_container children;
};

namespace model_typesafe_tree {
struct is_terminal {
	template<class TypesafeTree>
	static constexpr decltype( auto ) apply( const TypesafeTree &tree ) {
		return tree.is_terminal();
	}
};

struct arity {
	template<class TypesafeTree>
	static constexpr decltype( auto ) apply( const TypesafeTree &tree ) {
		return tree.arity();
	}
};

struct apply_to_children {
	template<class TypesafeTree, class F>
	static constexpr decltype( auto ) apply( TypesafeTree &&tree, F &&f ) {
	
	}
};
}

namespace impl {
template<class NodeDataTypes, class Arities> struct default_tree<typesafe_tree<NodeDataTypes, Arities>> {
	using type =
};
}
}

#endif //CXXMATH_MODELS_TYPESAFE_TREE_HPP
