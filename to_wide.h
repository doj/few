/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <string>
std::wstring to_wide(const std::string& s);
inline std::wstring to_wide(const std::wstring& s) { return s; }
std::string to_utf8(const std::wstring& s);
inline std::string to_utf8(const std::string& s) { return s; }
