/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <string>
#include <stdint.h>

bool use_color();
void use_color(bool enable_color);

uint64_t color(int fg, int bg);
