//
// Created by jayz on 17.03.20.
//

#define BOOST_TEST_MODULE cxxmath - mapping prescription

#include <boost/test/included/unit_test.hpp>

#include "../cxxmath.hpp"

struct plus_node_data {
	bool operator==( const plus_node_data & ) const { return true; }
};
struct x_node_data {
	bool operator==( const x_node_data & ) const { return true; }
};

std::ostream &operator<<( std::ostream &os, const plus_node_data & ) {
	return os << "plus";
}
std::ostream &operator<<( std::ostream &os, const x_node_data & ) {
	return os << "x";
}

/*struct node_is_argument {
	template<class T> static constexpr bool apply( T && ) {
		return std::is_same_v<std::decay_t<T>, x_node>;
	}
};*/

BOOST_AUTO_TEST_CASE( test_mapping_prescription ) {
	using namespace cxxmath;
	using node_data_types = std::tuple<int, plus_node_data, x_node_data>;
	using node_arities = std::tuple<
	    std::integral_constant<int, 0>,
	    std::integral_constant<int, 2>,
		std::integral_constant<int, 0>
	>;
	using tree_type = typesafe_tree<node_data_types, node_arities>;
	using tree_tag = tag_of_t<tree_type>;
	
	using tree = default_tree_t<tree_tag>;
	using tree_node = typename tree::tree_node;
	//using mapping_prescription = concepts::mapping_prescription_t<tree, node_is_argument>;
	
	auto tree_1 = make<tree_tag>( 1 );
	auto tree_2 = make<tree_tag>( 2 );
	auto tree_x = make<tree_tag>( x_node_data{} );
	auto tree_1_plus_2 = make<tree_tag>( plus_node_data{}, tree_1, tree_2 );
	auto tree_1_plus_2_plus_x = make<tree_tag>( plus_node_data{}, tree_1_plus_2, tree_x );
	
	BOOST_TEST_REQUIRE( tree::holds_node<plus_node_data>( tree_1_plus_2_plus_x ) );
	auto node_1_plus_2_plus_x = tree::get_node<plus_node_data>( tree_1_plus_2_plus_x );
	auto children_node_1_plus_2_plus_x = tree_node::children( node_1_plus_2_plus_x );
	
	BOOST_TEST_REQUIRE( std::size( children_node_1_plus_2_plus_x ) == 2 );
	auto compare_tree_1_plus_2 = children_node_1_plus_2_plus_x[0];
	auto compare_tree_x = children_node_1_plus_2_plus_x[1];
	
	BOOST_TEST( compare_tree_1_plus_2 == compare_tree_1_plus_2 );
	BOOST_TEST( compare_tree_x == tree_x );
	
	BOOST_TEST_REQUIRE( tree::holds_node<x_node_data>( compare_tree_x ) );
	auto node_x = tree::get_node<x_node_data>( compare_tree_x );
	[[maybe_unused]] x_node_data x = tree_node::data( node_x );
	
	BOOST_TEST_REQUIRE( tree::holds_node<plus_node_data>( compare_tree_1_plus_2 ) );
	auto node_1_plus_2 = tree::get_node<plus_node_data>( compare_tree_1_plus_2 );
	auto children_node_1_plus_2 = tree_node::children( node_1_plus_2 );
	
	BOOST_TEST_REQUIRE( std::size( children_node_1_plus_2 ) == 2 );
	auto compare_tree_1 = children_node_1_plus_2[0];
	auto compare_tree_2 = children_node_1_plus_2[1];
	
	BOOST_TEST( compare_tree_1 == tree_1 );
	BOOST_TEST( compare_tree_2 == tree_2 );
	
	BOOST_TEST_REQUIRE( tree::holds_node<int>( compare_tree_1 ) );
	auto node_1 = tree::get_node<int>( compare_tree_1 );
	int one = tree_node::data( node_1 );
	
	BOOST_TEST( one == 1 );
	
	BOOST_TEST_REQUIRE( tree::holds_node<int>( compare_tree_2 ) );
	auto node_2 = tree::get_node<int>( compare_tree_2 );
	int two = tree_node::data( node_2 );
	
	BOOST_TEST( two == 2 );
}
