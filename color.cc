/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "color.h"
#include "curses_attr.h"
#include <map>

namespace {
    bool color_enabled = false;
}

bool use_color() { return color_enabled; }
void use_color(bool enable_color) { color_enabled = enable_color; }

namespace {
    unsigned color_pair_num = 0;
    std::map<std::pair<int,int>, uint64_t> color_map;
}

uint64_t color(int fg, int bg)
{
    if (! color_enabled) {
	return 0;
    }

    auto p = std::make_pair(fg,bg);
    auto it = color_map.find(p);
    if (it != color_map.end()) {
	return it->second;
    }

    ++color_pair_num;
    init_pair(color_pair_num, fg, bg);
    uint64_t cp = COLOR_PAIR(color_pair_num);
    color_map.insert(std::make_pair(p, cp));
    return cp;
}
