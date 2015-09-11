/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include <wchar.h>
#include <getopt.h>
#include <sysexits.h>
#include <iostream>
#include <cassert>
#include <stdint.h>
#include <stdlib.h>
#include <memory>
#include <regex>
#include <map>
#include <fstream>
#include <thread>
#include <iterator>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if defined(__unix__)
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "file_index.h"
#include "regex_index.h"
#include "error.h"
#include "display_info.h"
#include "normalize_regex.h"
#include "curses_attr.h"
#include "history.h"
#include "foreach.h"
#include "getRSS.h"
#include "to_wide.h"
#include "event.h"
#include "intersect.h"
#include "search.h"
#include "temporary_file.h"
#include "console.h"
#include "errno_str.h"
#include "click_link.h"
#include "command.h"
#include "maximize_window.h"
#include "merge_command_line.h"
#include "tokenize_command_line.h"
#include "getenv_str.h"
#include "color.h"
#include "timeGetTime.h"
#include "complete_filename.h"
#include "word_set.h"

#undef max

/// verbosity level
unsigned verbose = 0;

#define lightgray_on_black color(COLOR_WHITE, COLOR_BLACK)
#define white_on_black (lightgray_on_black | A_BOLD | A_STANDOUT)
#define gray_on_black (lightgray_on_black | A_DIM)

namespace {
    /// file name of line edit history
    const char* line_edit_history_rc = ".few.history";

    /// the filename of the currently executed process (argv[0]).
    std::string argv0;

    /// the filename as used on the command line that is displayes.
    std::string command_line_filename = "-";

    /// the info string which is shown in the lower right corner
    std::string info;

    /// maximum number of regular expressions. 10 for keys 1..0, 12 for F1..F12.
    const unsigned max_regex_num = 10 + 12;

    /// minimum screen height
    const unsigned min_screen_height = 1 // lines
	+ max_regex_num
	+ 1 // search
	;

    /// minimum screen width
    const unsigned min_screen_width = 16;

    /// width of screen in characters
#define screen_width static_cast<unsigned>(COLS)
    /// height of screen in characters
#define screen_height static_cast<unsigned>(LINES)

    /// width of a tab character in characters
    unsigned tab_width = 8;

    /// current search regular expression string
    std::string search_str;
    /// compiled search regular expression
    std::wregex search_rgx;
    /// error string if search regular expression could not be compiled
    std::string search_err;
    /// the y position of the search window
    unsigned search_y;

    /// regular expression to match links
    std::wregex link_rgx(L"(ht|f)tps?://[a-zA-Z0-9/~&=%_.-]+", std::regex::ECMAScript | std::regex::optimize | std::regex::icase);
    typedef std::pair<unsigned, unsigned> coordinate_t;
    std::map<coordinate_t, std::wstring> link;

    /// regular expression to match emails
    /// see http://www.regular-expressions.info/email.html
    std::wregex email_rgx(L"\\b[a-z0-9._%+-]+\\@[a-z0-9.-]+\\.[a-z]{2,4}\\b", std::regex::ECMAScript | std::regex::optimize | std::regex::icase);
    std::map<coordinate_t, std::wstring> email;

    /// the file that is displayed
    file_index::ptr_t f_idx;

    /// object to manage displayed lines
    DisplayInfo::ptr_t display_info;

    /// height of the lines window
    unsigned w_lines_height;

    /// line number displayed at the middle of the lines window
    line_number_t middle_line_number = 0;

    /// the y position of the lines filter regex window
    unsigned filter_y;

    struct regex_container_t
    {
	/// the regular expression string
	std::string rgx_;
	/// an error string if rgx_ is invalid
	std::string err_;

	/// object used for lines filter
	std::shared_ptr<regex_index> ri_;

	///@{

	/// regex object used for replace display filter
	std::shared_ptr<std::regex> replace_df_rgx_;
	/// replace display filter replacement text
	std::string replace_df_text_;

	///@}

	///@{

	/// regex object used for the attribute display filter
	std::shared_ptr<std::wregex> attribute_df_rgx_;
	/// attribute display filter curses attributes
	uint64_t attribute_df_attr_;

	///@}
    };

    typedef std::vector<std::shared_ptr<regex_container_t>> regex_vec_t;

    /**
     * type of a regex_container_t cache.
     * key is the regular expression string and has to be equal to value->rgx_.
     * value is a shared pointer to the regex container object.
     */
    typedef std::map<std::string, std::shared_ptr<regex_container_t>> regex_cache_t;

    /// the regular expressions for the display and filter regex
    regex_vec_t regex_vec;

    void regex_vec_resize(const unsigned num)
    {
	while (regex_vec.size() < num) {
	    regex_vec.push_back(std::make_shared<regex_container_t>());
	}
    }

    /// the filter regex cache
    regex_cache_t filter_cache;

    /// @return number of digits in i.
    int digits(uint64_t i)
    {
	if (i < 10llu) return 1;
	if (i < 100llu) return 2;
	if (i < 1000llu) return 3;
	if (i < 10000llu) return 4;
	if (i < 100000llu) return 5;
	if (i < 1000000llu) return 6;
	if (i < 10000000llu) return 7;
	if (i < 100000000llu) return 8;
	if (i < 1000000000llu) return 9;
	if (i < 10000000000llu) return 10;
	if (i < 100000000000llu) return 11;
	if (i < 1000000000000llu) return 12;
	if (i < 10000000000000llu) return 13;
	if (i < 100000000000000llu) return 14;
	if (i < 1000000000000000llu) return 15;
	if (i < 10000000000000000llu) return 16;
	if (i < 100000000000000000llu) return 17;
	if (i < 1000000000000000000llu) return 18;
	if (i < 10000000000000000000llu) return 19;
	return 20;
    }

    /// fill the row y between x and screen_width with space characters.
    void fill(unsigned y, unsigned x)
    {
	while (x < screen_width) {
	    mvaddch(y, x++, ' ');
	}
    }

    unsigned print_line_prefix(const unsigned y, const unsigned line_num, const unsigned line_len, const unsigned line_num_width)
    {
	unsigned x = 0;

	curses_attr a(A_REVERSE | color(COLOR_WHITE, COLOR_BLACK));

	const unsigned line_len_w = digits(line_len) + 1;
	const unsigned line_num_w = digits(line_num) + 1;

	// is there enough space to print the line length?
	if (line_len_w + line_num_w <= line_num_width) {
	    curses_attr a(A_BOLD);
	    mvprintw(y, x, "%u ", line_len);
	    x += line_len_w;
	}

	// print spaces to separate the numbers
	for (; x < line_num_width - line_num_w; ++x) {
	    mvaddch(y, x, ' ');
	}

	mvprintw(y, x, "%u ", line_num);
	x += line_num_w;

	return x;
    }

