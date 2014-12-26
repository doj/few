/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include <wchar.h>
#include <getopt.h>
#include <sysexits.h>
#include <iostream>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <cassert>
#include <stdint.h>
#include <stdlib.h>
#include <memory>
#include <regex>
#include <map>
#include <fstream>
#include <thread>
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

/// verbosity level
unsigned verbose = 0;

namespace {
    /// file name of line edit history
    const char* line_edit_history_rc = ".few.history";

    /// the info string which is shown in the lower right corner
    std::string info;

    /// minimum screen height
    const unsigned min_screen_height = 1 // lines
	+ 9 // filters
	+ 9 // display filters
	+ 1 // search
	;

    /// minimum screen width
    const unsigned min_screen_width = 16;

    /// width of screen in characters
    unsigned screen_width;
    /// height of screen in characters
    unsigned screen_height;

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

    /// the file that is displayed
    std::shared_ptr<file_index> f_idx;

    /// object to manage displayed lines
    DisplayInfo display_info;

    /// height of the lines window
    unsigned w_lines_height;

    /// line number displayed at the middle of the lines window
    line_number_t middle_line_number = 0;

    /// the y position of the lines filter regex window
    unsigned filter_y;

    /// the y position of the display filter regex window
    unsigned df_y;

    struct regex_container_t
    {
	/// the regular expression string
	std::string rgx_;
	/// an error string if rgx_ is invalid
	std::string err_;
	/// object used for lines filter
	std::shared_ptr<regex_index> ri_;

	///@{

	/// regex object used for display filter
	std::shared_ptr<std::regex> df_rgx_;
	/// display filter replacement
	std::string df_replace_;

	///@}
    };

    typedef std::vector<std::shared_ptr<regex_container_t>> regex_vec_t;

    void regex_vec_resize(regex_vec_t& vec, const unsigned num)
    {
	while(vec.size() < num) {
	    vec.push_back(std::make_shared<regex_container_t>());
	}
    }

    /**
     * type of a regex_container_t cache.
     * key is the regular expression string and has to be equal to value->rgx_.
     * value is a shared pointer to the regex container object.
     */
    typedef std::map<std::string, std::shared_ptr<regex_container_t>> regex_cache_t;

    /// the regular expressions for the lines filter
    regex_vec_t filter_vec;

    /// the filter regex cache
    regex_cache_t filter_cache;

