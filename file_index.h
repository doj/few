/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include "memorymap.h"
#include "progress_functor.h"
#include "regex_index.h"
#include <vector>
#include <cassert>

class file_index
{
    /// the character type we are using. Maybe we use wide characters one day.
    typedef char c_t;

    doj::memorymap_ptr<c_t> file_;

    /**
     * all lines, indexed by their line number.
     * The first line in the file has line number 1.
     * The entry with index 0 is invalid and should not be used.
     */
    std::vector<line_t> line_;

    /// true if the entire file has been parsed.
    bool has_parsed_all_;

    /**
     * parse line number num from the file.
     * @param num line number to parse, the first line is 1.
     * @return true if the line exists and was parsed; false if the line does not exist.
     */
    bool parse_line(const line_number_t num);

    void push_line(const c_t* beg, const c_t* end, const c_t* next, const line_number_t num);

    /// if true, abort any threads running parse_all_in_background()
    static bool abortBackgroundParse_s;

public:

    /// abort any background parsing threads
    static void abort_background_parse() { abortBackgroundParse_s = true; }

    typedef std::shared_ptr<file_index> ptr_t;

    typedef std::vector<std::shared_ptr<regex_index>> regex_index_vec_t;

    /**
     * construct and initialize the file_index with the contents of filename.
     * @param filename file name to initialize lines from.
     */
    explicit file_index(const std::string& filename);

    /// @return the number of currently parsed lines. This could be less than the total number of lines in the file.
    line_number_t size() const
    {
	return line_.size() - 1;
    }

    /**
     * get line number num.
     * @throws std::runtime_error if the line does not exist.
     */
    line_t line(const line_number_t num);

    /// @return percentage into the file, that line number num is.
    unsigned perc(const line_number_t num);

    /**
     * @param os output stream to print progress information on, can be nullptr.
     */
    void parse_all(regex_index_vec_t& regex_index_vec, ProgressFunctor *func = nullptr);

    void parse_all(std::shared_ptr<regex_index> ri, ProgressFunctor *func = nullptr);

    void parse_all();

    /**
     * allow a background thread to parse the entire file and match with a regex_index object.
     * This function is only valid if parse_all() has been called before and the entire file is indexed.
     * @return true if parsing finished.
     * @return false if parse was aborted.
     */
    bool parse_all_in_background(std::shared_ptr<regex_index> ri) const;

    /// @return the line number vector of all lines in the file.
    lineNum_vector_t lineNum_vector();
};
