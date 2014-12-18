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
    regex_index(file_index& f_idx, const std::string& rgx, const std::string& flags);

    unsigned size() const { return index_vec.size(); }
};
