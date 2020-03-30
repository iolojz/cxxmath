//
// Created by jayz on 17.03.20.
//

#define BOOST_TEST_MODULE cxxmath - tree

#include <boost/test/included/unit_test.hpp>

#include "../cxxmath.hpp"

struct plus_node_data {
	bool operator==( const plus_node_data & ) const { return true; }
};
struct times_node_data {
	bool operator==( const times_node_data & ) const { return true; }
};
struct x_node_data {
	bool operator==( const x_node_data & ) const { return true; }
};
struct delta_x_node_data {
	bool operator==( const delta_x_node_data & ) const { return true; }
};

std::ostream &operator<<( std::ostream &os, const plus_node_data & ) {
	return os << "plus";
}
std::ostream &operator<<( std::ostream &os, const times_node_data & ) {
	return os << "times";
}
std::ostream &operator<<( std::ostream &os, const x_node_data & ) {
	return os << "x";
}
std::ostream &operator<<( std::ostream &os, const delta_x_node_data & ) {
	return os << "\\Delta x";
}

using node_data_types = std::tuple<int, plus_node_data, times_node_data, x_node_data, delta_x_node_data>;
using node_arities = std::tuple<
	std::integral_constant<int, 0>,
	std::integral_constant<int, 2>,
	std::integral_constant<int, 2>,
	std::integral_constant<int, 0>,
	std::integral_constant<int, 0>
>;
using tree_type = cxxmath::typesafe_tree<node_data_types, node_arities>;

struct node_derivative_impl {
	tree_type operator()( int i ) const { return { i }; }
	tree_type operator()( x_node_data ) const { return { delta_x_node_data{} }; }
	tree_type operator()( delta_x_node_data ) const { return { delta_x_node_data{} }; }
	
	template<class Summands, class TransformedSummands>
	tree_type operator()( plus_node_data, Summands &&, TransformedSummands &&ts ) const {
		if constexpr( std::is_rvalue_reference_v<TransformedSummands &&> )
			return { plus_node_data{}, std::move( ts.front() ), std::move( ts.back() ) };
		else
			return { plus_node_data{}, ts.front(), ts.back() };
	}
	
	template<class Factors, class TransformedFactors>
	tree_type operator()( times_node_data, Factors &&factors, TransformedFactors &&tfactors ) const {
		tree_type part1 = { times_node_data{}, factors.front(), tfactors.back() };
		tree_type part2 = { times_node_data{}, tfactors.front(), factors.back() };
		
		return { plus_node_data{}, std::move( part1 ), std::move( part2 ) };
	}
};
static constexpr node_derivative_impl node_derivative;

BOOST_AUTO_TEST_CASE( test_tree_basic ) {
	using namespace cxxmath;
	using tree_tag = tag_of_t<tree_type>;
	using tree_node = typename default_tree_t<tree_tag>::tree_node;
	
	auto tree_1 = make<tree_tag>( 1 );
	auto tree_2 = make<tree_tag>( 2 );
	auto tree_x = make<tree_tag>( x_node_data{} );
	auto tree_1_plus_2 = make<tree_tag>( plus_node_data{}, tree_1, tree_2 );
	auto tree_1_plus_2_plus_x = make<tree_tag>( plus_node_data{}, tree_1_plus_2, tree_x );
	
	BOOST_TEST_REQUIRE( holds_node<plus_node_data>( tree_1_plus_2_plus_x ) );
	auto node_1_plus_2_plus_x = get_node<plus_node_data>( tree_1_plus_2_plus_x );
	auto children_node_1_plus_2_plus_x = tree_node::children( node_1_plus_2_plus_x );
	
	BOOST_TEST_REQUIRE( std::size( children_node_1_plus_2_plus_x ) == 2 );
	auto compare_tree_1_plus_2 = children_node_1_plus_2_plus_x[0];
	auto compare_tree_x = children_node_1_plus_2_plus_x[1];
	
	BOOST_TEST( compare_tree_1_plus_2 == compare_tree_1_plus_2 );
	BOOST_TEST( compare_tree_x == tree_x );
	
	BOOST_TEST_REQUIRE( holds_node<x_node_data>( compare_tree_x ) );
	auto node_x = get_node<x_node_data>( compare_tree_x );
	[[maybe_unused]] x_node_data x = tree_node::data( node_x );
	
	BOOST_TEST_REQUIRE( holds_node<plus_node_data>( compare_tree_1_plus_2 ) );
	auto node_1_plus_2 = get_node<plus_node_data>( compare_tree_1_plus_2 );
	auto children_node_1_plus_2 = tree_node::children( node_1_plus_2 );
	
	BOOST_TEST_REQUIRE( std::size( children_node_1_plus_2 ) == 2 );
	auto compare_tree_1 = children_node_1_plus_2[0];
	auto compare_tree_2 = children_node_1_plus_2[1];
	
	BOOST_TEST( compare_tree_1 == tree_1 );
	BOOST_TEST( compare_tree_2 == tree_2 );
	
	BOOST_TEST_REQUIRE( holds_node<int>( compare_tree_1 ) );
	auto node_1 = get_node<int>( compare_tree_1 );
	int one = tree_node::data( node_1 );
	
	BOOST_TEST( one == 1 );
	
	BOOST_TEST_REQUIRE( holds_node<int>( compare_tree_2 ) );
	auto node_2 = get_node<int>( compare_tree_2 );
	int two = tree_node::data( node_2 );
	
	BOOST_TEST( two == 2 );
}

BOOST_AUTO_TEST_CASE( test_tree_derivative ) {
	using namespace cxxmath;
	using tree_tag = tag_of_t<tree_type>;
	using tree_node = typename default_tree_t<tree_tag>::tree_node;
	
	auto tree_x = make<tree_tag>( x_node_data{} );
	auto tree_dx = make<tree_tag>( delta_x_node_data{} );
	auto tree_x_2 = make<tree_tag>( times_node_data{}, tree_x, tree_x );
	
	auto derivative = recursive_tree_transform( tree_x_2, node_derivative );
	
	auto tree_x_dx = make<tree_tag>( times_node_data{}, tree_x, tree_dx );
	auto tree_dx_x = make<tree_tag>( times_node_data{}, tree_dx, tree_x );
	
	auto compare_derivative = make<tree_tag>( plus_node_data{}, tree_x_dx, tree_dx_x );
	BOOST_TEST( compare_derivative == derivative );
}