    /// the regular expressions for the display filter
    regex_vec_t df_vec;

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
	while(x < screen_width) {
	    mvaddch(y, x++, ' ');
	}
    }

    unsigned print_line_prefix(const unsigned y, const unsigned line_num, const unsigned line_len, const unsigned line_num_width)
    {
	unsigned x = 0;

	curses_attr a(A_REVERSE);

	const unsigned line_len_w = digits(line_len) + 1;
	const unsigned line_num_w = digits(line_num) + 1;

	// is there enough space to print the line length?
	if (line_len_w + line_num_w <= line_num_width) {
	    curses_attr a(A_BOLD);
	    mvprintw(y, x, "%u ", line_len);
	    x += line_len_w;
	}

	// print spaces to separate the numbers
	for(; x < line_num_width - line_num_w; ++x) {
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
	curses_attr a(A_BOLD);
	mvprintw(w_lines_height - 1, screen_width - info.size(), "%s", info.c_str());
    }

    void refresh_lines_window()
    {
	assert(tab_width > 0);

	middle_line_number = 0;
	unsigned y = 0;
	if (display_info.start()) {
	    while(y < w_lines_height) {
		const line_number_t current_line_num = display_info.current();
		if (y < w_lines_height/2) {
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

		// apply Display Filters?
		if (! df_vec.empty() && !line.empty()) {
		    static std::string l;
		    l = line.to_string();

		    for(auto df : df_vec) {
			if (df->df_rgx_) {
			    l = std::regex_replace(l, *(df->df_rgx_), df->df_replace_);
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
		    if (display_info.next()) {
			continue; // there is a next line to display
		    } else {
			break; // last line displayed
		    }
		}

		auto wline = to_wide(std::string(line.beg_, line.end_));

		// map of pointers into the line and a corresponding curses attribute for the character
		std::map<std::wstring::iterator, unsigned> character_attr;

		// apply search?
		if (search_err.empty()) {
		    // apply search regex to line
		    for(auto it = std::wsregex_iterator(wline.begin(), wline.end(), search_rgx); it != std::wsregex_iterator(); ++it ) {
			std::wstring::iterator b = wline.begin() + it->position();
			std::wstring::iterator e = b + it->length();
			assert(b <= e);
			// set character attribute for all matched characters
			for(std::wstring::iterator i = b; i != e; ++i) {
			    character_attr[i] |= A_REVERSE;
			}
		    }
		}

		// print the current line
		auto it = wline.begin();
		while(it != wline.end() && y < w_lines_height) {
		    unsigned x = 0;
		    // block to print left info column
		    {
			curses_attr a(A_REVERSE);
			// are we at the start of the line?
			if (it == wline.begin()) {
			    // print line number
			    x += print_line_prefix(y, line.num_, wline.size(), line_num_width);
			} else {
			    // print empty space
			    for(; x < line_num_width; ++x) {
				mvaddch(y, x, ' ');
			    }
			}
		    }
		    // print line in chunks of screen width
		    for(; it != wline.end() && x < screen_width; ++it) {
			auto c = *it;
			// handle tab character
			if (c == '\t') {
			    do {
				mvaddch(y, x++, ' ');
			    } while (x % tab_width);
			} else {
			    // replace non printable characters with a space
			    if (iswprint(c)) {
				curses_attr a(character_attr[it]);
				mvaddwch(y, x, c);
			    } else {
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
		    if (display_info.next()) {
			continue; // there is a next line to display
		    } else {
			break; // last line displayed
		    }
		}
	    }
	}

	while(y < w_lines_height) {
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
	if (s.size() > screen_width - x) {
	    s.resize(screen_width - x);
	}
	assert(x + s.size() < screen_width);
	mvprintw(y, x, "%s", s.c_str());
	return s.size();
    }

    void refresh_regex_window(unsigned y, const std::string& title_param, const regex_vec_t& vec)
    {
	curses_attr a(A_REVERSE);
	unsigned cnt = 0;
	for(auto c : vec) {
	    std::string s = c->rgx_;
	    std::string title = title_param;

	    if (! c->err_.empty()) {
		s += " : ";
		s += c->err_;
		title = "error";
	    }

	    unsigned X = 0;
	    {
		curses_attr a(A_BOLD);
		mvprintw(y, X, "%s %u ", title.c_str(), ++cnt);
		X += 8;
	    }

	    if (! s.empty()) {
		X += print_string(y, X, s);

		if (c->ri_) {
		    curses_attr a(A_BOLD);
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

    void print_centered(const std::string s)
    {
	int x = screen_width / 2 - s.size() / 2;
	if (x < 0) { x = 0; }
	mvprintw(screen_height / 2, x, "%s", s.c_str());
    }

    void refresh_windows()
    {
	if (screen_height >= min_screen_height && screen_width >= min_screen_width) {
	    refresh_lines_window();
	    refresh_regex_window(filter_y, "regex", filter_vec);
	    refresh_regex_window(df_y, "dispf", df_vec);

	    if (! search_str.empty()) {
		curses_attr a(A_BOLD);
		mvprintw(search_y, 0, "Search: %s %s", search_str.c_str(), search_err.c_str());
		fill(search_y, 8 + search_str.size());
	    }
	} else if (screen_height < min_screen_height) {
	    erase();
	    print_centered("Minimum screen height is " + std::to_string(min_screen_height) + " lines.");
	} else if (screen_width < min_screen_width) {
	    erase();
	    print_centered("Min Width: " + std::to_string(min_screen_width));
	} else {
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
	w_lines_height -= filter_vec.size();
	w_lines_height -= df_vec.size();
	if (! search_str.empty()) {
	    --w_lines_height;
	}
	assert(w_lines_height > 0);

	// lines window
	unsigned y = w_lines_height;

	if (filter_vec.size() == 0) {
	    filter_y = 0;
	} else {
	    assert(filter_vec.size() <= 9);
	    filter_y = y;
	    y += filter_vec.size();
	}

	if (df_vec.size() == 0) {
	    df_y = 0;
	} else {
	    assert(df_vec.size() <= 9);
	    df_y = y;
	    y += df_vec.size();
	}

	search_y = screen_height - 1;
    }

    void create_windows()
    {
	calculate_window_sizes();
	refresh_windows();
    }

    void initialize_curses()
    {
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, false);
	keypad(stdscr, true);
	halfdelay(3);
	//start_color();

	create_windows();
    }

    void close_curses()
    {
	endwin();
    }

    void get_screen_size()
    {
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	screen_width = w.ws_col;
	screen_height = w.ws_row;
    }

    void handle_winch(int sig)
    {
	signal(SIGWINCH, SIG_IGN);
	close_curses();
	get_screen_size();
	initialize_curses();
	signal(SIGWINCH, handle_winch);
    }

    void key_up()
    {
	if (display_info.isFirstLineDisplayed()) {
	    info = "moved to top";
	} else {
	    display_info.up();
	}
	refresh_lines_window();
	refresh();
    }

    void key_down()
    {
	if (display_info.isLastLineDisplayed()) {
	    info = "moved to bottom";
	} else {
	    display_info.down();
	}
	refresh_lines_window();
	refresh();
    }

    void key_npage()
    {
	if (display_info.isLastLineDisplayed()) {
	    info = "moved to bottom";
	} else {
	    display_info.page_down();
	}
	refresh_lines_window();
	refresh();
    }

    void key_ppage()
    {
	if (! display_info.start()) {
	    info = "nothing to display";
	} else if (display_info.isFirstLineDisplayed()) {
	    info = "moved to top";
	} else {
	    // scroll up until the old top line is the current bottom line
	    const line_number_t oldTopLineNum = display_info.current();
	    while (!display_info.isFirstLineDisplayed()) {
		display_info.up();
		refresh_lines_window();
		if (display_info.bottomLineNum() <= oldTopLineNum) {
		    break;
		}
	    }
	}

	refresh_lines_window();
	refresh();
    }

    // position display on top line
    void key_g()
    {
	if (display_info.isFirstLineDisplayed()) {
	    info = "moved to top";
	} else {
	    display_info.top();
	}
	refresh_lines_window();
	refresh();
    }

    // position display on bottom line
    void key_G()
    {
	if (! display_info.start()) {
	    info = "nothing to display";
	} else if (display_info.isLastLineDisplayed()) {
	    info = "moved to bottom";
	} else {
	    const line_number_t lastLineNum = display_info.lastLineNum();
	    display_info.go_to(lastLineNum);
	    // scroll up until we don't print the last line any more
	    while (!display_info.isFirstLineDisplayed()) {
		display_info.up();
		refresh_lines_window();
		if (display_info.bottomLineNum() != lastLineNum) {
		    break;
		}
	    }
	    // now scroll down again one line, so we see the last line
	    display_info.down();
	}
	refresh_lines_window();
	refresh();
    }

    void key_d()
    {
	if (middle_line_number == 0) {
	    return;
	}
	display_info.go_to(middle_line_number);
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
	while (!display_info.isFirstLineDisplayed() && display_info.bottomLineNum() > old_mln) {
	    display_info.up();
	    refresh_lines_window();
	}
	refresh();
    }

    /**
     * provision the display_info object.
     * use the lines from f_idx and filter with the regular expression vector filter_vec.
     */
    void intersect_regex(ProgressFunctor *func)
    {
	// set up a vector of ILineNumSetProvider
	typedef std::vector<std::shared_ptr<ILineNumSetProvider>> v_t;
	v_t v;
	v.push_back(f_idx);
	for(auto c : filter_vec) {
	    if (c->ri_) {
		v.push_back(c->ri_);
	    }
	}

	// a lambda function to iterate the vector and intersect the sets
	auto f = [](v_t::iterator begin, v_t::iterator end, lineNum_set_t& s, ProgressFunctor *func)
	    {
		unsigned cnt = 0;
		if (func) func->progress(++cnt, 0);
		s = (*begin)->lineNum_set();
		while(++begin != end) {
		    if (func) func->progress(++cnt, 0);
		    s = (*begin)->intersect(s);
		}
	    };

	lineNum_set_t s;
	if (v.size() < 4) {
	    // main thread works through v
	    f(v.begin(), v.end(), s, func);
	} else {
	    // use two threads to work on the vector
	    const unsigned mid = v.size() / 2u;
	    lineNum_set_t r;
	    std::thread t1(f, v.begin(), v.begin() + mid, std::ref(s), nullptr);
	    std::thread t2(f, v.begin() + mid, v.end(), std::ref(r), nullptr);
	    if (func) func->progress(0, 0);
	    t1.join();
	    if (func) func->progress(1, 0);
	    t2.join();
	    if (func) func->progress(2, 0);
	    // if any set is empty, there's nothing to intersect
	    if (s.empty() || r.empty()) {
		s.clear();
	    } else {
		// ensure that s has less elements than r
		if (s.size() > r.size()) {
		    s.swap(r);
		}
		// loop over s and remove all elements not present in r
		foreach_e(s, it) {
		    if (r.count(*it) == 0) {
			s.erase(it);
		    }
		    if (s.empty()) {
			break;
		    }
		}
	    }
	    if (func) func->progress(3, 0);
	}
	if (func) func->progress(4, 0);
	display_info = s;
	if (func) func->progress(5, 0);
    }

    void intersect_regex_curses()
    {
	CursesProgressFunctor func(screen_height / 2, screen_width / 2 - 10, A_REVERSE|A_BOLD, " intersect regex: ");
	intersect_regex(&func);
    }

    History::ptr_t line_edit_history;

    /**
     * read an input string with curses.
     * The input windows is positioned at coordinates x,y and has a maximum width of max_width.
     * If you press the enter/return key the function finishes and returns the currently edited string.
     * If you press the escape key the function finished and returns the input string.
     *
     * @param y vertical coordinate.
     * @param x horizontal coordinate.
     * @param input initial string.
     * @param max_width maximum width of edit window. This will also limit the size of the returned string.
     * @return edited string.
     */
    std::string line_edit(const unsigned y, const unsigned x, const std::string& input, const unsigned max_width)
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

	while(true) {
	    for(unsigned i = 0; i < max_width; ++i) {
		mvaddch(y, x + i, ' ');
	    }
	    mvprintw(y, x, "%s", s.c_str());
	    move(y, x+X);
	    refresh();

	    const int key = getch();
	    switch(key) {
	    case ERR:
		break;

	    case '\r':
	    case '\n':
	    case KEY_ENTER:
		line_edit_history->add(s);
		return s;

	    case '\e':
		return input;

	    case KEY_BACKSPACE:
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
	    }

	}

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

    add_regex_status add_regex(const unsigned regex_num, std::string rgx, regex_vec_t& vec, const bool isFilterRgx, ProgressFunctor *func)
    {
	assert(regex_num < 9);
	assert(! rgx.empty());

	// normalize regular expression
	rgx = normalize_regex(rgx);
	assert(rgx.size() >= 3);
	assert(rgx[0] == '/');

	regex_vec_resize(vec, regex_num + 1);

	// do we have the regex container already in the cache?
	if (isFilterRgx) {
	    auto it = filter_cache.find(rgx);
	    if (it != filter_cache.end()) {
		// check that cache key really matches the value rgx_
		assert(it->first == it->second->rgx_);
		vec[regex_num] = it->second;
		info = "found regex in cache";
		return foundInCache;
	    }
	}

	// create new regex container object
	auto c = std::make_shared<regex_container_t>();
	vec[regex_num] = c;
	c->rgx_ = rgx;

	try {
	    if (isFilterRgx) {
		// Lines Filter
		auto ri = std::make_shared<regex_index>(rgx);
		std::thread t(parse_regex, f_idx, ri, regex_num);
		t.detach();
		info = "matching...";
		return startedBackgroundMatch;
	    } else {
		// Display Filter
		const std::string flags = get_regex_flags(rgx);
		rgx = get_regex_str(rgx);

		// parse flags
		bool positive_match = true;
		std::regex_constants::syntax_option_type fl;
		convert(flags, fl, positive_match);

		// separate regex and replacement
		std::string::size_type pos = rgx.find('/');
		if (pos == std::string::npos) {
		    c->err_ = "could not separate regex and replace parts";
		} else {
		    // create replace string
		    c->df_replace_ = rgx;
		    c->df_replace_.erase(0, pos + 1);

		    // construct regex
		    rgx.erase(pos);
		    c->df_rgx_ = std::make_shared<std::regex>(rgx, fl);
		}

		info = "created new display filter";
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

    void edit_regex(unsigned& y, const unsigned regex_num, regex_vec_t& vec, const bool isFilterRgx)
    {
	assert(regex_num < 9);
	regex_vec_resize(vec, regex_num + 1);

	create_windows();

	// get new regex string
	auto c = vec[regex_num];
	std::string rgx;
	{
	    curses_attr a(A_REVERSE);
	    rgx = line_edit(y + regex_num, 8, c->rgx_, screen_width - 8);
	    rgx = normalize_regex(rgx);
	}

	bool should_intersect = true;
	if (rgx.empty()) {
	    vec[regex_num] = std::make_shared<regex_container_t>(); // overwrite with new/empty container object
	    // pop regular expression container from vector if they're empty
	    while(vec.size() > 0 && vec[vec.size()-1]->rgx_.empty()) {
		vec.resize(vec.size() - 1);
	    }
	} else if (rgx == c->rgx_) {
	    // nothing changed, do nothing
	    should_intersect = false;
	} else {
	    CursesProgressFunctor func(screen_height / 2, screen_width / 2 - 10, A_REVERSE|A_BOLD, " matching line ");
	    const add_regex_status s = add_regex(regex_num, rgx, vec, isFilterRgx, &func);
	    should_intersect = (s == foundInCache);
	}

	if (isFilterRgx && should_intersect) {
	    intersect_regex_curses();
	}
	create_windows();
    }

    void go_to_line()
    {
	static const std::string title = "Go To Line #: ";
	mvprintw(search_y, 0, "%s", title.c_str());
	std::string line_num = line_edit(search_y, title.size(), "", screen_width - title.size());
	if (line_num.empty()) {
	    return;
	}
	int64_t l_n = atoll(line_num.c_str());
	if (l_n < 1) {
	    info = "invalid line number: " + line_num;
	} else if (l_n > static_cast<int64_t>(std::numeric_limits<line_number_t>::max())) {
	    info = "line number too big: " + line_num;
	} else if (! display_info.go_to(l_n)) {
	    info = "line number " + line_num + " not currently displayed";
	}
	refresh_windows();
    }

    void go_to_perc()
    {
	static const std::string title = "Go To Percent %: ";
	mvprintw(search_y, 0, "%s", title.c_str());
	std::string perc = line_edit(search_y, title.size(), "", screen_width - title.size());
	if (perc.empty()) {
	    return;
	}
	int64_t p = atoll(perc.c_str());
	if (p < 0) {
	    info = "invalid percentage: " + perc;
	} else {
	    display_info.go_to_perc(p);
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
	    mvprintw(search_y, 0, "Search: ");
	    compile_search_regex( line_edit(search_y, 8, search_str, screen_width - 8) );
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
		assert(e.ri_idx_ < 9);
		assert(e.ri_idx_ < filter_vec.size());

		// get the regex_container_t
		auto c = filter_vec[e.ri_idx_];
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

}

void help();

int realmain_impl(int argc, char * const argv[])
{
    if (argc < 2) {
	help();
	return EX_USAGE;
    }
    if (argv == nullptr) {
	help();
	return EX_USAGE;
    }

    enum {
	opt_tabwidth = 500,
	opt_regex,
	opt_df,
	opt_search,
	opt_goto,
	opt_help,
    };
    const struct option longopts[] = {
	{ "tabwidth", required_argument, nullptr, opt_tabwidth },
	{ "regex", required_argument, nullptr, opt_regex },
	{ "df", required_argument, nullptr, opt_df },
	{ "search", required_argument, nullptr, opt_search },
	{ "goto", required_argument, nullptr, opt_goto },
	{ "help", no_argument, nullptr, opt_help },
	{ nullptr, 0, nullptr, 0 }
    };

    line_number_t topLine = 0;
    std::vector<std::string> command_line_filter_regex, command_line_df_regex;
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

	case opt_regex:
	    if (command_line_filter_regex.size() >= 9) {
		std::cerr << "can only add up to 9 regular expressions with the --regex argument" << std::endl;
		return EX_USAGE;
	    }
	    {
		std::string s = optarg;
		if (! s.empty()) {
		    command_line_filter_regex.push_back(s);
		}
	    }
	    break;

	case opt_df:
	    if (command_line_df_regex.size() >= 9) {
		std::cerr << "can only add up to 9 display filters with the --df argument" << std::endl;
		return EX_USAGE;
	    }
	    {
		std::string s = optarg;
		if (! s.empty()) {
		    command_line_df_regex.push_back(s);
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

    if (optind >= argc) {
	std::cerr << "no filename specified" << std::endl;
	return EX_USAGE;
    }

    const std::string filename = argv[optind];

    setlocale(LC_ALL, "");

    f_idx = std::make_shared<file_index>(filename);
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
	const add_regex_status s = add_regex(u, command_line_filter_regex[u], filter_vec, true, nullptr);
	if (s != foundInCache) {
	    std::cerr << "did not find cached regex '" << command_line_filter_regex[u] << "'" << std::endl;
	    return EX_SOFTWARE;
	}
    }
    for(unsigned u = 0; u != command_line_df_regex.size(); ++u) {
	const add_regex_status s = add_regex(u, command_line_df_regex[u], df_vec, false, nullptr);
	assert(s == createdDisplayFilter);
    }
    {
	std::shared_ptr<OStreamProgressFunctor> func;
	if (command_line_filter_regex.size() > 0 && verbose) {
	    func = std::make_shared<OStreamProgressFunctor>(std::clog, "intersect regular expressions: ");
	}
	intersect_regex(func.get());
    }

    const std::string stdinfo = filename + " (" + std::to_string(f_idx->size()) + " lines)";
    info = stdinfo;

    get_screen_size();
    if (screen_width == 0) {
	std::cerr << "screen width is 0. Are you executing this program in an interactive terminal?" << std::endl;
	return EX_USAGE;
    }
    if (screen_height < min_screen_height) {
	std::cerr << "screen height is " << screen_height << " lines. Minimum required screen height is " << min_screen_height << " lines." << std::endl;
	return EX_USAGE;
    }
    signal(SIGWINCH, handle_winch);

    line_edit_history = std::make_shared<History>(std::string(getenv("HOME")) + "/" + line_edit_history_rc);

    if (topLine > 0) {
	display_info.go_to_approx(topLine);
    }

    atexit(close_curses);
    initialize_curses();

    while(true) {
	// loop until a key was pressed
	do {
	    refresh_info();
	    key = getch();
	    process_event_queue();
	} while(key == ERR);

	if (verbose) {
	    info= stdinfo + " "
		+ std::to_string(f_idx->perc(display_info.topLineNum())) + "%"
		+ " use " + std::to_string(getCurrentRSS()/1024/1024) + " MB"
		;
	} else {
	    info = stdinfo;
	}

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

	case 'P':
	    go_to_line();
	    break;

	case '%':
	    go_to_perc();
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
	    edit_regex(filter_y, key - '1', filter_vec, true);
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
	    edit_regex(df_y, key - KEY_F(1), df_vec, false);
	    break;
	}
    }

    // print command line
    close_curses();

    if (verbose) {
	std::clog << "peak memory use: " << getPeakRSS()/1024/1024 << " MB." << std::endl;
    }

    std::cout << "few";

    for(auto c : filter_vec) {
	if (c->rgx_.empty()) {
	    continue;
	}
	std::cout << " --regex '" << c->rgx_ << "'";
    }

    for(auto c : df_vec) {
	if (c->rgx_.empty()) {
	    continue;
	}
	std::cout << " --df '" << c->rgx_ << "'";
    }

    if (! search_str.empty()) {
	std::cout << " --search '" << search_str << "'";
    }

    display_info.start();

    std::cout << " --tabwidth " << tab_width;
    if (display_info.current() > 0) {
	std::cout << " --goto " << display_info.current();
    }
    std::cout << " '" << filename << "'" << std::endl;

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
