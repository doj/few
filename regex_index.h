/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include "file_index.h"

class regex_index
{
public:
    typedef std::vector<unsigned> index_vec_t;

protected:
    index_vec_t index_vec;

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

    unsigned size() const { return index_vec.size(); }
};