    void mvaddwch(unsigned y, unsigned x, wchar_t c)
    {
	wchar_t wc[2] = { c, 0 };
	mvaddwstr(y, x, wc);
    }

    void refresh_info()
    {
	if (info.empty()) {
	    return;
	}
	if (info.size() > screen_width) {
	    info.resize(screen_width);
	}
	curses_attr a(use_color() ? color(COLOR_RED, COLOR_BLACK) : A_BOLD);
	mvprintw(w_lines_height - 1, screen_width - info.size(), "%s", info.c_str());
    }

    void refresh_lines_window()
    {
	assert(tab_width > 0);
	link.clear();
	email.clear();
	clear_word_set();

	middle_line_number = 0;
	unsigned y = 0;
	if (display_info->start()) {
	    while (y < w_lines_height) {
		const line_number_t current_line_num = display_info->current();
		if (y < w_lines_height / 2) {
		    middle_line_number = current_line_num;
		}

		line_t line = f_idx->line(current_line_num);
		assert(current_line_num == line.num_);

		unsigned line_num_width = digits(current_line_num);
		if (line_num_width < tab_width) {
		    line_num_width = tab_width;
		}
		if (line_num_width < 8) {
		    line_num_width = 8;
		}

		// apply Replace Display Filters
		if (! line.empty()) {
		    static std::string l;
		    l = line.to_string();

		    for (auto df : regex_vec) {
			if (df->replace_df_rgx_) {
			    l = std::regex_replace(l, *(df->replace_df_rgx_), df->replace_df_text_);
			}
		    }

		    line.assign(l);
		}

		// handle empty line
		if (line.empty()) {
		    unsigned x = print_line_prefix(y, line.num_, 0, line_num_width);
		    fill(y, x);

		    // are we at the end of the lines window?
		    if (++y >= w_lines_height) {
			return;
		    }

		    // do we have another line to display?
		    if (display_info->next()) {
			continue; // there is a next line to display
		    }
		    else {
			break; // last line displayed
		    }
		}

		add_to_word_set(line.to_string());
		auto wline = to_wide(line.to_string());

		// map of pointers into the line and a corresponding curses attribute for the character
		std::map<std::wstring::iterator, unsigned> character_attr;

		// apply Attribute Display Filters
		for(auto df : regex_vec) {
		    if (df->attribute_df_rgx_) {
			for(auto it = std::wsregex_iterator(wline.begin(), wline.end(), *(df->attribute_df_rgx_)), it_end = std::wsregex_iterator(); it != it_end; ++it) {
			    std::wstring::iterator b = wline.begin() + it->position();
			    std::wstring::iterator e = b + it->length();
			    assert(b <= e);
			    // set character attribute for all matched characters
			    for (std::wstring::iterator i = b; i != e; ++i) {
				character_attr[i] &= ~A_COLOR; // clear any previous color
				character_attr[i] |= df->attribute_df_attr_; // set new attribute and color
			    }
			}
		    }
		}

		// apply search?
		if (search_err.empty()) {
		    // apply search regex to line
		    for (auto it = std::wsregex_iterator(wline.begin(), wline.end(), search_rgx); it != std::wsregex_iterator(); ++it) {
			std::wstring::iterator b = wline.begin() + it->position();
			std::wstring::iterator e = b + it->length();
			assert(b <= e);
			// set character attribute for all matched characters
			for (std::wstring::iterator i = b; i != e; ++i) {
			    character_attr[i] |= (use_color() ? (color(COLOR_GREEN, COLOR_BLACK) | A_BOLD) : A_REVERSE);
			}
		    }
		}

		// look for links
		std::map<std::wstring::iterator, std::wstring> iterator2link;
		for (auto it = std::wsregex_iterator(wline.begin(), wline.end(), link_rgx); it != std::wsregex_iterator(); ++it) {
		    std::wstring::iterator b = wline.begin() + it->position();
		    std::wstring::iterator e = b + it->length();
		    assert(b <= e);
		    std::wstring l(b, e);
		    // set character attribute for all matched characters
		    for (std::wstring::iterator i = b; i != e; ++i) {
			character_attr[i] |= A_UNDERLINE;
			iterator2link[i] = l;
		    }
		}
		// look for emails
		std::map<std::wstring::iterator, std::wstring> iterator2email;
		for (auto it = std::wsregex_iterator(wline.begin(), wline.end(), email_rgx); it != std::wsregex_iterator(); ++it) {
		    std::wstring::iterator b = wline.begin() + it->position();
		    std::wstring::iterator e = b + it->length();
		    assert(b <= e);
		    std::wstring l(b, e);
		    // set character attribute for all matched characters
		    for (std::wstring::iterator i = b; i != e; ++i) {
			character_attr[i] |= A_UNDERLINE;
			iterator2email[i] = l;
		    }
		}

		// print the current line
		auto it = wline.begin();
		while (it != wline.end() && y < w_lines_height) {
		    unsigned x = 0;
		    // block to print left info column
		    {
			// are we at the start of the line?
			if (it == wline.begin()) {
			    // print line number
			    x += print_line_prefix(y, line.num_, wline.size(), line_num_width);
			}
			else {
			    // print empty space
			    curses_attr a(A_REVERSE | color(COLOR_WHITE, COLOR_BLACK));
			    for (; x < line_num_width; ++x) {
				mvaddch(y, x, ' ');
			    }
			}
		    }
		    // print line in chunks of screen width
		    curses_attr a(gray_on_black);
		    for (; it != wline.end() && x < screen_width; ++it) {
			// check for link
			auto i = iterator2link.find(it);
			if (i != iterator2link.end()) {
			    link.emplace(std::make_pair(x, y), i->second);
			}
			// check for email
			i = iterator2email.find(it);
			if (i != iterator2email.end()) {
			    email.emplace(std::make_pair(x, y), i->second);
			}

			auto c = *it;
			// handle tab character
			if (c == '\t') {
			    do {
				mvaddch(y, x++, ' ');
			    } while (x % tab_width);
			}
			else {
			    // replace non printable characters with a space
			    if (iswprint(c)) {
				const unsigned attr = character_attr[it];
				curses_attr a(attr);
				mvaddwch(y, x, c);
			    }
			    else {
				mvaddwch(y, x, L'\uFFFD');
			    }
			    ++x;
			}
		    }
		    fill(y, x);

		    // are we at the end of the lines window?
		    if (++y >= w_lines_height) {
			return;
		    }
		}

		// did we display the full line?
		if (it == wline.end()) {
		    // do we have another line to display?
		    if (display_info->next()) {
			continue; // there is a next line to display
		    }
		    else {
			break; // last line displayed
		    }
		}
	    }
	}

	while (y < w_lines_height) {
	    fill(y++, 0);
	}
    }

