/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "history.h"
#include <fstream>
#include <algorithm>
#include <iterator>

History::History(const std::string& filename) :
    filename_(filename)
{
    if (! filename_.empty()) {
	std::ifstream is(filename_);
	while(is) {
	    std::string l;
	    getline(is, l);
	    add(l);
	}
    }
    if (v_.size() > 1000u) {
	v_.erase(v_.begin(), v_.begin() + (v_.size() - 1000u));
    }
}

History::~History()
{
    if (! filename_.empty()) {
	std::ofstream os(filename_);
	std::copy(v_.begin(), v_.end(), std::ostream_iterator<std::string>(os, "\n"));
    }
}

void
History::add(const std::string& s)
{
    if (s.empty()) return;
    v_.push_back(s);
}

std::shared_ptr<History::iterator>
History::begin(const std::string& s)
{
    return std::make_shared<History::iterator>(shared_from_this(), s);
}

History::iterator::iterator(std::shared_ptr<History> h, const std::string& s) :
    h_(h),
    s_(s),
    idx_(h->v_.size())
{}

bool
History::iterator::atEnd() const
{
    return idx_ == h_->v_.size();
}

const std::string&
History::iterator::next()
{
    if (atEnd()) {
	return s_;
    }
    ++idx_;
    if (atEnd()) {
	return s_;
    }
    return h_->v_[idx_];
}

const std::string&
History::iterator::prev()
{
    if (idx_ > 0) {
	--idx_;
    }
    if (atEnd()) {
	return s_;
    }
    return h_->v_[idx_];
}
