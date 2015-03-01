/*!
 \file test/Library_test.cpp
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

const uint8_t test_library[] = {
        'L', //< Type
        0x1C,
        's', 0x09,
        84, 101, 115, 116, 105, 110, 103, //< data: Testing
        's', 0x0A,
        'T', 'e', 's', 't', 'i', 'n', 'g', '.', //< data: Testing.
        's', 0x07,
        'T', 'h', 'i', 'r', 'd' //< data: Third
};

const std::string first_string("Testing");
const std::string second_string("Testing.");
const std::string third_string("Third");

void test_Library_default_ctr()
{
    watson::Library l;

    TEST_ASSERT(l.size() == 0);
}

void test_Library_copy_ctr()
{
    watson::Library l(watson::Ngrdnt::temp(test_library));

    watson::Library l_copy(l);
    TEST_ASSERT_MSG(std::to_string(l_copy.size()), l_copy.size() == 3);
    TEST_ASSERT_MSG(l_copy[0], l_copy[0].compare(first_string) == 0);
    TEST_ASSERT_MSG(l_copy[1], l_copy[1].compare(second_string) == 0);
    TEST_ASSERT_MSG(l_copy[2], l_copy[2].compare(third_string) == 0);

    watson::Library c = l_copy;
    TEST_ASSERT_MSG(std::to_string(l_copy.size()), l_copy.size() == 3);
    TEST_ASSERT_MSG(c[0], c[0].compare(first_string) == 0);
    TEST_ASSERT_MSG(c[1], c[1].compare(second_string) == 0);
    TEST_ASSERT_MSG(c[2], c[2].compare(third_string) == 0);
}

void test_Library_ingredient_ctr()
{
    watson::Library l(watson::Ngrdnt::temp(test_library));

    TEST_ASSERT_MSG(std::to_string(l.size()), l.size() == 3);
    TEST_ASSERT_MSG(l[0], l[0].compare(first_string) == 0);
    TEST_ASSERT_MSG(l[1], l[1].compare(second_string) == 0);
    TEST_ASSERT_MSG(l[2], l[2].compare(third_string) == 0);
}

void test_Library_move_semantics()
{
    watson::Library l1(watson::Ngrdnt::temp(test_library));
    watson::Library l2;

    TEST_ASSERT(l1.size() == 3);
    TEST_ASSERT(l2.size() == 0);

    l2 = std::move(l1);
    TEST_ASSERT(l1.size() == 0);
    TEST_ASSERT(l2.size() == 3);

    watson::Library l3(std::move(l2));
    TEST_ASSERT(l2.size() == 0);
    TEST_ASSERT(l3.size() == 3);

    TEST_ASSERT_MSG(l3[0], l3[0].compare(first_string) == 0);
    TEST_ASSERT_MSG(l3[1], l3[1].compare(second_string) == 0);
    TEST_ASSERT_MSG(l3[2], l3[2].compare(third_string) == 0);
}

void test_Library_adoption_ctr()
{
    watson::Library::Children c(3);
    c[0] = "Testing";
    c[1] = second_string;
    c[2] = third_string;
    watson::Library l(std::move(c));
    watson::Ngrdnt i(watson::new_ngrdnt(l));

    for (int h = 0; h < i.size(); ++h)
    {
        std::ostringstream oss;
        oss << "h=" << h << " result=" << ((int)i.data()[h]);
        oss << " expected=" << ((int)test_library[h]);
        TEST_ASSERT_MSG(oss.str(), i.data()[h] == test_library[h]);
    }
}

const Test_entry tests[] = {
    PREPARE_TEST(test_Library_default_ctr),
    PREPARE_TEST(test_Library_copy_ctr),
    PREPARE_TEST(test_Library_ingredient_ctr),
    PREPARE_TEST(test_Library_move_semantics),
    PREPARE_TEST(test_Library_adoption_ctr),
    {0, ""}
};

int main(int argc, char** argv)
{
    return Test_util::runner("watson::Library", tests);
}