    /**
     * print the string s at the screen at position x,y.
     * The string does not print beyond screen_width.
     * @param y vertical coordinate.
     * @param x horizontal coordinate.
     * @param s string.
     * @return number of characters printed.
     */
    unsigned print_string(const unsigned y, const unsigned x, std::string s)
    {
	if (x >= screen_width) {
	    return 0;
	}
	if (y >= screen_height) {
	    return 0;
	}
	if (s.empty()) {
	    return 0;
	}
	if (s.size() >= screen_width - x) {
	    s.resize(screen_width - x);
	}
	assert(x + s.size() <= screen_width);
	mvprintw(y, x, "%s", s.c_str());
	return s.size();
    }

    void refresh_regex_window(unsigned y)
    {
	unsigned cnt = 0;
	for(auto c : regex_vec) {
	    ++cnt;
	    std::string s = c->rgx_;
	    std::string title = "regex";
	    unsigned attr = A_REVERSE;

	    if (c->err_.empty()) {
		attr |= (cnt & 1) ? gray_on_black : lightgray_on_black;

		if (c->ri_) {
		    title = (cnt < 10) ? "filtr" : "filt";
		} else if (c->replace_df_rgx_) {
		    title = (cnt < 10) ? "disft" : "disf";
		} else if (c->attribute_df_rgx_) {
		    title = (cnt < 10) ? "dfatr" : "dfat";
		} else {
		    title = (cnt < 10) ? "     " : "    ";
		}
	    } else {
		attr |= color(COLOR_RED, COLOR_BLACK);
		s += " : ";
		s += c->err_;
		title = (cnt < 10) ? "error" : "err ";
	    }

	    curses_attr a(attr);

	    unsigned X = 0;
	    {
		curses_attr a(A_BOLD);
		mvprintw(y, X, "%s %u ", title.c_str(), cnt);
		X += 8;
	    }

	    if (!s.empty()) {
		X += print_string(y, X, s);

		if (c->ri_) {
		    curses_attr a(use_color() ? 0 : A_BOLD);
		    s = " (";
		    const uint64_t num = c->ri_->size();
		    s += std::to_string(num);
		    s += " match";
		    if (num != 1) {
			s += "es";
		    }
		    s += ", " + std::to_string(num * 100llu / f_idx->size()) + "%)";

		    X += print_string(y, X, s);
		}
	    }

	    fill(y, X);

	    ++y;
	}
    }

    void print_centered(const std::string s, int y_offset = 0)
    {
	int x = screen_width / 2 - s.size() / 2;
	if (x < 0) { x = 0; }
	int y = screen_height / 2 + y_offset;
	if (y < 0) { y = 0; }
	mvprintw(y, x, "%s", s.c_str());
    }

    void refresh_windows()
    {
	if (screen_height >= min_screen_height && screen_width >= min_screen_width) {
	    refresh_lines_window();
	    refresh_regex_window(filter_y);

	    if (!search_str.empty()) {
		curses_attr a(A_BOLD);
		mvprintw(search_y, 0, "Search: %s %s", search_str.c_str(), search_err.c_str());
		fill(search_y, 8 + search_str.size());
	    }
	}
	else if (screen_height < min_screen_height) {
	    erase();
	    print_centered("Minimum screen height is " + std::to_string(min_screen_height) + " lines.");
	    print_centered("Current screen height is " + std::to_string(screen_height) + " lines.", 1);
	}
	else if (screen_width < min_screen_width) {
	    erase();
	    print_centered("Min Width: " + std::to_string(min_screen_width));
	    print_centered("Cur Width: " + std::to_string(screen_width), 1);
	}
	else {
	    // this case should not happen
	    assert(false);
	}

	refresh();
    }

    void calculate_window_sizes()
    {
	if (screen_height < min_screen_height) {
	    return;
	}

	w_lines_height = screen_height;
	w_lines_height -= regex_vec.size();
	if (!search_str.empty()) {
	    --w_lines_height;
	}
	assert(w_lines_height > 0);

	// lines window
	unsigned y = w_lines_height;

	if (regex_vec.size() == 0) {
	    filter_y = 0;
	}
	else {
	    assert(regex_vec.size() <= 10+12);
	    filter_y = y;
	    y += regex_vec.size();
	}

	search_y = screen_height - 1;
    }
}
    void create_windows()
    {
	calculate_window_sizes();
	refresh_windows();
    }

    bool initialize_curses()
    {
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, false);
	keypad(stdscr, true);
	halfdelay(3);
	mousemask(BUTTON1_CLICKED, nullptr);
	curs_set(0); // disable cursor

	if (use_color()) {
	    use_color(has_colors() ? true : false);
	}
	if (use_color()) {
	    start_color();
	}

	create_windows();
	return true;
    }

    void close_curses()
    {
	endwin();
    }

namespace {
    void key_up()
    {
	if (display_info->isFirstLineDisplayed()) {
	    info = "moved to top";
	} else {
	    display_info->up();
	}
	refresh_lines_window();
	refresh();
    }

    void key_down()
    {
	if (display_info->isLastLineDisplayed()) {
	    info = "moved to bottom";
	} else {
	    display_info->down();
	}
	refresh_lines_window();
	refresh();
    }

    void key_npage()
    {
	if (display_info->isLastLineDisplayed()) {
	    info = "moved to bottom";
	} else {
	    display_info->page_down();
	}
	refresh_lines_window();
	refresh();
    }

    void key_ppage()
    {
	if (! display_info->start()) {
	    info = "nothing to display";
	} else if (display_info->isFirstLineDisplayed()) {
	    info = "moved to top";
	} else {
	    const line_number_t oldTopLineNum = display_info->current();

	    // scroll up one page, even if it is too far.
	    for(unsigned i = 0; i < w_lines_height - 1; ++i) {
		display_info->up();
	    }

	    // render lines
	    refresh_lines_window();

	    // now scroll down, until old top line is at the bottom
	    while(display_info->bottomLineNum() < oldTopLineNum) {
		display_info->down();
		refresh_lines_window();
	    }
	}

	refresh_lines_window();
	refresh();
    }

