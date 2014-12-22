/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <string>
std::string normalize_regex(std::string regex);

/// @return flags of normalized regular expression string.
std::string get_regex_flags(std::string str);

/// @return regular expression string (the characters between the forward slashes) of normalized regular expression string.
std::string get_regex_str(std::string str);
