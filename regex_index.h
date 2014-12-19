/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include "file_index.h"

class regex_index
{
protected:
    index_set_t index_set_;

public:
    /**
     * filter f_idx with the regular expression rgx.
     * The string flags can contain the following characters to modify the regular expression matching:
     * - i case insensitive.
     * - ! negative regex, will match all lines *not* matching rgx.
     *
     * @throws std::runtime_error if regular expression could not be parsed.
     */
    regex_index(file_index& f_idx, const std::string& rgx, const std::string& flags);

    unsigned size() const { return index_set_.size(); }

    const index_set_t& index_set() const { return index_set_; }

    /**
     * intersect the object's index set with s.
     * Only the elements common to the object's index set and s are included in the result set.
     * @param[in] s index set.
     * @return new set with the common elements.
     */
    index_set_t intersect(const index_set_t& s);

    index_set_t intersect(const regex_index& r)
    {
	return intersect(r.index_set());
    }
};
