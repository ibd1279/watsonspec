/*!
 \file test/Container_test.cpp
 \brief WatSON Library Test Suite

 Copyright (c) 2015, Jason Watson
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 * Neither the name of the LogJammin nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "testhelper.h"
#include "watson.h"

const uint8_t test_container[] = {
        'C',
        0x25,
        's', 0x09,
        84, 101, 115, 116, 105, 110, 103, //< data: Testing
        's', 0x0A,
        'T', 'e', 's', 't', 'i', 'n', 'g', '.', //< data: Testing.
        's', 0x07,
        'T', 'h', 'i', 'r', 'd', //< data: Third
        '0', //< data: false
        '1', //< data: true
        '?', //< data: null
        'i', 0x06,
        0xF0, 0xF0, 0xF0, 0xF1, //< data: -235867920
};

const std::string first_string("Testing");
const std::string second_string("Testing.");
const std::string third_string("Third");
const bool expected_false(false);
const bool expected_true(true);
const int32_t expected_int(0xF1F0F0F0);

namespace
{
    void verify_object(const watson::Container& obj)
    {
        TEST_ASSERT_MSG(std::to_string(obj.size()), obj.size() == 7);
        TEST_ASSERT_MSG(watson::to_string(obj[0]),
                watson::to_string(obj[0]).compare(first_string) == 0);

        TEST_ASSERT_MSG(watson::to_string(obj[1]),
                watson::to_string(obj[1]).compare(second_string) == 0);
        TEST_ASSERT_MSG(watson::to_string(obj[2]),
                watson::to_string(obj[2]).compare(third_string) == 0);
        TEST_ASSERT_MSG("False",
                watson::to_bool(obj[3]) == expected_false);
        TEST_ASSERT_MSG("True",
                watson::to_bool(obj[4]) == expected_true);
        TEST_ASSERT_MSG("Null",
                watson::is_null(obj[5]));
        TEST_ASSERT_MSG(watson::to_string(obj[6]),
                watson::to_int32(obj[6]) == expected_int);
    }
}; // namespace (anonymous)

void test_Container_default_ctr()
{
    watson::Container obj;

    TEST_ASSERT(obj.size() == 0);
}

void test_Container_copy_ctr()
{
    watson::Container obj(watson::Ngrdnt::temp(test_container));

    watson::Container b(obj);
    verify_object(b);

    watson::Container c = b;
    verify_object(c);
}

void test_Container_ingredient_ctr()
{
    watson::Container obj(watson::Ngrdnt::temp(test_container));
    verify_object(obj);
}

void test_Container_move_semantics()
{
    watson::Container a(watson::Ngrdnt::temp(test_container));
    watson::Container b;

    TEST_ASSERT(a.size() == 7);
    TEST_ASSERT(b.size() == 0);

    b = std::move(a);
    TEST_ASSERT(a.size() == 0);
    TEST_ASSERT(b.size() == 7);

    watson::Container c(std::move(b));
    TEST_ASSERT(b.size() == 0);
    TEST_ASSERT(c.size() == 7);

    verify_object(c);
}

void test_Container_adoption_ctr()
{
    watson::Container::Children kids(7);
    kids[0] = watson::new_ngrdnt(first_string);
    kids[1] = watson::new_ngrdnt(second_string);
    kids[2] = watson::new_ngrdnt(third_string);
    kids[3] = watson::new_ngrdnt(expected_false);
    kids[4] = watson::new_ngrdnt(expected_true);
    kids[5] = watson::new_ngrdnt();
    kids[6] = watson::new_ngrdnt(expected_int);

    watson::Container obj(std::move(kids));
    watson::Ngrdnt i(watson::new_ngrdnt(obj));

    for (int h = 0; h < i.size(); ++h)
    {
        std::ostringstream oss;
        oss << "h=" << h << " result=" << ((int)i.data()[h]);
        oss << " expected=" << ((int)test_container[h]);
        TEST_ASSERT_MSG(oss.str(), i.data()[h] == test_container[h]);
    }
}

const Test_entry tests[] = {
    PREPARE_TEST(test_Container_default_ctr),
    PREPARE_TEST(test_Container_copy_ctr),
    PREPARE_TEST(test_Container_ingredient_ctr),
    PREPARE_TEST(test_Container_move_semantics),
    PREPARE_TEST(test_Container_adoption_ctr),
    {0, ""}
};

int main(int argc, char** argv)
{
    return Test_util::runner("watson::Container", tests);
}

