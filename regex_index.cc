/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "regex_index.h"
#include <iostream>

void convert(const std::string& flags, std::regex_constants::syntax_option_type& fl, bool& positiveMatch)
{
    bool positive_match = true;
    std::regex_constants::syntax_option_type flg = std::regex::ECMAScript | std::regex::optimize;
    for(auto f : flags) {
	switch(f) {
	case 'i': flg |= std::regex::icase; break;
	case '!': positive_match = false; break;

	default:
	    throw std::runtime_error(std::string("invalid regular expression flags character: ") + f);
	}
    }

    // set output parameters
    positiveMatch = positive_match;
    fl = flg;
}

regex_index::regex_index(std::shared_ptr<file_index> f_idx, const std::string& rgx, const std::string& flags, ProgressFunctor *func)
{
    bool positive_match = true;
    std::regex_constants::syntax_option_type fl;
    convert(flags, fl, positive_match);

    const uint64_t total_lines = f_idx->size();
    uint64_t cnt = 0;

    std::regex r(rgx, fl);
    for(auto line : *f_idx) {
	//std::clog << line.num_ << ":" << line.to_string();
	bool res = std::regex_search(line.beg_, line.end_, r);
	if (( positive_match &&  res) ||
	    (!positive_match && !res)) {
	    lineNum_set_.insert(line.num_);
	    //std::clog << " !match!";
	}
	//std::clog << std::endl;

	if (func && ((++cnt) % 10000) == 0) {
	    func->progress(cnt, cnt * 100 / total_lines);
	}
    }
}

lineNum_set_t
intersect(const lineNum_set_t& L, const lineNum_set_t& R)
{
    // make l the smaller set
    const bool L_smaller_R = L.size() < R.size();
    const lineNum_set_t &l = L_smaller_R ? L : R;
    const lineNum_set_t &r = L_smaller_R ? R : L;

    lineNum_set_t s;
    for(auto i : l) {
	if (r.count(i)) {
	    s.insert(i);
	}
    }
    return s;
}

lineNum_set_t
ILineNumSetProvider::intersect(const lineNum_set_t& s)
{
    return ::intersect(lineNum_set(), s);
}

const lineNum_set_t&
regex_index::lineNum_set()
{
    return lineNum_set_;
}
