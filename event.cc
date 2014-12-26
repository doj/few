/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#include "event.h"
#include <mutex>
#include <deque>

namespace {
    std::mutex lock_;
    std::deque<event> q_;
}

bool eventPending()
{
    std::lock_guard<std::mutex> _(lock_);
    return ! q_.empty();
}

event eventGet()
{
    std::lock_guard<std::mutex> _(lock_);
    if (q_.empty()) {
	throw std::runtime_error("eventGet(): empty q_");
    }
    event e = q_.front();
    q_.pop_front();
    return e;
}

void eventAdd(const event& e)
{
    std::lock_guard<std::mutex> _(lock_);
    q_.push_back(e);
}
