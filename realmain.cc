/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include <sysexits.h>
#include <iostream>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <ncurses.h>
#include <cassert>
#include <stdint.h>
#include <memory>
#include <regex>
#include "memorymap.h"
#include "file_index.h"
#include "regex_index.h"

namespace {
    /// width of screen in characters
    unsigned screen_width;
    /// height of screen in characters
    unsigned screen_height;

    /// width of a tab character in characters
    unsigned tab_width = 8;

    /// current search regular expression
    std::string search;
    // the y position of the search window
    unsigned search_y;

    // the y position of the regex window
    unsigned regex_y;

    unsigned top_line = 1;
    file_index *f_idx = nullptr;

    /// height of the lines window
    unsigned w_lines_height;

    struct regex_container_t
    {
	/// the regular expression string
	std::string rgx_;
	/// an error string if rgx_ is invalid
	std::string err_;
	std::shared_ptr<regex_index> r_idx_;
    };

    std::vector<regex_container_t> regex_vec;

    /// the number of lines currently displayed in the lines window. Can be less than lines window height.
    unsigned lines_currently_displayed;

    /// true if refresh_lines() printed the last line of the file
    bool last_line_printed;





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

    void refresh_lines()
    {
	lines_currently_displayed = 0;
	last_line_printed = false;

	// the number of the last line printed
	unsigned last_line_num = 0;

	// intersect lines from file with regular expression matches
	auto s = f_idx->index_set();
	for(const auto& c : regex_vec) {
	    if (c.r_idx_) {
		s = c.r_idx_->intersect(s);
	    }
	}

	unsigned y = 0;
	for(auto i : s) {
	    if (y >= w_lines_height) {
		break;
	    }
	    if (i < top_line) {
		continue;
	    }

	    ++lines_currently_displayed;

	    const line_t line = f_idx->line(i);
	    unsigned line_num_width = digits(i);
	    if (line_num_width < tab_width) {
		line_num_width = tab_width;
	    }

	    // empty line?
	    if (line.beg_ == line.end_) {
		unsigned x = print_line_prefix(y, line.num_, 0, line_num_width);
		fill(y, x);
		++y;
		last_line_num = line.num_;
		continue;
	    }

	    const char *beg = line.beg_;
	    while(beg < line.end_)
	    {
		unsigned x = 0;
		{
		    curses_attr a(A_REVERSE);
		    if (beg == line.beg_) {
			x += print_line_prefix(y, line.num_, line.end_ - beg, line_num_width);
			last_line_num = line.num_;
		    } else {
			for(; x < line_num_width; ++x) {
			    mvaddch(y, x, ' ');
			}
		    }
		}
		while(beg < line.end_ && x < screen_width) {
		    char c = *beg++;
		    if (c == '\t') {
			do {
			    mvaddch(y, x++, ' ');
			} while (x % tab_width);
		    } else {
			if (! isprint(c)) {
			    c = ' ';
			}
			mvaddch(y, x++, c);
		    }
		}
		fill(y, x);
		++y;
	    }
	}

	if (last_line_num == f_idx->lines()) {
	    last_line_printed = true;
	}

	while(y < w_lines_height) {
	    fill(y++, 0);
	}
    }

    void refresh_regex()
    {
	curses_attr a(A_REVERSE);
	unsigned y = regex_y;
	unsigned cnt = 0;
	for(const auto& c : regex_vec) {
	    std::string s = c.rgx_;
	    const char* title = nullptr;

	    if (c.err_.empty()) {
		title = "regex";
	    } else {
		s += " : ";
		s += c.err_;
		title = "error";
	    }

	    {
		curses_attr a(A_BOLD);
		mvprintw(y, 0, "%s %u ", title, ++cnt);
	    }
	    if (s.size() > screen_width - 8) {
		s.resize(screen_width - 8);
	    }
	    assert(8 + s.size() < screen_width);
	    mvprintw(y, 8, "%s", s.c_str());
	    fill(y, 8 + s.size());

	    ++y;
	}
    }

    void refresh_windows()
    {
	refresh_lines();
	refresh_regex();

	if (! search.empty()) {
	    curses_attr a(A_BOLD);
	    mvprintw(search_y, 0, "Search: %s", search.c_str());
	    fill(search_y, 8+search.size());
	}

	refresh();
    }

