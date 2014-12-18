/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "regex_index.h"
#include <regex>
#include <iostream>
regex_index::regex_index(file_index& f_idx, const std::string& rgx, const std::string& flags)
{
    // \todo handle flags
    std::regex r(rgx);
    for(auto line : f_idx) {
	//std::clog << line.num_ << ":" << line.to_string();
	if (std::regex_match(line.beg_, line.end_, r)) {
	    index_vec.push_back(line.num_);
	}
    }
}
