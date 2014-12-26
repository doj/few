/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */

#pragma once
#include "regex_index.h"

struct event
{
    /// a new info string
    std::string info_;

    /// a new regex_index that has finished matching the file
    std::shared_ptr<regex_index> ri_;

    /// the index into the regex vector for ri_
    const unsigned ri_idx_;

    explicit event(const std::string& i) : info_(i), ri_idx_(0) {}
    explicit event(std::shared_ptr<regex_index> ri, const unsigned idx) : ri_(ri), ri_idx_(idx) {}

    bool operator== (const event& r) const
    {
	return info_ == r.info_ && ri_ == r.ri_ && ri_idx_ == r.ri_idx_;
    }
};

/// @return true if an event is scheduled for delivery
bool eventPending();
/**
 * @return a scheduled event. This function may only be called if eventPending() returned true.
 * @throws std::runtime_error if called with an empty event queue.
 */
event eventGet();
/// schedule an event
void eventAdd(const event& e);
