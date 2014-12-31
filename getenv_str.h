/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <string>

/**
 * look up an environment variable.
 * @param[in] name environment variable name.
 * @param[out] value environment variable value if the function returns true.
 * @return true if the environment variable exists.
 */
bool getenv_str(const std::string& name, std::string& value);
