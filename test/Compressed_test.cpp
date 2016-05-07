/*!
 \file test/Compressed_test.cpp
 \brief WatSON Compressed Test Suite

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

const uint8_t test_compressed_container[] = {
        'Z',
        0x25, //< size.
        0x25, 0x30, 0x43, 0x25, 0x73, 0x09,
        0x54, 0x65, 0x73, 0x74, 0x69, 0x6E, 0x67, 0x73,
        0x0A, 0x0D, 0x09, 0x40, 0x2E, 0x73, 0x07, 0x54,
        0x68, 0x69, 0x72, 0x64, 0x30, 0x31, 0x3F, 0x69,
        0x06, 0xF0, 0xF0, 0xF0, 0xF1,
}; // 

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

void test_Compressed_default_ctr()
{
    const watson::Compressed obj;

    TEST_ASSERT(watson::ngrdnt_type(obj->type_marker()) == watson::Ngrdnt_type::k_null);
    TEST_ASSERT(obj->size() == 1);
}

void test_Compressed_copy_ctr()
{
    watson::Compressed obj(watson::Ngrdnt::temp(test_compressed_container));
    verify_object(watson::Container(*obj));

    watson::Compressed b(obj);
    verify_object(watson::Container(*obj));
    verify_object(watson::Container(*b));
    TEST_ASSERT(&(*obj) != &(*b));

    watson::Compressed c = b;
    verify_object(watson::Container(*b));
    verify_object(watson::Container(*c));
    TEST_ASSERT(&(*b) != &(*c));
}

void test_Compressed_ingredient_ctr()
{
    watson::Compressed obj(watson::Ngrdnt::temp(test_compressed_container));
    verify_object(watson::Container(*obj));
}

void test_Compressed_move_semantics()
{
    watson::Compressed obj(watson::Ngrdnt::temp(test_compressed_container));
    watson::Compressed b;
    const uint8_t* const expected = obj->data();
    TEST_ASSERT(obj->data() != b->data());

    b = std::move(obj);
    TEST_ASSERT((*obj) == nullptr);
    TEST_ASSERT(b->data() == expected);

    watson::Compressed c(std::move(b));
    TEST_ASSERT((*b) == nullptr);
    TEST_ASSERT(c->data() == expected);

    verify_object(watson::Container(*c));
}

void test_Compressed_adoption_ctr()
{
    watson::Compressed obj(watson::Ngrdnt::clone(test_container));
    watson::Ngrdnt::Ptr i(watson::new_ngrdnt(obj));

    for (int h = 0; h < i->size(); ++h)
    {
        std::ostringstream oss;
        oss << "h=" << h << " result=" << ((int)i->data()[h]);
        oss << " expected=" << ((int)test_compressed_container[h]);
        TEST_ASSERT_MSG(oss.str(), i->data()[h] == test_compressed_container[h]);
    }
}

void test_Compressed_read_write()
{
    watson::Compressed obj(watson::Ngrdnt::clone(test_container));
    const watson::Ngrdnt::Ptr i(watson::new_ngrdnt(obj));
    watson::Compressed b(i);

    verify_object(watson::Container(*obj));
    verify_object(watson::Container(*b));
    // TODO Actually test reading and writing to an IO stream.
}

const Test_entry tests[] = {
    PREPARE_TEST(test_Compressed_default_ctr),
    PREPARE_TEST(test_Compressed_copy_ctr),
    PREPARE_TEST(test_Compressed_ingredient_ctr),
    PREPARE_TEST(test_Compressed_move_semantics),
    PREPARE_TEST(test_Compressed_adoption_ctr),
    PREPARE_TEST(test_Compressed_read_write),
    {0, ""}
};

int main(int argc, char** argv)
{
    return Test_util::runner("watson::Compressed", tests);
}

