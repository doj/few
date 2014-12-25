/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#include "info.h"
#include <mutex>
namespace {
    std::string i;
    std::mutex l;
}
std::string info()
{
    std::string inf;
    {
	std::lock_guard<std::mutex> _(l);
	inf = i;
    }
    return inf;
}
void info(const std::string& inf)
{
    std::lock_guard<std::mutex> _(l);
    i = inf;
}
