/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "temporary_file.h"
#include <cstdlib>
#include <stdexcept>
#include <atomic>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
namespace {
    /**
     * check if an environment variable name exists and has a value.
     * if a value if found, prepend dirs with the value.
     * @param[in] name an environment variable name.
     * @param[in,out] a vector of directory names, which may be modified.
     */
    void check_env(const char* name, std::vector<std::string>& dirs)
    {
	const char* cc = getenv(name);
	if (!cc) return;

	std::string s = cc;
	if (s.empty()) return;

	dirs.insert(dirs.begin(), s);
    }
}
#else
#error unknown platform!
#endif

namespace {
    std::atomic_ullong cnt_;
}

TemporaryFile::TemporaryFile() :
    f_(nullptr)
{
    std::vector<std::string> dirs = { "/tmp", "/var/tmp" };
    check_env("TMP", dirs);
    check_env("TEMP", dirs);
    check_env("TMPDIR", dirs);
    check_env("TEMPDIR", dirs);

    std::string err = "TemporaryFile(): could not create a temp file, tried in directories: ";
    for(auto d : dirs) {
	if (can_create_file(d)) {
	    return;
	}
	err += d + " ";
    }

    throw std::runtime_error(err);
}

bool
TemporaryFile::can_create_file(const std::string& dir)
{
    std::string fn = dir + "/TemporaryFile." + std::to_string(getppid()) + "." + std::to_string(cnt_.fetch_add(1));
    int fd = creat(fn.c_str(), S_IRUSR | S_IWUSR);
    if (fd < 0) {
	return false;
    }
    if (::close(fd) < 0) {
	return false;
    }
    filename_ = fn;
    return true;
}

TemporaryFile::~TemporaryFile()
{
    if (! filename_.empty()) {
	unlink(filename_.c_str());
    }
}

const std::string&
TemporaryFile::filename()
{
    return filename_;
}

FILE*
TemporaryFile::file()
{
    if (f_) {
	return f_;
    }
    f_ = fopen(filename_.c_str(), "w+b");
    return f_;
}

bool
TemporaryFile::close()
{
    if (!f_) {
	return false;
    }
    const int ret = fclose(f_);
    f_ = nullptr;
    return ret == 0;
}
