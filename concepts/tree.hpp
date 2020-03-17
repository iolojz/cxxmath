//
// Created by jayz on 17.03.20.
//

#ifndef CXXMATH_CONCEPTS_TREE_HPP
#define CXXMATH_CONCEPTS_TREE_HPP

namespace cxxmath
{
namespace concepts
{
template<class IsTerminal, class Arity, class Children, class MakeNode>
struct tree
{
	static constexpr auto is_terminal = function_object_v<IsTerminal>;
	static constexpr auto arity = function_object_v<Arity>;
	static constexpr auto children = function_object_v<Children>;
	static constexpr auto make_node = function_object_v<MakeNode>;
};

template<class> struct is_tree : std::false_type {};
template<class IsTerminal, class Arity, class Children>
struct is_tree<tree<IsTerminal, Arity, Children>> : std::true_type {};

CXXMATH_DEFINE_STATIC_CONSTEXPR_VALUE_TEMPLATE(is_tree)
}

template<class DispatchTag, class Tree>
struct models_concept<DispatchTag, Tree, std::enable_if_t<concepts::is_tree_v<Tree>>>
{
	static constexpr bool value = (
		Tree::is_terminal.template supports_tag<DispatchTag>() &&
		Tree::arity.template supports_tag<DispatchTag>() &&
		Tree::children.template supports_tag<DispatchTag>()
	);
};

CXXMATH_DEFINE_CONCEPT( tree )

CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( is_terminal, tree )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( arity, tree )
CXXMATH_DEFINE_DEFAULT_DISPATCHED_FUNCTION( children, tree )
}

#endif //CXXMATH_CONCEPTS_TREE_HPP
