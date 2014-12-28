/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "temporary_file.h"
#include <cstdlib>
#include <stdexcept>
#include <atomic>
#include <vector>

#include <Windows.h>

namespace {
    std::atomic_ullong cnt_;
}

TemporaryFile::TemporaryFile()
{
    wchar_t dirname[MAX_PATH];
    DWORD ret = GetTempPath(sizeof(dirname), dirname);
    if (ret == 0) {
	throw std::runtime_error("could not get temp path");
    }
    wchar_t fn[MAX_PATH];
    ret = GetTempFileName(dirname, L"few", 0, fn);
    if (ret == 0) {
	throw std::runtime_error("could not get temp filename");
    }
    filename_ = fn;
}

TemporaryFile::~TemporaryFile()
{
    if (! filename_.empty()) {
	_wunlink(filename_.c_str());
    }
}

const std::wstring&
TemporaryFile::filename()
{
    return filename_;
}
