/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <string>

/**
 * run a command in a child process.
 * wait for the command to finish.
 *
 * If an error occurs from running the command line or if the exit
 * status of cmd is not 0, the info string will show an error
 * message.
 *
 * @param[in] cmd shell command line to run.
 * @param[out] error_msg will contain an error message if the function returns false.
 * @return true upon success.
 */
bool run_command(const std::string& cmd, std::string& error_msg);

/**
 * run a program in a child process.
 * The program can be interactive, as the curses library is closed before running the program.
 * @param[in] cmd shell command line to run.
 * @param[out] error_msg will contain an error message if the function returns false.
 * @return true if cmd had an exit status of 0.
 */
bool run_program(const std::string& cmd, std::string& error_msg);

/**
 * run a command in the background, detached from the current process.
 * @param[in] cmd shell command line to execute in background.
 * @param[out] msg will contain a message about success or failure to run cmd.
 * @return process ID of the background program; -1 upon error.
 */
int run_command_background(std::string cmd, std::string& msg);
