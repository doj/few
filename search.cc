/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "search.h"
#include "to_wide.h"
#include <cassert>

bool
search_next(std::wregex rgx, DisplayInfo::ptr_t di, file_index::ptr_t fi)
{
    if (! di->start()) {
	return false;
    }
    while(di->next()) {
	const line_number_t num = di->current();
	const line_t line = fi->line(num);
	std::wstring s = to_wide(line.to_string());
	if (std::regex_search(s, rgx)) {
	    const bool b = di->go_to(line.num_);
	    assert(b);
	    return true;
	}
    }
    return false;
}

bool
search_prev(std::wregex rgx, DisplayInfo::ptr_t di, file_index::ptr_t fi)
{
    if (! di->start()) {
	return false;
    }
    while(di->prev()) {
	const line_number_t num = di->current();
	const line_t line = fi->line(num);
	std::wstring s = to_wide(line.to_string());
	if (std::regex_search(s, rgx)) {
	    const bool b = di->go_to(line.num_);
	    assert(b);
	    return true;
	}
    }
    return false;
}
