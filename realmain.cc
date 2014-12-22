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
#include <ncurses.h>
#include <cassert>
#include <stdint.h>
#include <stdlib.h>
#include <memory>
#include <regex>
#include "file_index.h"
#include "regex_index.h"
#include "error.h"
#include "display_info.h"
#include "normalize_regex.h"

namespace {
    /// width of screen in characters
    unsigned screen_width;
    /// height of screen in characters
    unsigned screen_height;

    /// width of a tab character in characters
    unsigned tab_width = 8;

    /// current search regular expression
    std::string search_rgx;
    // the y position of the search window
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

	void clear()
	{
	    rgx_.erase();
	    err_.erase();
	    r_idx_ = nullptr;
	    df_rgx_ = nullptr;
	    df_replace_.erase();
	}
    };

    typedef std::vector<regex_container_t> regex_vec_t;

    /// the regular expressions for the lines filter
    regex_vec_t filter_vec;

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

    /**
     * helper class to manage curses attributes.
     * The constructor sets an attribute, the destructor unsets it.
     */
    class curses_attr
    {
	const unsigned a_;
    public:
	explicit curses_attr(unsigned a) : a_(a) { attron(a_); }
	~curses_attr() { attroff(a_); }
    };

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

		    for(const auto& df : df_vec) {
			if (df.df_rgx_) {
			    l = std::regex_replace(l, *(df.df_rgx_), df.df_replace_);
			}
		    }

		    line.assign(l);
		}

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
		    while(beg < line.end_ && x < screen_width) {
			char c = *beg++;
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
	for(const auto& c : vec) {
	    std::string s = c.rgx_;
	    std::string title = title_param;

	    if (! c.err_.empty()) {
		s += " : ";
		s += c.err_;
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

		if (c.r_idx_) {
		    curses_attr a(A_BOLD);
		    s = " (";
		    const unsigned num = c.r_idx_->size();
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

    void refresh_windows()
    {
	refresh_lines_window();
	refresh_regex_window(filter_y, "regex", filter_vec);
	refresh_regex_window(df_y, "dispf", df_vec);

	if (! search_rgx.empty()) {
	    curses_attr a(A_BOLD);
	    mvprintw(search_y, 0, "Search: %s", search_rgx.c_str());
	    fill(search_y, 8 + search_rgx.size());
	}

	refresh();
    }

    void calculate_window_sizes()
    {
	w_lines_height = screen_height;
	w_lines_height -= filter_vec.size();
	w_lines_height -= df_vec.size();
	if (! search_rgx.empty()) {
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
		if (display_info.bottomLineNum() == oldTopLineNum) {
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
    void apply_regex()
    {
	// get lines from file
	auto s = f_idx->lineNum_set();

	// intersect lines from file with regular expression matches
	for(const auto& c : filter_vec) {
	    if (c.r_idx_) {
		s = c.r_idx_->intersect(s);
	    }
	}

	display_info = s;
    }

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
	if (max_width < 1) {
	    return input;
	}

	std::string s = input;
	if (s.size() > max_width) {
	    s.resize(max_width);
	}

	// the cursor position
	unsigned X = s.size();

	while(true) {
	    for(unsigned i = 0; i < max_width; ++i) {
		mvaddch(y, x + i, ' ');
	    }
	    mvprintw(y, x, "%s", s.c_str());
	    refresh();

	    const int key = getch();
	    switch(key) {
	    case '\r':
	    case '\n':
	    case KEY_ENTER:
		return s;

	    case '\e':
		return input;

	    case KEY_BACKSPACE:
		if (s.size() > 0) {
		    s.resize(s.size() - 1);
		}
		break;

	    default:
		if (key >= 32 && key < 256 && s.size() < max_width) {
		    s += static_cast<char>(key);
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

    void add_regex(const unsigned regex_num, std::string rgx, regex_vec_t& vec, const bool isFilterRgx)
    {
	assert(regex_num < 9);
	assert(! rgx.empty());

	// normalize regular expression
	rgx = normalize_regex(rgx);
	assert(rgx.size() >= 3);
	assert(rgx[0] == '/');

	// check if we need to expand vec
	if (vec.size() <= regex_num) {
	    vec.resize(regex_num + 1);
	}

	auto& c = vec[regex_num];
	c.clear();
	c.rgx_ = rgx;

	// check for flags
	std::string flags;
	unsigned last_idx = rgx.size() - 1;
	while(last_idx >= 2) {
	    const char last_c = rgx[last_idx];
	    rgx.erase(last_idx); // erase last character
	    if (last_c == '/') {
		break;
	    }
	    flags += last_c;
	    --last_idx;
	}
	rgx.erase(0, 1); // erase first '/'

	try {
	    if (isFilterRgx) {
		// Lines Filter
		c.r_idx_ = std::make_shared<regex_index>(f_idx, rgx, flags);
	    } else {
		// Display Filter

		// parse flags
		bool positive_match = true;
		std::regex_constants::syntax_option_type fl;
		convert(flags, fl, positive_match);

		// separate regex and replacement
		std::string::size_type pos = rgx.find('/');
		if (pos == std::string::npos) {
		    c.err_ = "could not separate regex and replace parts";
		} else {
		    // create replace string
		    c.df_replace_ = rgx;
		    c.df_replace_.erase(0, pos + 1);

		    // construct regex
		    rgx.erase(pos);
		    c.df_rgx_ = std::make_shared<std::regex>(rgx, fl);
		}
	    }
	} catch (std::regex_error& e) {
	    c.err_ << e.code();
	} catch (std::runtime_error& e) {
	    c.err_ = e.what();
	} catch (...) {
	    c.err_ = "caught unknown exception";
	}
    }

    void edit_regex(unsigned& y, const unsigned regex_num, regex_vec_t& vec, const bool isFilterRgx)
    {
	assert(regex_num < 9);

	// check if we need to expand vec
	if (vec.size() <= regex_num) {
	    vec.resize(regex_num + 1);
	}

	create_windows();

	// get new regex string
	auto& c = vec[regex_num];
	{
	    curses_attr a(A_REVERSE);
	    c.rgx_ = line_edit(y + regex_num, 8, c.rgx_, screen_width - 8);
	}

	if (c.rgx_.empty()) {
	    c.clear();
	    // pop regular expression container from vector if they're empty
	    while(vec.size() > 0 && vec[vec.size()-1].rgx_.empty()) {
		vec.resize(vec.size() - 1);
	    }
	    // c may be invalid at this point
	} else {
	    add_regex(regex_num, c.rgx_, vec, isFilterRgx);
	}

	apply_regex();
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

    void edit_search()
    {
	{
	    curses_attr a(A_BOLD);
	    mvprintw(search_y, 0, "Search: ");
	    search_rgx = normalize_regex( line_edit(search_y, 8, search_rgx, screen_width - 8) );
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
    while((key = getopt_long(argc, argv, "", longopts, nullptr)) > 0) {
	switch(key) {
	case '?':
	case 'h':
	    help();
	    return EXIT_FAILURE;

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
	    search_rgx = normalize_regex(optarg);
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
    for(unsigned u = 0; u != command_line_filter_regex.size(); ++u) {
	add_regex(u, command_line_filter_regex[u], filter_vec, true);
    }
    for(unsigned u = 0; u != command_line_df_regex.size(); ++u) {
	add_regex(u, command_line_df_regex[u], df_vec, false);
    }
    apply_regex();

    const std::string stdinfo = filename + " (" + std::to_string(f_idx->size()) + " lines)";
    info = stdinfo;

    get_screen_size();
    if (screen_width == 0) {
	std::cerr << "screen width is 0. Are you executing this program in an interactive terminal?" << std::endl;
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
	    if (search_rgx.empty()) {
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

    for(const auto& c : filter_vec) {
	if (c.rgx_.empty()) {
	    continue;
	}
	std::cout << " --regex '" << c.rgx_ << "'";
    }

    for(const auto& c : df_vec) {
	if (c.rgx_.empty()) {
	    continue;
	}
	std::cout << " --df '" << c.rgx_ << "'";
    }

    if (! search_rgx.empty()) {
	std::cout << " --search '" << search_rgx << "'";
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
