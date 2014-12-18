/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include "file_index.h"

class regex_index
{
public:
    typedef std::vector<line_t> line_vec_t;

protected:
    line_vec_t line_;

public:
    regex_index(const file_index& f_idx, const std::string& rgx);
};
