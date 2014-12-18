/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include "memorymap.h"
#include "line.h"
#include <vector>
#include <cassert>

typedef doj::memorymap_ptr<char> file_t;

class file_index
{
    const file_t& file_;
    std::vector<line_t> line_;

    /**
     * parse line number "index" from the file.
     * @param index line number to parse, the first line has index 1.
     * @return true if the line exists and was parsed; false if the line does not exist.
     */
    bool parse_line(const unsigned index);

    void push_line(file_t::const_iterator beg, file_t::const_iterator end, file_t::const_iterator next, const unsigned num);

public:
    explicit file_index(const file_t& f);

    /// @return the number of currently parsed lines. This could be less than the total number of lines in the file.
    unsigned lines() const
    {
	return line_.size() - 1;
    }

    /**
     * get line number "idx".
     * @throws std::runtime_error if the line does not exist.
     */
    line_t line(const unsigned idx);

    class iterator
    {
	file_index* f_idx_;
	unsigned line_num_;

    public:
	explicit iterator(file_index& f_idx) :
	f_idx_(&f_idx),
	line_num_(0)
	{
	    assert(! is_end());
	}

	/// construct on "end" iterator
	explicit iterator(bool) :
	f_idx_(nullptr),
	line_num_(0)
	{
	    assert(is_end());
	}

	bool is_end() const { return f_idx_ == nullptr; }

	bool operator== (const iterator& r)
	{
	    if (is_end() && r.is_end()) {
		return true;
	    }
	    if (!is_end() && !r.is_end()) {
		return line_num_ == r.line_num_;
	    }
	    return false;
	}

	bool operator!= (const iterator& r)
	{
	    return !(*this == r);
	}

	iterator& operator++ ()
	{
	    if (is_end()) {
		return *this;
	    }
	    if (f_idx_->parse_line(++line_num_)) {
		assert(! is_end());
		return *this;
	    }
	    line_num_ = 0;
	    f_idx_ = nullptr;
	    assert(is_end());
	    return *this;
	}

	line_t operator* ()
	{
	    if (is_end()) {
		return f_idx_->line_[0];
	    }
	    if (line_num_ == 0) {
		++(*this);
	    }
	    assert(line_num_ <= f_idx_->lines());
	    line_t l = f_idx_->line_[line_num_];
	    assert(l.num_ == line_num_);
	    return l;
	}
    };
    friend class iterator;

    iterator begin()
    {
	return iterator(*this);
    }

    iterator end()
    {
	return iterator(false);
    }

    void parse_all()
    {
#if 1
	for(auto l : *this) { (void)l; }
#else
	iterator it = begin();
	while(it != end()) {
	    ++it;
	}
#endif
    }

};
