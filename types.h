/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <set>
#include <stdint.h>
#include <vector>

typedef uint32_t line_number_t;
typedef std::set<line_number_t> lineNum_set_t;
typedef std::vector<line_number_t> lineNum_vector_t;
typedef std::vector<std::pair<lineNum_set_t::iterator, lineNum_set_t::iterator>> lineNum_set_intersect_vector_t;

/**
 * an interface class to provide a methods to intersect lineNum_set_t objects.
 */
class ILineNumSetProvider
{
public:
    virtual const lineNum_set_t& lineNum_set() = 0;
};
