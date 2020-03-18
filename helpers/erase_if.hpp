//
// Created by jayz on 29.08.19.
//

#ifndef CXXMATH_CORE_ERASE_IF_HPP
#define CXXMATH_CORE_ERASE_IF_HPP

namespace cxxmath {
template<class ErasableRange, class Predicate>
void erase_if( ErasableRange &r, Predicate predicate ) {
	auto it = r.begin();
	auto end = r.end();
	
	while( it != end ) {
		if( predicate( *it ) ) {
			it = r.erase( it );
			end = r.end();
		} else {
			++it;
		}
	}
}
}

#endif //CXXMATH_CORE_ERASE_IF_HPP
