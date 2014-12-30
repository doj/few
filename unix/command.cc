/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "command.h"
#include "few_curses.h"
#include "errno_str.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

bool
run_command(const std::string& cmd, std::string& error_msg)
{
    int s = system(cmd.c_str());
    if (s < 0) {
	error_msg = "could not run: " + cmd;
    } else {
	if (WIFEXITED(s)) {
	    s = WEXITSTATUS(s);
	    if (s == 0) {
		return true;
	    } else {
		error_msg = "command exit status: " + std::to_string(s);
	    }
	} else if (WIFSIGNALED(s)) {
#ifdef WCOREDUMP
	    if (WCOREDUMP(s)) {
		error_msg = "command produced a core dump";
	    } else
#endif
		{
		    error_msg = std::string("command exited with signal: ") + strsignal(WTERMSIG(s));
		}
	} else {
	    error_msg = "unknown error when running: " + cmd;
	}
    }
    return false;
}

bool
run_program(const std::string& cmd, std::string& error_msg)
{
    close_curses();
    const bool b = run_command(cmd, error_msg);
    initialize_curses();
    return b;
}

int
run_command_background(std::string cmd, std::string& msg)
{
    const pid_t pid = fork();
    if (pid < 0) {
	msg = "could not fork: " + errno_str();
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
	msg = "created child PID " + std::to_string(pid);
    }

    return pid;
}