    void calculate_window_sizes()
    {
	w_lines_height = screen_height;
	if (! search.empty()) {
	    --w_lines_height;
	}

	if (regex_vec.size() == 0) {
	    regex_y = 0;
	} else {
	    assert(regex_vec.size() <= 9);
	    w_lines_height -= regex_vec.size();
	    regex_y = w_lines_height;
	    assert(regex_y > 0);
	}

	search_y = screen_height - 1;
    }

    void create_windows()
    {
	calculate_window_sizes();
	refresh_windows();
    }

    void enter_search()
    {
	search = "Dirk";
	create_windows();
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

    void key_up_impl()
    {
	if (top_line > 1) {
	    --top_line;
	}
    }

    void key_up()
    {
	key_up_impl();
	refresh_lines();
	refresh();
    }

    void key_down_impl()
    {
	if (last_line_printed) {
	    return;
	}

	++top_line;
    }

    void key_down()
    {
	key_down_impl();
	refresh_lines();
	refresh();
    }

    void key_npage()
    {
	for(unsigned i = 0; i < lines_currently_displayed; ++i) {
	    key_down_impl();
	}
	refresh_lines();
	refresh();
    }

    void key_ppage()
    {
	for(unsigned i = 0; i < w_lines_height; ++i) {
	    key_up_impl();
	}
	refresh_lines();
	refresh();
    }

    void key_g()
    {
	if (top_line != 1) {
	    top_line = 1;
	    refresh_lines();
	    refresh();
	}
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

    void edit_regex(unsigned regex_num)
    {
	if (regex_num >= 9) {
	    return;
	}

	// check if we need to expand regex_vec
	if (regex_vec.size() <= regex_num) {
	    regex_vec.resize(regex_num + 1);
	}

	create_windows();

	// get new regex string
	auto& c = regex_vec[regex_num];
	{
	    curses_attr a(A_REVERSE);
	    c.rgx_ = line_edit(regex_y + regex_num, 8, c.rgx_, screen_width - 8);
	}

	if (c.rgx_.empty()) {
	    c.err_.erase();
	    c.r_idx_ = nullptr;
	    // pop regular expression container from vector if they're empty
	    while(regex_vec.size() > 0 && regex_vec[regex_vec.size()-1].rgx_.empty()) {
		regex_vec.resize(regex_vec.size() - 1);
	    }
	} else {
	    std::string rgx = c.rgx_;
	    // check for flags
	    std::string flags;
	    unsigned s = rgx.size();
	    if (s >= 4 &&
		rgx[0] == '/' &&
		rgx[s - 2] == '/' &&
		(rgx[s - 1] == 'i' || rgx[s - 1] == '!') ) {
		flags = rgx[s - 1];
		rgx.erase(s - 2);
		rgx.erase(0, 1);
	    } else if (s > 1 && rgx[0] == '!') {
		flags = "!";
		rgx.erase(0, 1);
	    }

	    try {
		c.r_idx_ = std::make_shared<regex_index>(*f_idx, rgx, flags);
		c.err_.erase();
	    } catch (std::regex_error& e) {
		c.err_ << e.code();
	    }
	}

	create_windows();
    }
}

int realmain(int argc, const char* argv[])
{
    if (argc < 2) {
	return EX_USAGE;
    }
    if (argv == nullptr) {
	return EX_USAGE;
    }

    doj::memorymap_ptr<char> file_ptr(argv[1]);
    if (file_ptr.empty()) {
	std::cerr << "could not memory map: " << argv[1] << std::endl;
	return EX_NOINPUT;
    }

    setlocale(LC_ALL, "");

    get_screen_size();
    if (screen_width == 0) {
	std::cerr << "screen width is 0. Are you executing this program in an interactive terminal?" << std::endl;
	return EX_USAGE;
    }
    signal(SIGWINCH, handle_winch);

    file_index fi(file_ptr);
    f_idx = &fi;

    atexit(close_curses);
    initialize_curses();

    while(true) {
	int key = getch();
	if (key == 'q') {
	    break;
	}
	switch(key) {
	case '/': enter_search(); break;
	case KEY_UP: key_up(); break;
	case KEY_DOWN: key_down(); break;

	case 'n':
	    if (search.empty()) {
		key_down();
	    } else {

	    }
	    break;

	case 'p':
	    if (search.empty()) {
		key_up();
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
	    key_g();
	    break;

	case KEY_END:
	case 'G':
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
	    edit_regex(key - '1');
	    break;

	case '0':
	    break;
	}
    }

    return EXIT_SUCCESS;
}
