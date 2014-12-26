/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 *
 * Copyright (c) 2014 Dirk Jagdmann <doj@cubic.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you
 *        must not claim that you wrote the original software. If you use
 *        this software in a product, an acknowledgment in the product
 *        documentation would be appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and
 *        must not be misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *        distribution.
 */

#include "intersect.h"
#include "gtest/gtest.h"
#include <iterator>

namespace multiple_set_intersect_test1 {
    typedef std::vector<int> container_t;
    typedef std::pair<container_t::iterator, container_t::iterator> pair_t;

    TEST(multiple_set_intersect, without_any_set)
    {
	std::vector<pair_t> v;
	container_t out;
	ASSERT_EQ(0u, multiple_set_intersect(v.begin(), v.end(), std::back_insert_iterator<container_t>(out)));
    }

    TEST(multiple_set_intersect, with_a_single_empty_set)
    {
	container_t in;
	ASSERT_EQ(0u, in.size());

	std::vector<pair_t> v;
	v.push_back(std::make_pair(in.begin(), in.end()));
	ASSERT_EQ(1u, v.size());

	container_t out;
	ASSERT_EQ(0u, multiple_set_intersect(v.begin(), v.end(), std::back_insert_iterator<container_t>(out)));
	ASSERT_EQ(0u, out.size());
    }

    TEST(multiple_set_intersect, with_a_single_set)
    {
	container_t in = { 1, 2, 3 };
	ASSERT_EQ(3u, in.size());

	std::vector<pair_t> v;
	v.push_back(std::make_pair(in.begin(), in.end()));
	ASSERT_EQ(1u, v.size());

	container_t out;
	ASSERT_EQ(3u, multiple_set_intersect(v.begin(), v.end(), std::back_insert_iterator<container_t>(out)));
	ASSERT_EQ(3u, out.size());
	ASSERT_EQ(in, out);
    }

    TEST(multiple_set_intersect, with_multiple_sets_with_common_elements)
    {
	std::vector<pair_t> v;

	container_t a = { 1, 2, 3, 11, 555 };
	v.push_back(std::make_pair(a.begin(), a.end()));

	container_t b = { 3, 6, 10 };
	v.push_back(std::make_pair(b.begin(), b.end()));

	container_t c = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	v.push_back(std::make_pair(c.begin(), c.end()));

	container_t out;
	ASSERT_EQ(1u, multiple_set_intersect(v.begin(), v.end(), std::back_insert_iterator<container_t>(out)));
	ASSERT_EQ(1u, out.size());
	ASSERT_EQ(3, out[0]);
    }

    TEST(multiple_set_intersect, with_multiple_sets_with_identical_elements)
    {
	std::vector<pair_t> v;

	container_t a = { 1, 2, 3, 11, 555 };
	v.push_back(std::make_pair(a.begin(), a.end()));

	container_t b = { 1, 2, 3, 11, 555 };
	v.push_back(std::make_pair(b.begin(), b.end()));

	container_t c = { 1, 2, 3, 11, 555 };
	v.push_back(std::make_pair(c.begin(), c.end()));

	container_t out;
	ASSERT_EQ(5u, multiple_set_intersect(v.begin(), v.end(), std::back_insert_iterator<container_t>(out)));
	ASSERT_EQ(a, out);
    }

    TEST(multiple_set_intersect, with_multiple_sets_with_no_common_elements)
    {
	std::vector<pair_t> v;

	container_t a = { 1, 2, 3, 11, 555 };
	v.push_back(std::make_pair(a.begin(), a.end()));

	container_t b = { 1, 2, 3, 11, 555 };
	v.push_back(std::make_pair(b.begin(), b.end()));

	container_t c = { 1000, 1001, 1010, 1020, 1030 };
	v.push_back(std::make_pair(c.begin(), c.end()));

	container_t out;
	ASSERT_EQ(0u, multiple_set_intersect(v.begin(), v.end(), std::back_insert_iterator<container_t>(out)));
    }

}

#include <string>
#include <set>
#include <list>
namespace multiple_set_intersect_test2 {
    typedef std::set<std::string> container_t;
    typedef std::pair<container_t::iterator, container_t::iterator> pair_t;

    TEST(multiple_set_intersect, with_sets_of_strings)
    {
	std::list<pair_t> v;

	container_t a = { "1", "2", "3", "11", "555" };
	v.push_back(std::make_pair(a.begin(), a.end()));

	container_t b = { "3", "6", "10" };
	v.push_back(std::make_pair(b.begin(), b.end()));

	container_t c = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };
	v.push_back(std::make_pair(c.begin(), c.end()));

	container_t out;
	ASSERT_EQ(1u, multiple_set_intersect(v.begin(), v.end(), std::inserter(out, out.begin())));
	ASSERT_EQ(1u, out.size());
	ASSERT_EQ(std::string("3"), *(out.begin()));
    }
}
