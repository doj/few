/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <set>
#include <stdint.h>

typedef uint32_t line_number_t;
typedef std::set<line_number_t> lineNum_set_t;

/**
 * an interface class to provide a methods to intersect lineNum_set_t objects.
 */
class ILineNumSetProvider
{
public:
    virtual const lineNum_set_t& lineNum_set() = 0;

    /**
     * intersect the object's index set with s.
     * Only the elements common to the object's index set and s are included in the result set.
     * @param[in] s index set.
     * @return new set with the common elements.
     */
    lineNum_set_t intersect(const lineNum_set_t& s);
};

lineNum_set_t intersect(const lineNum_set_t& l, const lineNum_set_t& r);
