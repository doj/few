/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
/**
 * open the console/tty device as STDIN (file descriptor 0).
 * @return true upon success.
 */
bool open_tty_as_stdin();
