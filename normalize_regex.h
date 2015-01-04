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
 * check if a regular expression is of the _display filter with curses
 * attribute_ form.
 *
 * @param[in] regex regular expression string.
 * @param[out] curses attributes if the function returns true.
 * @param[out] fg curses foreground color; -1 if no color was specified.
 * @param[out] bg curses background color; -1 if no color was specified.
 * @return true if regex is a _display filter with curses attribute_.
 */
bool is_df_with_curses_attr(const std::string& str, uint64_t& attr, int& fg, int& bg);
