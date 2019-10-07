//
// Created by jayz on 06.10.19.
//

#ifndef CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP
#define CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP

#include "polynomial.hpp"

namespace cxxmath
{
template<class Polynomial>
struct polynomial_with_abstract_indices_tag;

template<class Polynomial>
struct polynomial_with_abstract_indices
{
	static_assert( is_polynomial_tag<tag_of_t < Polynomial>>::value, "Polynomial is not a polynomial." );
	using dispatch_tag = polynomial_with_abstract_indices_tag<Polynomial>;
private:
	Polynomial polynomial_;
	
	void contract( void )
	{
	
	}
};

namespace impl
{
template<class Polynomial>
struct multiply_polynomials_with_abstract_indices
: supports_tag_helper<polynomial_with_abstract_indices_tag<Polynomial>>
{
	template<class AIPolynomial1, class AIPolynomial2>
	static constexpr decltype( auto ) apply()
	{
	
	}
};
}
}

#endif //CXXMATH_MODELS_ABSTRACT_INDEX_NOTATION_HPP