    // position display on top line
    void key_g()
    {
	if (display_info->isFirstLineDisplayed()) {
	    info = "moved to top";
	} else {
	    display_info->top();
	}
	refresh_lines_window();
	refresh();
    }

    // position display on bottom line
    void key_G()
    {
	if (! display_info->start()) {
	    info = "nothing to display";
	} else if (display_info->isLastLineDisplayed()) {
	    info = "moved to bottom";
	} else {
	    const line_number_t lastLineNum = display_info->lastLineNum();
	    display_info->go_to(lastLineNum);
	    // scroll up until we don't print the last line any more
	    while (!display_info->isFirstLineDisplayed()) {
		display_info->up();
		refresh_lines_window();
		if (display_info->bottomLineNum() != lastLineNum) {
		    break;
		}
	    }
	    // now scroll down again one line, so we see the last line
	    display_info->down();
	}
	refresh_lines_window();
	refresh();
    }

    void key_d()
    {
	if (middle_line_number == 0) {
	    return;
	}
	display_info->go_to(middle_line_number);
	refresh_lines_window();
	refresh();
    }

    void key_u()
    {
	if (middle_line_number == 0) {
	    return;
	}
	// scroll up until the old middle line number is the bottom line
	const line_number_t old_mln = middle_line_number;
	while (!display_info->isFirstLineDisplayed() && display_info->bottomLineNum() > old_mln) {
	    display_info->up();
	    refresh_lines_window();
	}
	refresh();
    }

    // \todo maybe background search?
    void key_n()
    {
	info = "searching...";
	refresh_lines_window();
	refresh_info();
	refresh();
	if (! search_next(search_rgx, display_info, f_idx)) {
	    info = "did not find any next search match";
	} else {
	    info = "next match found";
	}
	refresh_lines_window();
	refresh_info();
	refresh();
    }

    void key_N()
    {
	info = "searching...";
	refresh_lines_window();
	refresh_info();
	refresh();
	if (! search_prev(search_rgx, display_info, f_idx)) {
	    info = "did not find any next search match";
	} else {
	    info = "prev match found";
	}
	refresh_lines_window();
	refresh_info();
	refresh();
    }

    void key_h()
    {
#if defined(__unix__)
	// try to render a text version of the manpage
	try {
	    TemporaryFile tmp;
	    if (! run_command("MANWIDTH=" + std::to_string(screen_width - 10) + " man few > " + tmp.filename() + " 2> /dev/null", info)) {
		return;
	    }
	    run_program(argv0 + " " + tmp.filename(), info);
	} catch (std::exception& e) {
	    info = "could not show help: ";
	    info += e.what();
	} catch (...) {
	    info = "could not show help: unknown exception";
	}
#endif
    }

    void key_M()
    {
	maximize_window(info);
    }

    void key_mouse()
    {
	MEVENT e;
	if (getmouse(&e) != OK) {
	    return;
	}
	if (e.bstate & BUTTON1_CLICKED) {
	    // check for link
	    auto it = link.find(std::make_pair(static_cast<unsigned>(e.x), static_cast<unsigned>(e.y)));
	    if (it != link.end()) {
		const std::string l = to_utf8(it->second);
		if (click_link(l, info)) {
		    info = "opened " + l + " in browser";
		}
		refresh_lines_window();
		refresh();
	    }
	    // check for emails
	    it = email.find(std::make_pair(static_cast<unsigned>(e.x), static_cast<unsigned>(e.y)));
	    if (it != email.end()) {
		const std::string e = to_utf8(it->second);
		if (click_email(e, command_line_filename, info)) {
		    info = "created email to " + e;
		}
		refresh_lines_window();
		refresh();
	    }
	}
    }

    /**
     * provision the display_info object.
     * use the lines from f_idx and filter with the regular expression vector filter_vec.
     */
    void intersect_regex(ProgressFunctor *func)
    {
	// set up a vector regex_index lineNum_vector iterator pairs
	std::shared_ptr<regex_index> ri;
	lineNum_vector_intersect_vector_t v;
	for(auto c : regex_vec) {
	    if (c->ri_) {
		ri = c->ri_;
		const auto& s = ri->lineNum_vector();
		v.push_back(std::make_pair(s.begin(), s.end()));
	    }
	}

	lineNum_vector_t s;

	// if there are no regex_index objects found, show the complete file
	if (v.empty()) {
	    // file_index::lineNum_vector() return a temporary object which we can move
	    s = std::move(f_idx->lineNum_vector());
	} else if (v.size() == 1) {
	    // if there is only a single regex_index object, use that one
	    // regex_index::lineNum_vector() returns a const reference which we must copy
	    s = ri->lineNum_vector();
	} else {
	    multiple_set_intersect(v.begin(), v.end(), std::back_insert_iterator<lineNum_vector_t>(s));
	}

	display_info->assign(std::move(s));
    }

    void intersect_regex_curses()
    {
	CursesProgressFunctor func(screen_height / 2, screen_width / 2 - 10, A_REVERSE|A_BOLD, " intersect regex: ");
	intersect_regex(&func);
    }

    class CursesCursorHelper
    {
	const int prev_;
    public:
	CursesCursorHelper(int state) :
	    prev_(curs_set(state))
	{}
	~CursesCursorHelper()
	{
	    if (prev_ != ERR) {
		curs_set(prev_);
	    }
	}
    };

    History::ptr_t line_edit_history;

    /// the function pointer type of an autocomplete function.
    typedef std::set<std::string> (*autocomplete_f)(std::string& path, std::string& err);

