/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#pragma once
#include "types.h"
#include <vector>
#include <string>

class DisplayInfo
{
    typedef std::vector<unsigned> displayedLineNum_t;

    displayedLineNum_t displayedLineNum;
    displayedLineNum_t::iterator topLineIt;
    displayedLineNum_t::iterator bottomLineIt;

public:

    DisplayInfo& operator= (const lineNum_set_t& s);

    /// @return the number of lines managed by this object.
    unsigned size() const { return displayedLineNum.size(); }

    /**
     * start an iteration over the lines.
     * This function can only be called after operator=(lineNum_set_t) has been called.
     * @return true if there are lines to display; false if there is nothing to display.
     */
    bool start();

    /**
     * get the current line number.
     * This function can only be called after start() has been called.
     * @return the current line number.
     */
    unsigned current() const;

    /**
     * advance to the next line.
     * This function can only be called after start() has been called.
     * @return true if there is a next line; false if the current line is the last line and nothing was advanced.
     */
    bool next();

    /**
     * This function can only be called after start() has been called.
     * @return the line number of the bottom line on the display.
     */
    unsigned bottomLineNum() const;

    /**
     * This function can only be called after operator=(lineNum_set_t) has been called.
     * @return the line number of the last line managed by this object.
     */
    unsigned lastLineNum() const;

    /**
     * position the object onto a line number.
     * This function can only be called after operator=(lineNum_set_t) has been called.
     * @return true if the line number is managed by this object; false if the line was not found.
     */
    bool go_to(const unsigned lineNum);

    /**
     * check if the first line is displayed.
     * This function can only be called after start() has been called.
     * @return true if the first line of the file is displayed.
     */
    bool isFirstLineDisplayed() const;

    /**
     * check if the last line is displayed.
     * This function can only be called after start() has been called.
     * @return true if the first line of the file is displayed.
     */
    bool isLastLineDisplayed() const;

    /**
     * move the display down one line.
     * This function can only be called after start() has been called.
     */
    void down();

    /**
     * move the display up one line.
     * This function can only be called after start() has been called.
     */
    void up();

    /// move the display to the first line.
    void top();

    /**
     * move the display down one page.
     * The old bottom line becomes the new top line.
     * This function can only be called after start() has been called.
     */
    void page_down();

    std::string info() const;
};