/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include "memorymap.h"
#include <vector>

typedef doj::memorymap_ptr<char> file_t;

class file_index
{
    const file_t& file_;
    std::vector<file_t::const_iterator> line_;

public:
    explicit file_index(const file_t& f);

    unsigned lines() const
    {
	return line_.size();
    }

};
