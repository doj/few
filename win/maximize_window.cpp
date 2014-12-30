/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#include "../maximize_window.h"
#include <Windows.h>
#undef MOUSE_MOVED // this macro is defined in Windows.h and curses.h
#include "../curses_attr.h"
#include "getlasterror_str.h"
void create_windows();

bool
maximize_window(std::string& error_msg)
{
    if (!SetWindowPos(GetConsoleWindow(),
	nullptr, // HWND hWndInsertAfter
	0, // x
	0, // y
	0, // width
	0, // height
	SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW)) {
	error_msg = GetLastError_str();
	return false;
    }

    // idea taken from
    // http://stackoverflow.com/questions/18818600/method-to-tell-pdcurses-to-make-the-window-the-same-size-as-the-physical-screen
    // Resize the terminal to something larger than the physical screen
    resize_term(2000, 2000);
    // Get the largest physical screen dimensions
    int r = 0, c = 0;
    getmaxyx(stdscr, r, c);
    // Resize so it fits
    resize_term(r - 1, c - 1);
    create_windows();

    return true;
}