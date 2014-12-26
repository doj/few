/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 *
 * Copyright (c) 2014 Dirk Jagdmann <doj@cubic.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you
 *        must not claim that you wrote the original software. If you use
 *        this software in a product, an acknowledgment in the product
 *        documentation would be appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and
 *        must not be misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *        distribution.
 */

#pragma once
#include <algorithm>
#include <vector>

template <typename PairIter, typename OutputIter>
unsigned long long multiple_set_intersect(PairIter pair_begin, PairIter pair_end, OutputIter out)
{
    unsigned long long cnt = 0u;
    // if nothing was provided, there's nothing to do
    if (pair_begin == pair_end) {
	return cnt;
    }

    // check if there's only one pair?
    PairIter pair_first = pair_begin;
    ++pair_begin;
    if (pair_begin == pair_end) {
	while(pair_first->first != pair_first->second) {
	    *out = *(pair_first->first);
	    ++out;
	    ++(pair_first->first);
	    ++cnt;
	}
	return cnt;
    }

    // intersect two or more...

    // loop over the elements of the first set
    for(auto i = pair_first->first ; i != pair_first->second ; ++i) {
	bool all_equal = true;

	// loop over all the other sets
	for(PairIter p = pair_begin; p != pair_end ; ++p) {
	    // if any set is fully processed, we are done
	    if (p->first == p->second) {
		return cnt;
	    }

	    // skip over all smaller elements
	    while(*(p->first) < *i) {
		++(p->first);

		// if we've found the end of this set, we are done
		if (p->first == p->second) {
		    return cnt;
		}
	    }

	    // check if the element is equal to the first element
	    if (all_equal && !(*(p->first) == *i)) {
		all_equal = false;
	    }
	}

	if (all_equal) {
	    *out = *i;
	    ++out;
	    ++cnt;
	}
    }

    return cnt;
}
