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
 * @param[out] info_msg will contain a message about the command execution.
 * @return true upon success.
 */
bool run_command(const std::string& cmd, std::string& info_msg);

/**
 * run a program in a child process.
 * The program can be interactive, as the curses library is closed before running the program.
 * @param[in] cmd shell command line to run.
 * @param[out] info_msg will contain a message about the command execution.
 * @param[in] wait_for_key if true, wait until user presses the enter key before returning from the function;
 *                         if false return from the function if cmd finished running.
 * @return true if cmd had an exit status of 0.
 */
bool run_program(const std::string& cmd, std::string& info_msg, bool wait_for_key);

/**
 * run a command in the background, detached from the current process.
 * @param[in] cmd shell command line to execute in background.
 * @param[out] info_msg will contain a message about the command execution.
 * @return process ID of the background program; -1 upon error.
 */
int run_command_background(std::string cmd, std::string& info_msg);
