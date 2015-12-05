/*!
 \file test/Bytes_test.cpp
 \brief WatSON Bytes Test Suite

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

const uint8_t test_bytes[] = {
        0x42, //< Type
        0x20, //< Size
        0x01, 0x00, 0x00, 0x00, //< Subtype
        6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 //< data.
};

void test_Bytes_default_ctr()
{
    watson::Bytes b;

    TEST_ASSERT(b.marshal_hint() == 0);
    TEST_ASSERT(b.size() == 0);
    TEST_ASSERT(b.data() != nullptr);
}

void test_Bytes_copy_ctr()
{
    watson::Bytes b(watson::Ngrdnt::temp(test_bytes));
    uint32_t expected_marshal_hint = 1;
    uint64_t expected_size = 26;

    watson::Bytes b_copy(b);
    TEST_ASSERT(b_copy.marshal_hint() == expected_marshal_hint);
    TEST_ASSERT(b_copy.size() == expected_size);
    TEST_ASSERT(b_copy.data() != b.data());
    for (int h = 6; h < 32; ++h)
    {
        TEST_ASSERT(test_bytes[h] == b_copy.data()[h - 6]);
    }

    watson::Bytes c;

    c = b_copy;
    TEST_ASSERT(c.marshal_hint() == expected_marshal_hint);
    TEST_ASSERT(c.size() == expected_size);
    TEST_ASSERT(c.data() != b_copy.data());
    for (int h = 6; h < 32; ++h)
    {
        TEST_ASSERT(test_bytes[h] == c.data()[h - 6]);
    }
}

void test_Bytes_ingredient_ctr()
{
    watson::Bytes b(watson::Ngrdnt::temp(test_bytes));
    uint32_t expected_marshal_hint = 1;
    uint64_t expected_size = 26;

    TEST_ASSERT(b.marshal_hint() == expected_marshal_hint);
    TEST_ASSERT(b.size() == expected_size);
    for (int h = 6; h < 32; ++h)
    {
        TEST_ASSERT(test_bytes[h] == b.data()[h - 6]);
    }
}

void test_Bytes_temp_factory()
{
    watson::Bytes b;

    uint32_t expected_marshal_hint = 0xFFFF00FF;
    uint64_t expected_size = 26;
    uint32_t marshal_hint = 0xFFFF00FF;

    // Technically, this tests copy as well.
    b = watson::Bytes::temp(expected_size, &marshal_hint, test_bytes + 6);

    TEST_ASSERT(b.marshal_hint() == expected_marshal_hint);
    TEST_ASSERT(b.size() == expected_size);
    for (int h = 6; h < 32; ++h)
    {
        TEST_ASSERT(test_bytes[h] == b.data()[h - 6]);
    }
};

void test_Bytes_move_semantics()
{
    uint32_t expected_marshal_hint = 1;
    uint64_t expected_size = 26;

    watson::Bytes b1(watson::Ngrdnt::temp(test_bytes));
    watson::Bytes b2;

    TEST_ASSERT(b1.size() == expected_size);
    TEST_ASSERT(b1.marshal_hint() == expected_marshal_hint);
    TEST_ASSERT(b2.size() == 0);
    TEST_ASSERT(b2.marshal_hint() == 0);

    b2 = std::move(b1);
    TEST_ASSERT(b2.size() == expected_size);
    TEST_ASSERT(b2.marshal_hint() == expected_marshal_hint);
    for (int h = 6; h < 32; ++h)
    {
        TEST_ASSERT(test_bytes[h] == b2.data()[h - 6]);
    }

    watson::Bytes b3(std::move(b2));
    TEST_ASSERT(b3.size() == expected_size);
    TEST_ASSERT(b3.marshal_hint() == expected_marshal_hint);
    for (int h = 6; h < 32; ++h)
    {
        TEST_ASSERT(test_bytes[h] == b3.data()[h - 6]);
    }
}

void test_Bytes_adoption_ctr()
{
    uint64_t expected_size = 26;

    std::unique_ptr<uint8_t[]> input(new uint8_t[expected_size + 4]);
    memcpy(input.get(), test_bytes + 2, expected_size + 4);

    watson::Bytes b(std::move(input),
            expected_size + 4);

    watson::Ngrdnt::Ptr i(watson::new_ngrdnt(b));
    for (int h = 0; h < i->size(); ++h)
    {
        std::ostringstream oss;
        oss << "h=" << h << " result=" << ((int)i->data()[h]);
        oss << " expected=" << ((int)test_bytes[h]);
        TEST_ASSERT_MSG(oss.str(), i->data()[h] == test_bytes[h]);
    }
}

const Test_entry tests[] = {
    PREPARE_TEST(test_Bytes_default_ctr),
    PREPARE_TEST(test_Bytes_copy_ctr),
    PREPARE_TEST(test_Bytes_ingredient_ctr),
    PREPARE_TEST(test_Bytes_temp_factory),
    PREPARE_TEST(test_Bytes_move_semantics),
    PREPARE_TEST(test_Bytes_adoption_ctr),
    {0, ""}
};

int main(int argc, char** argv)
{
    return Test_util::runner("watson::Bytes", tests);
}

