//
// Created by jayz on 17.03.20.
//

#define BOOST_TEST_MODULE cxxmath - mapping prescription

#include <boost/test/included/unit_test.hpp>

#include "../cxxmath.hpp"

BOOST_AUTO_TEST_CASE( test_mapping_prescription ) {
	// A mapping prescription should work "lambda" style - i.e inline spec, no declaration
	// We need to specify which arguments are considered to be "the same", e.g a mapping prescription might be
	//   a + a
	// which is stored as
	//   plus( #1, #1 )
	// using Mathematica notation.
	// But this only applies to __unnamed__ arguments. Using named arguments, we would do
	//   plus( #argname, #argname )
	// instead.
	// --> We need a concept of named/unnamed arguments and functions thereof.
	
	// To make the representation efficient, the model shoule have a predefined type. (e.g a tree)
	
	// What operations do we want?
	// - none, really...
	// --> This makes our mapping prescription a model and not a concept
}