/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

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
#include "file_index.h"
#include "regex_index.h"
#include "error.h"
#include "display_info.h"
#include "normalize_regex.h"
#include "curses_attr.h"

namespace {
    /// verbosity level
    unsigned verbose = 0;

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
    std::regex search_rgx;
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
	std::shared_ptr<regex_index> r_idx_;

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

    /// an information string that is displayed in the lower right corner of the lines window
    std::string info;

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

    void refresh_lines_window()
    {
	assert(tab_width > 0);

	unsigned y = 0;
	if (display_info.start()) {
	    while(y < w_lines_height) {
		const unsigned current_line_num = display_info.current();
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
			goto display_lines_done;
		    }

		    // do we have another line to display?
		    if (display_info.next()) {
			continue; // there is a next line to display
		    } else {
			break; // last line displayed
		    }
		}

		// map of pointers into the line and a corresponding curses attribute for the character
		std::map<const char*, unsigned> character_attr;

		// apply search?
		if (search_err.empty()) {
		    // apply search regex to line
		    for(auto it = std::cregex_iterator(line.beg_, line.end_, search_rgx); it != std::cregex_iterator(); ++it ) {
			const char *b = line.beg_ + it->position();
			const char *e = b + it->length();
			assert(b <= e);
			// set character attribute for all matched characters
			for(const char *it = b; it != e; ++it) {
			    character_attr[it] |= A_REVERSE;
			}
		    }
		}

		// print the current line
		const char *beg = line.beg_;
		while(beg < line.end_ && y < w_lines_height) {
		    unsigned x = 0;
		    // block to print left info column
		    {
			curses_attr a(A_REVERSE);
			// are we at the start of the line?
			if (beg == line.beg_) {
			    // print line number
			    x += print_line_prefix(y, line.num_, line.end_ - beg, line_num_width);
			} else {
			    // print empty space
			    for(; x < line_num_width; ++x) {
				mvaddch(y, x, ' ');
			    }
			}
		    }
		    // print line in chunks of screen width
		    for(; beg < line.end_ && x < screen_width; ++beg) {
			char c = *beg;
			// handle tab character
			if (c == '\t') {
			    do {
				mvaddch(y, x++, ' ');
			    } while (x % tab_width);
			} else {
			    // replace non printable characters with a space
			    if (! isprint(c)) {
				c = ' ';
			    }
			    curses_attr a(character_attr[beg]);
			    mvaddch(y, x++, c);
			}
		    }
		    fill(y, x);

		    // are we at the end of the lines window?
		    if (++y >= w_lines_height) {
			goto display_lines_done;
		    }
		}

		// did we display the full line?
		if (beg == line.end_) {
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

    display_lines_done:
	if (! info.empty()) {
	    if (info.size() > screen_width) {
		info.resize(screen_width);
	    }
	    curses_attr a(A_BOLD);
	    mvprintw(w_lines_height - 1, screen_width - info.size(), "%s", info.c_str());
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

		if (c->r_idx_) {
		    curses_attr a(A_BOLD);
		    s = " (";
		    const unsigned num = c->r_idx_->size();
		    s += std::to_string(num);
		    s += ") match";
		    if (num != 1) {
			s += "es";
		    }
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
	    const unsigned oldTopLineNum = display_info.current();
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
	    const unsigned lastLineNum = display_info.lastLineNum();
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

    /**
     * provision the display_info object.
     * use the lines from f_idx and filter with the regular expression vector filter_vec.
     */
    void apply_regex(ProgressFunctor *func)
    {
	const unsigned total_steps = filter_vec.size() + 3;
	unsigned cnt = 0;
	++cnt; func->progress(1, cnt * 100 / total_steps);

	// get lines from file
	auto s = f_idx->lineNum_set();
	++cnt; func->progress(2, cnt * 100 / total_steps);

	// intersect lines from file with regular expression matches
	for(auto c : filter_vec) {
	    if (c->r_idx_) {
		s = c->r_idx_->intersect(s);
		++cnt; func->progress(3, cnt * 100 / total_steps);
	    }
	}
	func->progress(4, cnt * 100 / total_steps);

	display_info = s;
	func->progress(5, 100);
    }

    std::vector<std::string> line_edit_history;

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

	auto history_it = line_edit_history.rbegin();

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
	    case '\r':
	    case '\n':
	    case KEY_ENTER:
		line_edit_history.push_back(s);
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
		// save current edit state in history
		if (history_it == line_edit_history.rbegin()) {
		    line_edit_history.push_back(s);
		    history_it = line_edit_history.rbegin();
		    ++history_it;
		}
		if (history_it != line_edit_history.rend()) {
		    s = *history_it;
		    X = s.size();
		    ++history_it;
		}
		break;

	    case KEY_DOWN:
		if (history_it != line_edit_history.rbegin()) {
		    --history_it;
		    s = *history_it;
		    if (history_it == line_edit_history.rbegin()) {
			assert(line_edit_history.size() > 0);
			line_edit_history.resize(line_edit_history.size() - 1);
			history_it = line_edit_history.rbegin();
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

    void add_regex(const unsigned regex_num, std::string rgx, regex_vec_t& vec, const bool isFilterRgx, ProgressFunctor *func)
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
		return;
	    }
	}

	// create new regex container object
	auto c = std::make_shared<regex_container_t>();
	c->rgx_ = rgx;

	const std::string flags = get_regex_flags(rgx);
	rgx = get_regex_str(rgx);

	try {
	    if (isFilterRgx) {
		// Lines Filter
		c->r_idx_ = std::make_shared<regex_index>(f_idx, rgx, flags, func);
	    } else {
		// Display Filter

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
	    }
	} catch (std::regex_error& e) {
	    c->err_ << e.code();
	} catch (std::runtime_error& e) {
	    c->err_ = e.what();
	} catch (...) {
	    c->err_ = "caught unknown exception";
	}

	vec[regex_num] = c;
	if (isFilterRgx) {
	    filter_cache[c->rgx_] = c;
	}
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

	if (rgx.empty()) {
	    vec[regex_num] = std::make_shared<regex_container_t>(); // overwrite with new/empty container object
	    // pop regular expression container from vector if they're empty
	    while(vec.size() > 0 && vec[vec.size()-1]->rgx_.empty()) {
		vec.resize(vec.size() - 1);
	    }
	} else if (rgx == c->rgx_) {
	    // nothing changed, do nothing
	} else {
	    CursesProgressFunctor func(screen_height / 2, screen_width / 2 - 10, A_REVERSE|A_BOLD, " add regex: ");
	    add_regex(regex_num, rgx, vec, isFilterRgx, &func);
	}

	{
	    CursesProgressFunctor func(screen_height / 2, screen_width / 2 - 10, A_REVERSE|A_BOLD, " apply regex: ");
	    apply_regex(&func);
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
    std::string compile_regex(std::string str, std::regex& regex)
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
	    std::regex r(rgx, fl);
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
    };
    const struct option longopts[] = {
	{ "tabwidth", required_argument, nullptr, opt_tabwidth },
	{ "regex", required_argument, nullptr, opt_regex },
	{ "df", required_argument, nullptr, opt_df },
	{ "search", required_argument, nullptr, opt_search },
	{ nullptr, 0, nullptr, 0 }
    };

    std::vector<std::string> command_line_filter_regex, command_line_df_regex;
    int key;
    while((key = getopt_long(argc, argv, "vh", longopts, nullptr)) > 0) {
	switch(key) {
	case '?':
	case 'h':
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
	OStreamProgressFunctor func(std::clog, "parsing line: ");
	f_idx->parse_all(&func);
    }
    for(unsigned u = 0; u != command_line_filter_regex.size(); ++u) {
	OStreamProgressFunctor func(std::clog, "apply regex: " + command_line_filter_regex[u] + " : ");
	add_regex(u, command_line_filter_regex[u], filter_vec, true, &func);
    }
    for(unsigned u = 0; u != command_line_df_regex.size(); ++u) {
	OStreamProgressFunctor func(std::clog, "apply regex: " + command_line_df_regex[u] + " : ");
	add_regex(u, command_line_df_regex[u], df_vec, false, &func);
    }
    {
	OStreamProgressFunctor func(std::clog, "apply regular expressions: ");
	apply_regex(&func);
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

    atexit(close_curses);
    initialize_curses();

    while(true) {
	info = stdinfo;

	key = getch();
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

	case '0':
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
    std::cout << "fewer";

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

    std::cout << " --tabwidth " << tab_width
	      << " '" << filename << "'" << std::endl;
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