    /**
     * read an input string with curses.
     * The input windows is positioned at coordinates x,y and has a maximum width of max_width.
     * If you press the enter/return key the function finishes and returns the currently edited string.
     * If you press the escape key or CTRL+g the function finishes and returns the input string.
     *
     * If the function pointer autocomplete_f is set and the tab key
     * is pressed, the function will call the autocomplete function
     * with the current edited string.
     *
     * @param y vertical coordinate.
     * @param x horizontal coordinate.
     * @param input initial string.
     * @param max_width maximum width of edit window. This will also limit the size of the returned string.
     * @param autocomplete_func function pointer to an auto complete function, can be NULL.
     * @return edited string.
     */
    std::string line_edit(const unsigned y,
			  const unsigned x,
			  const std::string& input,
			  const unsigned max_width,
			  autocomplete_f autocomplete_func)
    {
	static std::string killring;

	if (max_width < 1) {
	    return input;
	}

	std::string s = input;
	if (s.size() > max_width) {
	    s.resize(max_width);
	}

	History::iterator_t history_it;

	// the cursor position
	unsigned X = s.size();

	// set curses cursor to very visible
	CursesCursorHelper cch(2);

	// an info string displayed on the edit line
	std::string line_edit_info;

	// \todo remove the debug stuff
#define LINEEDITDEBUG 0
#if LINEEDITDEBUG
	int doj=-1, dojkey=-1;
#endif
	while(true) {
#if LINEEDITDEBUG
	    mvprintw(0,0,"'%s' %i doj=%i key=%i X+%i ", s.c_str(), s.size(), doj, dojkey, X);
	    mvprintw(1,0,"'%s' %i ", line_edit_info.c_str(), line_edit_info.size());
#endif

	    // print current line, filling up with spaces to max_width
	    std::string displayed_line = s + line_edit_info;
	    for(unsigned i = 0; i < max_width; ++i) {
		char c = ' ';
		if (i < displayed_line.size()) {
		    c = displayed_line[i];
		}
		mvaddch(y, x + i, c);
	    }
	    move(y, x+X); // position cursor
	    refresh();

	    int key = getch();

	    if (key == '\t' && autocomplete_func) {
		std::string err;
		auto autocomplete_set = autocomplete_func(s, err);
		const auto autocomplete_size = autocomplete_set.size();
		if (autocomplete_size == 0 && ! err.empty()) {
		    line_edit_info = "   (";
		    line_edit_info += err;
		    line_edit_info += ')';
#if LINEEDITDEBUG
		    doj=1;
#endif
		} else if (autocomplete_size == 1) {
		    //s = *(autocomplete_set.begin());
		    assert(s == *(autocomplete_set.begin()));
		    assert(! s.empty());
		    line_edit_info.clear();
#if LINEEDITDEBUG
		    doj=2;
#endif
		} else if (autocomplete_size > 1) {
		    line_edit_info = "   (";
		    for(const auto& fn : autocomplete_set) {
			line_edit_info += fn;
			line_edit_info += ' ';
		    }
		    line_edit_info += ") ";
		    line_edit_info += err;
#if LINEEDITDEBUG
		    doj=3;
#endif
		} else {
		    line_edit_info.clear();
#if LINEEDITDEBUG
		    doj=4;
#endif
		}
		key = KEY_END; // handle END key, to position cursor
	    }
#if LINEEDITDEBUG
	    dojkey=key;
#endif

	    switch(key) {
	    case ERR:
		break;

	    case '\r':
	    case '\n':
	    case KEY_ENTER:
		line_edit_history->add(s);
		return s;

	    case 27: // esc
	    case 'g'-96: // CTRL+g
		return input;

	    case KEY_BACKSPACE:
	    case 8:
		if (X > 0) {
		    s.erase(--X,1);
		}
		break;

	    case KEY_DC: // delete character
	    case 'd'-96: // CTRL+d
		s.erase(X,1);
		break;

	    case KEY_LEFT:
	    case 'b'-96: // CTRL+b
		if (X > 0) {
		    --X;
		}
		break;

	    case KEY_RIGHT:
	    case 'f'-96: // CTRL+f
		if (X < s.size()) {
		    ++X;
		}
		break;

	    case KEY_HOME:
	    case 'a'-96: // CTRL+a
		X = 0;
		break;

	    case KEY_END:
	    case 'e'-96: // CTRL+e
		X = s.size();
		break;

	    case 'k'-96: // CTRL+k
		{
		    std::string k = s;
		    k.erase(0,X);
		    if (k.size() > 0) {
			killring = k;
		    }
		    s.erase(X);
		}
		break;

	    case 'y'-96: // CTRL+y
		if (! killring.empty()) {
		    s.insert(X, killring);
		    X += killring.size();
		}
		break;

	    case KEY_UP:
		if (! history_it) {
		    history_it = line_edit_history->begin(s);
		}
		s = history_it->prev();
		X = s.size();
		break;

	    case KEY_DOWN:
		if (history_it) {
		    s = history_it->next();
		    X = s.size();
		    if (history_it->atEnd()) {
			history_it = nullptr;
		    }
		}
		break;

	    default:
		if (key >= 32 && key < 256 && s.size() < max_width) {
		    if (X == s.size()) {
			s += static_cast<char>(key);
		    } else {
			char str[2] = { static_cast<char>(key), 0 };
			s.insert(X, str);
		    }
		    ++X;
		}
		break;
	    } // switch(key)
	    // don't add anything below the switch block
	} // while(true)

	return s;
    }

    std::string& operator<< (std::string& s, std::regex_constants::error_type etype)
    {
	switch (etype) {
	case std::regex_constants::error_collate:
	    s += "invalid collating element request"; break;
	case std::regex_constants::error_ctype:
	    s += "invalid character class"; break;
	case std::regex_constants::error_escape:
	    s += "invalid escape character or trailing escape"; break;
	case std::regex_constants::error_backref:
	    s += "invalid back reference"; break;
	case std::regex_constants::error_brack:
	    s += "mismatched bracket [ or ]"; break;
	case std::regex_constants::error_paren:
	    s += "mismatched parentheses ( or )"; break;
	case std::regex_constants::error_brace:
	    s += "mismatched brace { or }"; break;
	case std::regex_constants::error_badbrace:
	    s += "invalid range inside a { }"; break;
	case std::regex_constants::error_range:
	    s += "invalid character range(e.g., [z-a])"; break;
	case std::regex_constants::error_space:
	    s += "insufficient memory to handle this regular expression"; break;
	case std::regex_constants::error_badrepeat:
	    s += "a repetition character (*, ?, +, or {) was not preceded by a valid regular expression"; break;
	case std::regex_constants::error_complexity:
	    s += "the requested match is too complex"; break;
	case std::regex_constants::error_stack:
	    s += "insufficient memory to evaluate a match"; break;
	}
	return s;
    }

    /**
     * parse all lines in fi and match with ri.
     * when done, add an event.
     * This function will be executed in a background thread.
     */
    void parse_regex(std::shared_ptr<file_index> fi, std::shared_ptr<regex_index> ri, const unsigned idx)
    {
	fi->parse_all_in_background(ri);
	eventAdd(event(ri, idx));
    }

    /// return values of the add_regex() function
    enum add_regex_status {
	foundInCache,
	startedBackgroundMatch,
	createdDisplayFilter,
	regexError,
    };

