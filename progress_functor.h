/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
#pragma once
#include <iostream>

class ProgressFunctor
{
public:
    virtual ~ProgressFunctor() {}

    /**
     * record the progress of an action.
     * @param num a number.
     * @param perc a percentage value, should be between [0..100].
     */
    virtual void progress(unsigned num, unsigned perc) = 0;
};

class OStreamProgressFunctor : public ProgressFunctor
{
    std::ostream& os_;
    const std::string desc_;
public:
    OStreamProgressFunctor(std::ostream& os, std::string desc) : os_(os), desc_(desc) {}
    ~OStreamProgressFunctor();
    virtual void progress(unsigned num, unsigned perc);
};

class CursesProgressFunctor : public ProgressFunctor
{
    unsigned y_, x_, attr_;
    const std::string desc_;
    /// maximum length of a string created in progress().
    unsigned max_len_;
public:
    CursesProgressFunctor(unsigned y, unsigned x, unsigned attr, std::string desc) :
	y_(y), x_(x), attr_(attr), desc_(desc), max_len_(0)
    {}
    ~CursesProgressFunctor();
    virtual void progress(unsigned num, unsigned perc);
};
