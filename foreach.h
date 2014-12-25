/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once

/** a for loop to iterate over a container. You can call erase() on i
    if the erase() function does not invalidate iterators different
    from i. This is true for STL map, multimap, set, multiset.
    @param c (STL) container, has to support begin() and end() methods.
    @param i name of iterator which is available in loop body
*/
#define foreach_e(c,i) for(auto end##i = (c).end(), next##i = (c).begin(), \
                                i = (next##i==end##i)?next##i:next##i++; \
			   i != next##i; \
			   it = (next##i==end##i)?next##i:next##i++)