    add_regex_status add_regex(const unsigned regex_num, std::string rgx, ProgressFunctor *func)
    {
	assert(regex_num < max_regex_num);
	assert(! rgx.empty());

	// normalize regular expression
	rgx = normalize_regex(rgx);
	assert(rgx.size() >= 3);
	assert(rgx[0] == '/' || rgx[0] == '|');

	regex_vec_resize(regex_num + 1);

	// do we have the regex container already in the cache?
	const bool isFilterRgx = is_filter_regex(rgx);
	if (isFilterRgx) {
	    auto it = filter_cache.find(rgx);
	    if (it != filter_cache.end()) {
		// check that cache key really matches the value rgx_
		assert(it->first == it->second->rgx_);
		regex_vec[regex_num] = it->second;
		info = "found regex in cache";
		return foundInCache;
	    }
	}

	// create new regex container object
	auto c = std::make_shared<regex_container_t>();
	regex_vec[regex_num] = c;
	c->rgx_ = rgx;

	try {
	    uint64_t df_attr;
	    int df_fg, df_bg;

	    if (isFilterRgx) {
		// Lines Filter
		auto ri = std::make_shared<regex_index>(rgx);
		std::thread t(parse_regex, f_idx, ri, regex_num);
		t.detach();
		info = "matching...";
		return startedBackgroundMatch;
	    } else if (is_attr_df(rgx, df_attr, df_fg, df_bg)) {
		// Attribute Display Filter
		c->attribute_df_rgx_ = std::make_shared<std::wregex>(to_wide(get_regex_str(rgx)));
		c->attribute_df_attr_ = df_attr | color(df_fg, df_bg);
		info = "created new attribute display filter";
		return createdDisplayFilter;
	    } else {
		// Replace Display Filter

		// remove flags from rgx
		const std::string flags = get_regex_flags(rgx);
		if (! flags.empty()) {
		    if (flags.size() >= rgx.size()) {
			info = "malformed regex, strange flags";
			return regexError;
		    }
		    assert(rgx.size() > flags.size());
		    rgx.erase(rgx.size() - flags.size());
		}

		// parse flags
		bool positive_match = true;
		std::regex_constants::syntax_option_type fl;
		convert(flags, fl, positive_match);

		// separate regex and replacement
		std::string regex, rpl, err_msg;
		if (parse_replace_df(rgx, regex, rpl, err_msg)) {
		    c->replace_df_text_ = std::move(rpl);
		    c->replace_df_rgx_ = std::make_shared<std::regex>(regex, fl);
		    info = "created new replace display filter";
		} else {
		    c->err_ = "invalid replace display filter regex: " + err_msg;
		    info = "could not create replace display filter";
		    return regexError;
		}

		return createdDisplayFilter;
	    }
	} catch (std::regex_error& e) {
	    c->err_ << e.code();
	} catch (std::runtime_error& e) {
	    c->err_ = e.what();
	} catch (...) {
	    c->err_ = "caught unknown exception";
	}

	return regexError;
    }

    void edit_regex(unsigned& y, const unsigned regex_num)
    {
	assert(regex_num < max_regex_num);
	regex_vec_resize(regex_num + 1);

	create_windows();

	// get new regex string
	auto c = regex_vec[regex_num];
	std::string rgx;
	{
	    curses_attr a(A_REVERSE);
	    rgx = line_edit(y + regex_num, 8, c->rgx_, screen_width - 8, complete_word_set);
	    rgx = normalize_regex(rgx);
	}

	bool should_intersect = true;
	if (rgx.empty()) {
	    regex_vec[regex_num] = std::make_shared<regex_container_t>(); // overwrite with new/empty container object
	    // pop regular expression container from vector if they're empty
	    while(regex_vec.size() > 0 && regex_vec[regex_vec.size()-1]->rgx_.empty()) {
		regex_vec.resize(regex_vec.size() - 1);
	    }
	} else if (rgx == c->rgx_) {
	    // nothing changed, do nothing
	    should_intersect = false;
	} else {
	    CursesProgressFunctor func(screen_height / 2, screen_width / 2 - 10, A_REVERSE|A_BOLD, " matching line ");
	    const add_regex_status s = add_regex(regex_num, rgx, &func);
	    should_intersect = (s == foundInCache);
	    if (! is_filter_regex(rgx)) {
		should_intersect = false;
	    }
	}

	if (should_intersect) {
	    intersect_regex_curses();
	}
	create_windows();
    }

    void go_to_line()
    {
	static const std::string title = "Go To Line #: ";
	mvprintw(search_y, 0, "%s", title.c_str());
	std::string line_num = line_edit(search_y, title.size(), "", screen_width - title.size(), nullptr);
	if (line_num.empty()) {
	    return;
	}
	int64_t l_n = atoll(line_num.c_str());
	if (l_n < 1) {
	    info = "invalid line number: " + line_num;
	} else if (l_n > static_cast<int64_t>(std::numeric_limits<line_number_t>::max())) {
	    info = "line number too big: " + line_num;
	} else if (! display_info->go_to(static_cast<line_number_t>(l_n))) {
	    info = "line number " + line_num + " not currently displayed";
	}
	refresh_windows();
    }

    void go_to_perc()
    {
	static const std::string title = "Go To Percent %: ";
	mvprintw(search_y, 0, "%s", title.c_str());
	std::string perc = line_edit(search_y, title.size(), "", screen_width - title.size(), nullptr);
	if (perc.empty()) {
	    return;
	}
	int64_t p = atoll(perc.c_str());
	if (p < 0) {
	    info = "invalid percentage: " + perc;
	} else {
	    display_info->go_to_perc(static_cast<unsigned>(p));
	}
	refresh_windows();
    }

    /**
     * compile a regular expression string.
     * If str is the empty string, the function does not compile anything and returns the empty string.
     * @param[in] str regular expression string.
     * @param[out] regex compiled regular expression if function returns empty string.
     * @return empty string upon success; error string otherwise.
     */
    std::string compile_regex(std::string str, std::wregex& regex)
    {
	if (str.empty()) {
	    return str;
	}

	str = normalize_regex(str);
	std::string flags = get_regex_flags(str);
	std::string rgx = get_regex_str(str);
	std::regex_constants::syntax_option_type fl;
	bool positiveMatch;
	convert(flags, fl, positiveMatch);
	std::string err;
	try {
	    std::wregex r(to_wide(rgx), fl);
	    regex = r;
	} catch (std::regex_error& e) {
	    err << e.code();
	} catch (std::runtime_error& e) {
	    err = e.what();
	} catch (...) {
	    err = "caught unknown exception";
	}
	return err;
    }

    void compile_search_regex(const std::string& str)
    {
	search_str = normalize_regex(str);
	search_err = compile_regex(str, search_rgx);
	if (! search_err.empty()) {
	    search_err = ": " + search_err;
	}
    }

