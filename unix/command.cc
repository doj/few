/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "command.h"
#include "few_curses.h"
#include "errno_str.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

bool
run_command(const std::string& cmd, std::string& info_msg)
{
    int s = system(cmd.c_str());
    if (s < 0) {
	info_msg = "could not run: " + cmd;
    } else {
	if (WIFEXITED(s)) {
	    s = WEXITSTATUS(s);
	    info_msg = "command exit status: " + std::to_string(s);
	    return s == 0;
	} else if (WIFSIGNALED(s)) {
#ifdef WCOREDUMP
	    if (WCOREDUMP(s)) {
		info_msg = "command produced a core dump";
	    } else
#endif
		{
		    info_msg = std::string("command exited with signal: ") + strsignal(WTERMSIG(s));
		}
	} else {
	    info_msg = "unknown error when running: " + cmd;
	}
    }
    return false;
}

bool
run_program(const std::string& cmd, std::string& info_msg, bool wait_for_key)
{
    size_t size = 0;
    close_curses();
    if (wait_for_key) {
	std::string s = "execute: " + cmd + "\n\n";
	size = write(1, s.data(), s.size());
	assert(size == s.size());
    }
    const bool b = run_command(cmd, info_msg);
    if (wait_for_key) {
	std::string s = "\n[press enter to return to few]\n";
	size = write(1, s.data(), s.size());
	assert(size == s.size());
	char buf[128];
	size = read(0, buf, sizeof(buf)); // read from STDIN to wait for enter key press
	assert(size >= 0);
    }
    initialize_curses();
    return b;
}

int
run_command_background(std::string cmd, std::string& info_msg)
{
    const pid_t pid = fork();
    if (pid < 0) {
	info_msg = "could not fork: " + errno_str();
	return -1;
    } else if (pid == 0) {
	// child
	cmd += " > /dev/null 2>&1";
	int s = system(cmd.c_str());
	// _exit() will not run the atexit() handlers.
	if (WIFEXITED(s)) {
	    _exit(WEXITSTATUS(s));
	}
	_exit(EXIT_FAILURE);
    } else {
	info_msg = "created child PID " + std::to_string(pid);
    }

    return pid;
}
