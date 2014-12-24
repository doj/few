/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include "memorymap.h"
#include "line.h"
#include "types.h"
#include <vector>
#include <cassert>
#include "progress_functor.h"

class file_index
{
    /// the character type we are using. Maybe we use wide characters one day.
    typedef char c_t;

    doj::memorymap_ptr<c_t> file_;

    /**
     *all lines, indexed by their line number.
     * The first line in the file has line number 1.
     * The entry with index 0 is invalid and should not be used.
     */
    std::vector<line_t> line_;

    /// true if the entire file has been parsed.
    bool has_parsed_all_;

    /// this set contains the line number of all currently parsed lines.
    lineNum_set_t lineNum_set_;

    /**
     * parse line number num from the file.
     * @param num line number to parse, the first line is 1.
     * @return true if the line exists and was parsed; false if the line does not exist.
     */
    bool parse_line(const line_number_t num);

    void push_line(const c_t* beg, const c_t* end, const c_t* next, const line_number_t num);

public:
    /**
     * construct and initialize the file_index with the contents of filename.
     * @param filename file name to initialize lines from.
     */
    explicit file_index(const std::string& filename);

    /// @return the number of currently parsed lines. This could be less than the total number of lines in the file.
    unsigned size() const
    {
	return line_.size() - 1;
    }

    /**
     * get line number num.
     * @throws std::runtime_error if the line does not exist.
     */
    line_t line(const line_number_t num);

    class iterator
    {
	file_index* f_idx_;
	line_number_t line_num_;

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
	    assert(line_num_ <= f_idx_->size());
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

    /**
     * @param os output stream to print progress information on, can be nullptr.
     */
    void parse_all(ProgressFunctor *func = nullptr);

    /// @return the line number set of all lines in the file.
    const lineNum_set_t& lineNum_set();

};