    void edit_search()
    {
	{
	    curses_attr a(A_BOLD);
	    const std::string title = "Search: ";
	    mvprintw(search_y, 0, title.c_str());
	    compile_search_regex( line_edit(search_y, title.size(), search_str, screen_width - title.size(), complete_word_set) );
	}
	create_windows();
    }

    void process_event_queue()
    {
	bool do_refresh_windows = false;
	bool do_intersect = false;

	while(eventPending()) {
	    event e = eventGet();
	    if (e.ri_) {
		assert(e.ri_idx_ < regex_vec.size());

		// get the regex_container_t
		auto c = regex_vec[e.ri_idx_];
		c->ri_ = e.ri_;
		filter_cache[c->rgx_] = c;

		do_intersect = true;
		do_refresh_windows = true;
		info.erase();
	    }
	    if (! e.info_.empty()) {
		info = e.info_;
	    }
	}

	if (do_intersect) {
	    intersect_regex_curses();
	}
	if (do_refresh_windows) {
	    refresh_windows();
	}
    }

    void key_S()
    {
	do {
	    curses_attr a(A_BOLD);
	    const std::string title = "Save to File: ";
	    mvprintw(search_y, 0, title.c_str());
	    const std::string filename = line_edit(search_y, title.size(), "", screen_width - title.size(), complete_filename);
	    if (filename.empty()) {
		break;
	    }
	    if (display_info->save(filename, *f_idx)) {
		info = "save success";
	    } else {
		info = "save failed: ";
		info += errno_str();
	    }
	} while(0);

	refresh_windows();
    }

#if defined(__unix__)
    // check for terminated child process
    void check_for_zombies()
    {
	int status = 0;
	pid_t pid = waitpid(-1, &status, WNOHANG);
	if (pid <= 0) {
	    return;
	}
	info = "child PID " + std::to_string(pid);
	if (WIFEXITED(status)) {
	    info += " exit " + std::to_string(WEXITSTATUS(status));
	} else if (WIFSIGNALED(status)) {
#ifdef WCOREDUMP
	    if (WCOREDUMP(status)) {
		info += " produced core dump";
	    } else
#endif
		{
		    info += std::string(" signal ") + strsignal(WTERMSIG(status));
		}
	} else {
	    info += " unknown exit";
	}
    }
#endif
}

void help();

