/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include "file_index.h"
#include <memory>
#include <regex>

/**
 * parse regular expression flags string.
 * Valid flags are 'i' and '!'.
 * @param[in] flags string with regular expression flags.
 * @param[out] fl std::regex flags.
 * @param[out] positiveMatch false if the '!' flag was found; true otherwise.
 * @throws std::runtime_error if an unknown flag was found.
 */
void convert(const std::string& flags, std::regex_constants::syntax_option_type& fl, bool& positiveMatch);

class regex_index
{
protected:
    lineNum_set_t lineNum_set_;

public:
    /**
     * filter f_idx with the regular expression rgx.
     * The string flags can contain the following characters to modify the regular expression matching:
     * - i case insensitive.
     * - ! negative regex, will match all lines *not* matching rgx.
     *
     * @throws std::runtime_error if regular expression could not be parsed.
     */
    regex_index(std::shared_ptr<file_index> f_idx, const std::string& rgx, const std::string& flags);

    unsigned size() const { return lineNum_set_.size(); }

    const lineNum_set_t& lineNum_set() const { return lineNum_set_; }

    /**
     * intersect the object's index set with s.
     * Only the elements common to the object's index set and s are included in the result set.
     * @param[in] s index set.
     * @return new set with the common elements.
     */
    lineNum_set_t intersect(const lineNum_set_t& s);

    lineNum_set_t intersect(const regex_index& r)
    {
	return intersect(r.lineNum_set());
    }
};
