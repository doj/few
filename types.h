/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <stdint.h>
#include <vector>

typedef uint32_t line_number_t;
typedef std::vector<line_number_t> lineNum_vector_t;
typedef std::vector<std::pair<lineNum_vector_t::const_iterator, lineNum_vector_t::const_iterator>> lineNum_vector_intersect_vector_t;
