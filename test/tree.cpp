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
struct y_node_data {
	bool operator==( const y_node_data & ) const { return true; }
};
struct delta_x_node_data {
	bool operator==( const delta_x_node_data & ) const { return true; }
};
struct delta_y_node_data {
	bool operator==( const delta_y_node_data & ) const { return true; }
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
std::ostream &operator<<( std::ostream &os, const y_node_data & ) {
	return os << "y";
}
std::ostream &operator<<( std::ostream &os, const delta_x_node_data & ) {
	return os << "\\Delta x";
}
std::ostream &operator<<( std::ostream &os, const delta_y_node_data & ) {
	return os << "\\Delta y";
}

static constexpr auto node_data_arity_map = boost::hana::make_map(
	boost::hana::make_pair( boost::hana::type_c<int>, boost::hana::int_c<0> ),
	boost::hana::make_pair( boost::hana::type_c<plus_node_data>, boost::hana::int_c<2> ),
	boost::hana::make_pair( boost::hana::type_c<times_node_data>, boost::hana::int_c<2> ),
	boost::hana::make_pair( boost::hana::type_c<x_node_data>, boost::hana::int_c<0> ),
	boost::hana::make_pair( boost::hana::type_c<delta_x_node_data>, boost::hana::int_c<0> ),
	boost::hana::make_pair( boost::hana::type_c<y_node_data>, boost::hana::int_c<0> ),
	boost::hana::make_pair( boost::hana::type_c<delta_y_node_data>, boost::hana::int_c<0> )
);

using tree_type = cxxmath::typesafe_tree<decltype(node_data_arity_map)>;
using derivative_type = std::array<tree_type, 2>;

struct node_derivative_impl {
	derivative_type operator()( int i ) const { return { 0, 0 }; }
	derivative_type operator()( x_node_data ) const { return { delta_x_node_data{}, 0 }; }
	derivative_type operator()( y_node_data ) const { return { 0, delta_y_node_data{} }; }
	
	derivative_type operator()( delta_x_node_data ) const { throw std::runtime_error("foo"); }
	derivative_type operator()( delta_y_node_data ) const { throw std::runtime_error("bar"); }
	
	template<class Summands, class TransformedSummands>
	derivative_type operator()( plus_node_data, Summands &&, TransformedSummands &&ts ) const {
		if constexpr( std::is_rvalue_reference_v<TransformedSummands &&> ) {
			return {
				tree_type( plus_node_data{}, std::move( ts.front().front() ), std::move( ts.back().front() ) ),
				tree_type( plus_node_data{}, std::move( ts.back().back() ), std::move( ts.front().back() ) )
			};
		} else {
			return {
				tree_type( plus_node_data{}, ts.front().front(), ts.back().front() ),
				tree_type( plus_node_data{}, ts.back().back(), ts.front().back() )
			};
		}
	}
	
	template<class Factors, class TransformedFactors>
	derivative_type operator()( times_node_data, Factors &&factors, TransformedFactors &&tfactors ) const {
		tree_type dx_part1 = { times_node_data{}, factors.front(), tfactors.back().front() };
		tree_type dx_part2 = { times_node_data{}, tfactors.front().front(), factors.back() };
		
		tree_type dy_part1 = { times_node_data{}, factors.front(), tfactors.back().back() };
		tree_type dy_part2 = { times_node_data{}, tfactors.front().back(), factors.back() };
		
		return {
			tree_type{ plus_node_data{}, std::move( dx_part1 ), std::move( dx_part2 ) },
			tree_type{ plus_node_data{}, std::move( dy_part1 ), std::move( dy_part2 ) }
		};
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
	auto tree_y = make<tree_tag>( y_node_data{} );
	auto tree_dx = make<tree_tag>( delta_x_node_data{} );
	auto tree_x_2 = make<tree_tag>( times_node_data{}, tree_x, tree_x );
	auto tree_y_x_2 = make<tree_tag>( times_node_data{}, tree_y, tree_x_2 );
	
	auto derivative = recursive_tree_transform( tree_y_x_2, node_derivative );
	
	auto compare_dx_derivative = make<tree_tag>(
		plus_node_data{},
		make<tree_tag>(
			times_node_data{},
			y_node_data{},
			make<tree_tag>(
				plus_node_data{},
				make<tree_tag>(
					times_node_data{},
					x_node_data{},
					delta_x_node_data{}
				),
				make<tree_tag>(
					times_node_data{},
					delta_x_node_data{},
					x_node_data{}
				)
			)
		),
		make<tree_tag>(
			times_node_data{},
			0,
			make<tree_tag>(
				times_node_data{},
				x_node_data{},
				x_node_data{}
			)
		)
	);
	
	auto compare_dy_derivative = make<tree_tag>(
		plus_node_data{},
		make<tree_tag>(
			times_node_data{},
			y_node_data{},
			make<tree_tag>(
				plus_node_data{},
				make<tree_tag>(
					times_node_data{},
					x_node_data{},
					0
				),
				make<tree_tag>(
					times_node_data{},
					0,
					x_node_data{}
				)
			)
		),
		make<tree_tag>(
			times_node_data{},
			delta_y_node_data{},
			make<tree_tag>(
				times_node_data{},
				x_node_data{},
				x_node_data{}
			)
		)
	);
	derivative_type compare_derivative = {compare_dx_derivative, compare_dy_derivative};
	
	BOOST_TEST( derivative == compare_derivative );
}
