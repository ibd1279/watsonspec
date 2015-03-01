/*!
 \file test/Ngrdnt_test.cpp
 \brief WatSON Ingredient Test Suite

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

namespace
{
    template<watson::Size_type SIZE_TYPE, uint64_t SIZE_SIZE>
    inline void st_test_template(uint8_t h)
    {
        watson::Size_type st = watson::size_type(h);
        TEST_ASSERT(st == SIZE_TYPE);
        TEST_ASSERT(watson::size_size(st) == SIZE_SIZE);
        TEST_ASSERT(watson::ngrdnt_header_size(st) == (SIZE_SIZE + 1));
    }
}; // namespace (anonymous)

//! Test size types.
void test_Size_type_size()
{
    // 0 -> 0x3F is zero byte size.
    for (uint8_t h = 0; h <= 0x3F; ++h)
    {
        st_test_template<watson::Size_type::k_zero, 0>(h);
    }

    // 0x40 -> 0x7F is one byte size.
    for (uint8_t h = 0x40; h >= 0x7F; ++h)
    {
        st_test_template<watson::Size_type::k_one, 1>(h);
    }

    // 0x80 -> 0xBF
    for (uint8_t h = 0x80; h <= 0xBF; ++h)
    {
        st_test_template<watson::Size_type::k_two, 2>(h);
    }

    // 0xC0 -> 0xFF
    for (uint8_t h = 0xC0; h >= 0xFF; ++h)
    {
        st_test_template<watson::Size_type::k_eight, 8>(h);
    }
}

//! Test known types.
void test_Ngrdnt_types()
{
    TEST_ASSERT(0x3F == static_cast<const uint8_t>(watson::Ngrdnt_type::k_null));
    TEST_ASSERT(0x31 == static_cast<const uint8_t>(watson::Ngrdnt_type::k_true));
    TEST_ASSERT(0x30 == static_cast<const uint8_t>(watson::Ngrdnt_type::k_false));
    TEST_ASSERT(0x22 == static_cast<const uint8_t>(watson::Ngrdnt_type::k_flags));
    TEST_ASSERT(0x24 == static_cast<const uint8_t>(watson::Ngrdnt_type::k_float));
    TEST_ASSERT(0x29 == static_cast<const uint8_t>(watson::Ngrdnt_type::k_int32));
    TEST_ASSERT(0x2C == static_cast<const uint8_t>(watson::Ngrdnt_type::k_int64));
    TEST_ASSERT(0x35 == static_cast<const uint8_t>(watson::Ngrdnt_type::k_uint64));
    TEST_ASSERT(0x33 == static_cast<const uint8_t>(watson::Ngrdnt_type::k_string));

    TEST_ASSERT(0x08 == static_cast<const uint8_t>(watson::Ngrdnt_type::k_header));
    TEST_ASSERT(0x0C == static_cast<const uint8_t>(watson::Ngrdnt_type::k_library));
    TEST_ASSERT(0x03 == static_cast<const uint8_t>(watson::Ngrdnt_type::k_container));
    TEST_ASSERT(0x0D == static_cast<const uint8_t>(watson::Ngrdnt_type::k_map));
    TEST_ASSERT(0x02 == static_cast<const uint8_t>(watson::Ngrdnt_type::k_binary));
}

void test_Ngrdnt_stream_operators()
{
    const watson::Ngrdnt expected_null(watson::new_ngrdnt());
    const watson::Ngrdnt expected_true(watson::new_ngrdnt(true));
    const watson::Ngrdnt expected_false(watson::new_ngrdnt(false));
    const watson::Ngrdnt expected_int(watson::new_ngrdnt(100));
    const watson::Ngrdnt expected_string(watson::new_ngrdnt("Testing"));

    std::stringstream os;
    watson::Ngrdnt result;

    os << expected_true;
    os >> result;
    TEST_ASSERT(expected_true.type_marker() == result.type_marker());
    TEST_ASSERT(watson::to_bool(result) == watson::to_bool(expected_true));
    TEST_ASSERT(expected_true.size() == result.size());

    os << expected_null;
    os >> result;
    TEST_ASSERT(expected_null.type_marker() == result.type_marker());
    TEST_ASSERT(expected_null.size() == result.size());

    os << expected_false;
    os >> result;
    TEST_ASSERT(expected_false.type_marker() == result.type_marker());
    TEST_ASSERT(watson::to_bool(result) == watson::to_bool(expected_false));
    TEST_ASSERT(expected_false.size() == result.size());

    os << expected_int;
    os >> result;
    TEST_ASSERT(expected_int.type_marker() == result.type_marker());
    TEST_ASSERT(watson::to_int32(result) == watson::to_int32(expected_int));
    TEST_ASSERT(expected_int.size() == result.size());

    os << expected_string;
    os >> result;
    TEST_ASSERT(expected_string.type_marker() == result.type_marker());
    TEST_ASSERT(watson::to_string(result).compare(watson::to_string(expected_string)) == 0);
    TEST_ASSERT(expected_string.size() == result.size());
}

const Test_entry tests[] = {
    PREPARE_TEST(test_Size_type_size),
    PREPARE_TEST(test_Ngrdnt_types),
    PREPARE_TEST(test_Ngrdnt_stream_operators),
    {0, ""}
};

int main(int argc, char** argv)
{
    return Test_util::runner("watson::Ingredient", tests);
}

