/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
* vi: set shiftwidth=4 tabstop=8:
* :indentSize=4:tabSize=8:
*/
#pragma once
#include <string>
/**
 * maximize the terminal/console window.
 * @param[out] if the function returns false an error message is assigned to this output parameter.
 * @return true upon success; false upon failure.
 */
bool maximize_window(std::string& error_msg);