int realmain_impl(int argc, char * const argv[])
{
    if (argc < 1) {
	help();
	return EX_USAGE;
    }
    if (argv == nullptr) {
	help();
	return EX_USAGE;
    }
    argv0 = argv[0];

    // merge environment variable FEWOPTIONS into the command line?
    std::string fo;
    if (getenv_str("FEWOPTIONS", fo) && !fo.empty()) {
	int argc_fo = -1;
	char ** argv_fo = nullptr;
	if (! tokenize_command_line(fo, argc_fo, argv_fo)) {
	    std::cerr << "could not tokenize FEWOPTIONS environment variable" << std::endl;
	    return EX_USAGE;
	}
	if (! merge_command_line_lists(argc, const_cast<const char**&>(argv), argc_fo, const_cast<const char**>(argv_fo))) {
	    std::cerr << "could not merge FEWOPTIONS into command line" << std::endl;
	    return EX_USAGE;
	}
    }

    enum {
	opt_tabwidth = 500,
	opt_regex,
	opt_search,
	opt_goto,
	opt_help,
	opt_color,
    };
    const struct option longopts[] = {
	{ "tabwidth", required_argument, nullptr, opt_tabwidth },
	{ "regex", required_argument, nullptr, opt_regex },
	{ "search", required_argument, nullptr, opt_search },
	{ "goto", required_argument, nullptr, opt_goto },
	{ "help", no_argument, nullptr, opt_help },
	{ "color", no_argument, nullptr, opt_color },
	{ nullptr, 0, nullptr, 0 }
    };

    line_number_t topLine = 0;
    std::vector<std::string> command_line_filter_regex;
    int key;
    while((key = getopt_long(argc, argv, "vh?", longopts, nullptr)) > 0) {
	switch(key) {
	case '?':
	case 'h':
	case opt_help:
	    help();
	    return EXIT_FAILURE;

	case 'v':
	    ++verbose;
	    break;

	case opt_color:
	    use_color(true);
	    break;

	case opt_regex:
	    if (command_line_filter_regex.size() >= max_regex_num) {
		std::cerr << "can only add up to " << max_regex_num << " regular expressions with the --regex argument" << std::endl;
		return EX_USAGE;
	    }
	    {
		std::string s = optarg;
		if (! s.empty()) {
		    command_line_filter_regex.push_back(s);
		}
	    }
	    break;

	case opt_search:
	    compile_search_regex(optarg);
	    break;

	case opt_tabwidth:
	    tab_width = atoi(optarg);
	    if (tab_width > 80) {
		std::cerr << "tab width is > 80, this is likely an error." << std::endl;
		return EX_USAGE;
	    }
	    if (tab_width == 0) {
		std::cerr << "tab width is 0, this is likely an error." << std::endl;
		return EX_USAGE;
	    }
	    break;

	case opt_goto:
	    topLine = atoi(optarg);
	    if (topLine < 1) {
		std::cerr << "--goto line number is invalid: " << optarg << std::endl;
		return EX_USAGE;
	    }
	    break;
	}
    }

    if (optind < argc) {
	command_line_filename = argv[optind];
    }

    // if we should read from STDIN, create a temporary file
    std::string real_filename = command_line_filename;
    std::shared_ptr<TemporaryFile> stdin_tmpfile;
    if (command_line_filename == "-") {
	stdin_tmpfile = std::make_shared<TemporaryFile>();
	real_filename = to_utf8(stdin_tmpfile->filename());
	FILE *f = stdin_tmpfile->file();
	if (!f) {
	    std::cerr << "could not open temporary file " << real_filename << " for writing: " << errno_str() << std::endl;
	    return EX_CANTCREAT;
	}
	if (verbose) {
	    std::clog << "read STDIN into " << real_filename << std::endl;
	}
	while(!feof(stdin)) {
	    char buf[65536];
	    int r = fread(buf, 1, sizeof(buf), stdin);
	    if (r > 0) {
		int w = fwrite(buf, 1, r, f);
		if (w != r) {
		    std::cerr << "could not write STDIN into temporary file: " << errno_str() << std::endl;
		    return EX_IOERR;
		}
	    } else if (r == 0) {
		break;
	    } else {
		std::cerr << "could not read from STDIN: " << errno_str() << std::endl;
		return EX_IOERR;
	    }
	}
	stdin_tmpfile->close();
	clearerr(stdin);

	if (! open_tty_as_stdin()) {
	    std::cerr << "could not open console directly." << std::endl;
	    return EX_IOERR;
	}
    }

    setlocale(LC_ALL, "");
    display_info = std::make_shared<DisplayInfo>();

    f_idx = std::make_shared<file_index>(real_filename);
    {
	file_index::regex_index_vec_t v;
	for(auto rgx_ : command_line_filter_regex) {
	    auto rgx = normalize_regex(rgx_);
	    if (filter_cache.count(rgx) > 0) {
		std::clog << "--regex '" << rgx << "' seen more than once." << std::endl;
		continue;
	    }
	    auto ri = std::make_shared<regex_index>(rgx);
	    v.push_back(ri);

	    auto c = std::make_shared<regex_container_t>();
	    c->rgx_ = rgx;
	    c->ri_ = ri;
	    filter_cache[rgx] = c;
	}
	OStreamProgressFunctor func(std::clog, "parsing line: ");
	f_idx->parse_all(v, &func);
    }
    for(unsigned u = 0; u != command_line_filter_regex.size(); ++u) {
	const add_regex_status s = add_regex(u, command_line_filter_regex[u], nullptr);
	if (s == createdDisplayFilter) {
	    continue;
	}
	if (s != foundInCache) {
	    std::cerr << "did not find cached regex '" << command_line_filter_regex[u] << "'" << std::endl;
	    return EX_SOFTWARE;
	}
    }
    {
	std::shared_ptr<OStreamProgressFunctor> func;
	if (command_line_filter_regex.size() > 0 && verbose) {
	    func = std::make_shared<OStreamProgressFunctor>(std::clog, "intersect regular expressions: ");
	}
	intersect_regex(func.get());
    }

    const std::string stdinfo = command_line_filename + " (" + std::to_string(f_idx->size()) + " lines)";
    info = stdinfo;

    line_edit_history = std::make_shared<History>(line_edit_history_rc);

    if (topLine > 0) {
	display_info->go_to_approx(topLine);
    }

    atexit(close_curses);
    if (! initialize_curses()) {
	return EX_UNAVAILABLE;
    }

    if (screen_width == 0) {
	std::cerr << "screen width is 0. Are you executing this program in an interactive terminal?" << std::endl;
	return EX_USAGE;
    }
    if (screen_height < min_screen_height) {
	std::cerr << "screen height is " << screen_height << " lines. Minimum required screen height is " << min_screen_height << " lines." << std::endl;
	return EX_USAGE;
    }

    while (true) {
	// loop until a key was pressed
	do {
	    refresh_info();
	    key = getch();
	    process_event_queue();
	} while(key == ERR);

	if (verbose) {
	    info= stdinfo + " "
		+ std::to_string(f_idx->perc(display_info->topLineNum())) + "%"
		+ " use " + std::to_string(getCurrentRSS()/1024/1024) + " MB"
		;
	} else {
	    info = stdinfo;
	}
#if defined(__unix__)
	check_for_zombies();
#endif
	// process key presses
	if (key == 'q' || key == 'Q') {
	    break;
	}
	switch(key) {
	case '/':
	    edit_search();
	    break;

	case 'p':
	case KEY_UP:
	    key_up();
	    break;

	case KEY_DOWN:
	    key_down();
	    break;

	case 'n':
	    if (search_str.empty()) {
		key_down();
	    } else {
		key_n();
	    }
	    break;

	case 'N':
	    if (search_str.empty()) {
		info = "no search regex";
	    } else {
		key_N();
	    }
	    break;

	case ' ':
	case KEY_NPAGE:
	    key_npage();
	    break;

	case 'b':
	case KEY_PPAGE:
	    key_ppage();
	    break;

	case KEY_HOME:
	case 'g':
	case '<':
	    key_g();
	    break;

	case KEY_END:
	case 'G':
	case '>':
	    key_G();
	    break;

	case 'd':
	    key_d();
	    break;

	case 'u':
	    key_u();
	    break;

	case 'R':
	    refresh_windows();
	    break;

	case 'h':
	    key_h();
	    break;

	case 'P':
	    go_to_line();
	    break;

	case '%':
	    go_to_perc();
	    break;

	case 'M':
	    key_M();
	    break;

	case 'S':
	    key_S();
	    break;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    edit_regex(filter_y, key - '1');
	    break;
	case '0':
	    edit_regex(filter_y, 9);
	    break;
	case KEY_F(1):
	case KEY_F(2):
	case KEY_F(3):
	case KEY_F(4):
	case KEY_F(5):
	case KEY_F(6):
	case KEY_F(7):
	case KEY_F(8):
	case KEY_F(9):
	    edit_regex(filter_y, key - KEY_F(1) + 10);
	    break;

	case KEY_MOUSE:
	    key_mouse();
	    break;

	case KEY_RESIZE:
	    create_windows();
	    info = "screen was resized " + std::to_string(screen_width) + "x" + std::to_string(screen_height);
	    break;
	}
    }

    // print command line
    close_curses();

    if (verbose) {
	std::clog << "peak memory use: " << getPeakRSS()/1024/1024 << " MB." << std::endl;
    }

    std::cout << "few";

    for(auto c : regex_vec) {
	if (c->rgx_.empty()) {
	    continue;
	}
	std::cout << " --regex '" << c->rgx_ << "'";
    }

    if (! search_str.empty()) {
	std::cout << " --search '" << search_str << "'";
    }

    display_info->start();

    std::cout << " --tabwidth " << tab_width;
    if (display_info->current() > 0) {
	std::cout << " --goto " << display_info->current();
    }
    for(unsigned u = 0; u < verbose; ++u) {
	std::cout << " -v";
    }
    if (use_color()) {
	std::cout << " --color";
    }
    std::cout << " '" << command_line_filename << "'" << std::endl;

    return EXIT_SUCCESS;
}

int realmain(int argc, char * const argv[])
{
    int exit_status = EXIT_FAILURE;
    std::string exit_msg;

    try {
	exit_status = realmain_impl(argc, argv);
    }
    catch (const error& e) {
	exit_msg = e.what();
	exit_status = e.exit_status();
    }
    catch (std::regex_error& e) {
	exit_msg << e.code();
    }
    catch (const std::exception& e) {
	exit_msg = e.what();
    }
    catch (...) {
	exit_msg = "caught unknown exception";
    }

    if (! exit_msg.empty()) {
	std::cerr << std::endl << exit_msg << std::endl;
    }

    f_idx = nullptr;
    return exit_status;
}
