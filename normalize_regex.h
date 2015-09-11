/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <string>
#include <stdint.h>

std::string normalize_regex(std::string regex);

/// @return flags of normalized regular expression string.
std::string get_regex_flags(std::string str);

/// @return regular expression string (the characters between the forward slashes) of normalized regular expression string.
std::string get_regex_str(std::string str);

/**
 * check if a regular expression is an _attribute display filter_.
 *
 * @param[in] regex regular expression string.
 * @param[out] curses attributes if the function returns true.
 * @param[out] fg curses foreground color; -1 if no color was specified.
 * @param[out] bg curses background color; -1 if no color was specified.
 * @return true if regex is a _display filter with curses attribute_.
 */
bool is_attr_df(const std::string& str, uint64_t& attr, int& fg, int& bg);

/**
 * check if a regular expression is a filter regex.
 * @param str string to check for filter regular expression type.
 * @return true if str is a filter regex.
 */
bool is_filter_regex(std::string str);

/**
 * parse a _replace display filter_.
 * @param[in] expr the complete regular expression, without any flags.
 * @param[out] rgx the regular expression part (left side).
 * @param[out] rpl the replace string (right side).
 * @param[out] err_msg error message if return false.
 * @return true if the expression is a valid _replace display filter_ regex and rgx and rpl are set;
 *         false if the expression is not valid.
 */
bool parse_replace_df(const std::string& expr, std::string& rgx, std::string& rpl, std::string& err_msg);
