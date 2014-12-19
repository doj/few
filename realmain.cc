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
#include "memorymap.h"
#include "file_index.h"

namespace {
    unsigned screen_width;
    unsigned screen_height;

    std::string search;
    unsigned search_y;

    unsigned top_line = 1;
    file_index *f_idx = nullptr;
    unsigned w_lines_height;

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

    class curses_attr
    {
	unsigned a_;
    public:
	explicit curses_attr(unsigned a) :
	a_(a)
	{
	    attron(a_);
	}
	~curses_attr()
	{
	    attroff(a_);
	}
    };

    void fill(unsigned y, unsigned x)
    {
	while(x < screen_width) {
	    mvaddch(y, x++, ' ');
	}
    }

    void refresh_lines()
    {
	unsigned y = 0;
	auto s = f_idx->index_set();
	for(auto i : s) {
	    if (y >= w_lines_height) {
		break;
	    }
	    if (i < top_line) {
		continue;
	    }

	    const line_t line = f_idx->line(i);
	    const unsigned line_num_width = digits(i) + 1;

	    // empty line?
	    if (line.beg_ == line.end_) {
		{
		    curses_attr a(A_REVERSE);
		    mvprintw(y, 0, "%i:", line.num_);
		}
		fill(y, line_num_width);
		++y;
		continue;
	    }

	    const char *beg = line.beg_;
	    while(beg < line.end_)
	    {
		{
		    curses_attr a(A_REVERSE);
		    if (beg == line.beg_) {
			mvprintw(y, 0, "%i:", line.num_);
		    } else {
			for(unsigned x = 0; x < line_num_width; ++x) {
			    mvaddch(y, x, ' ');
			}
		    }
		}
		unsigned x = line_num_width;
		while(beg < line.end_ && x < screen_width) {
		    mvaddch(y, x++, *beg++);
		}
		fill(y, x);
		++y;
	    }
	}

	while(y < w_lines_height) {
	    fill(y++, 0);
	}
    }

    void refresh_windows()
    {
	refresh_lines();

	if (! search.empty()) {
	    mvprintw(search_y, 0, "Search: %s", search.c_str());
	}

	refresh();
    }

    void create_windows()
    {
	w_lines_height = screen_height;
	if (! search.empty()) {
	    --w_lines_height;
	}

	search_y = screen_height - 1;

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

    void key_up()
    {
	if (top_line > 1) --top_line;
	refresh_lines();
	refresh();
    }

    void key_down_impl()
    {
	// \todo check that we really can move further down.
	++top_line;
    }

    void key_down()
    {
	refresh_lines();
	refresh();
    }

    void key_npage()
    {
	for(unsigned i = 0; i < w_lines_height; ++i) {
	    key_down_impl();
	}
	refresh_lines();
	refresh();
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

	case ' ':
	case KEY_NPAGE:
	    key_npage();
	    break;

	case 'b':
	case KEY_PPAGE:
	    break;

	case 'g':
	    break;

	case 'G':
	    break;
	}
    }

    return EXIT_SUCCESS;
}
