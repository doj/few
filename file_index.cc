/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "file_index.h"
#include "error.h"
#include <cassert>
#include <sysexits.h>

file_index::file_index(const std::string& filename) :
    file_(filename),
    has_parsed_all_(false)
{
    if (file_.empty()) {
	throw error("could not memory map: " + filename, EX_NOINPUT);
    }

    // we start counting lines with number 1. So add an invalid pointer to index 0.
    line_.push_back(line_t(nullptr, nullptr, nullptr, 0));
}

bool file_index::parse_line(const unsigned index)
{
    if (file_.empty()) {
	has_parsed_all_ = true;
	return false;
    }

    if (index <= size()) {
	return true;
    }

    unsigned num = 0;
    const c_t* it = nullptr;
    if (index > 1 && size() > 0) {
	line_t current_line = line_[size()];
	it = current_line.next_;
	num = current_line.num_;
    } else {
	it = file_.begin();
    }

    if (it == file_.end()) {
	return false;
    }
    if (it == nullptr) {
	return false;
    }

    assert(it);
    const c_t* beg = it;
    while(num < index) {
	if (*it == '\n') {
	    const c_t* next = it + 1;
	    push_line(beg, it, next, ++num);
	    beg = next;
	}
	++it;
	if (it == file_.end()) {
	    if (it != beg) {
		push_line(beg, it, nullptr, ++num);
	    }
	    has_parsed_all_ = true;
	    break;
	}
    }

    return num == index;
}

void
file_index::push_line(const c_t* beg, const c_t* end, const c_t* next, const unsigned num)
{
    while(beg < end) {
	if (*(end - 1) == '\n') {
	    --end;
	    continue;
	}
	if (*(end - 1) == '\r') {
	    --end;
	    continue;
	}
	break;
    }
    line_.push_back(line_t(beg, end, next, num));
    lineNum_set_.insert(num);
}

line_t file_index::line(const unsigned idx)
{
    if (idx > size()) {
	if (! parse_line(idx)) {
	    throw std::runtime_error("file_index::line(" + std::to_string(idx) + "): index too large, file only contains " + std::to_string(size()));
	}
    }
    return line_[idx];
}

const lineNum_set_t&
file_index::lineNum_set()
{
    if (! has_parsed_all_) {
	parse_all();
    }
    assert(has_parsed_all_);
    return lineNum_set_;
}

void
file_index::parse_all(ProgressFunctor *func)
{
    if (has_parsed_all_) {
	return;
    }
    unsigned cnt = 0;
#if 1
    for(auto l : *this) {
	(void)l;
#else
    iterator it = begin();
    while(it != end()) {
	++it;
#endif
	if (func && (((++cnt) % 10000) == 0)) {
	    const uint64_t pos = line_[cnt].end_ - line_[1].beg_;
	    func->progress(cnt, pos * 100 / file_.size());
	}
    }
}
