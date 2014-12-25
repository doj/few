/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include "line.h"
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

class regex_index : public ILineNumSetProvider
{
    lineNum_set_t lineNum_set_;
    std::regex rgx_;
    bool positive_match_;

public:
    /**
     * create regular expression index object.
     * @param rgx a (normalized) regular expression string.
     * @throws std::runtime_error if regular expression could not be parsed.
     */
    explicit regex_index(std::string rgx);

    /// match line against the provisioned regular expression. If it matches add the line (number) to the set.
    void match(const line_t& line);

    unsigned size() const { return lineNum_set_.size(); }

    virtual const lineNum_set_t& lineNum_set();
};
