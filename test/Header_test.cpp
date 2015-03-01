/*!
 \file test/Header_test.cpp
 \brief WatSON Map Test Suite

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

const uint8_t test_header[] = {
        'H', //< Type
        0x1E,
        'a', 'b', 'c', 0x00, //< key 0
        '?', //< value null
        'd', 'e', 'f', 0x00, //< key 1
        '1', //< value true
        'g', 'h', 'i', 0x00, //< key 2
        '0', //< value false
        'j', 'k', 'l', 0x00, //< key 3
        's', 0x09,
        84, 101, 115, 116, 105, 110, 103 //< data: Testing
};

const std::string expected_string("Testing");
const std::string key_one("abc");
const std::string key_two("def");
const std::string key_three("ghi");
const std::string key_four("jkl");

namespace
{
    void verify_object(const watson::Header& obj)
    {
        TEST_ASSERT_MSG(std::to_string(obj.size()),
                obj.size() == 4);
        TEST_ASSERT_MSG("Expected Null",
                watson::ngrdnt_type(obj[key_one].type_marker()) == watson::Ngrdnt_type::k_null);
        TEST_ASSERT_MSG("Expected True",
                watson::ngrdnt_type(obj[key_two].type_marker()) == watson::Ngrdnt_type::k_true);
        TEST_ASSERT_MSG("Expected False",
                watson::ngrdnt_type(obj[key_three].type_marker()) == watson::Ngrdnt_type::k_false);

        watson::Ngrdnt str(obj[key_four]);
        TEST_ASSERT_MSG("Expected Testing.",
                watson::to_string(str).compare(expected_string) == 0);
    }
}; // namespace (anonymous)

void test_Header_default_ctr()
{
    watson::Header obj;

    TEST_ASSERT(obj.size() == 0);
}

void test_Header_copy_ctr()
{
    watson::Header obj(watson::Ngrdnt::temp(test_header));

    watson::Header ctor(obj);
    TEST_ASSERT_MSG("Expected different children",
            &(obj.children()) != &(ctor.children()));
    verify_object(ctor);
    verify_object(obj);

    watson::Header assign = ctor;
    TEST_ASSERT_MSG("Expected different children",
            &(assign.children()) != &(ctor.children()));
    verify_object(assign);
    verify_object(ctor);
}

void test_Header_ingredient_ctr()
{
    watson::Header obj(watson::Ngrdnt::temp(test_header));
    verify_object(obj);
}

void test_Header_move_semantics()
{
    watson::Header obj(watson::Ngrdnt::temp(test_header));
    watson::Header assign;
    verify_object(obj);
    TEST_ASSERT(assign.size() == 0);

    assign = std::move(obj);
    verify_object(assign);
    TEST_ASSERT(obj.size() == 0);

    watson::Header ctor(std::move(assign));
    verify_object(ctor);
    TEST_ASSERT(assign.size() == 0);
}

void test_Header_adoption_ctr()
{
    watson::Header::Children c;
    c[key_one] = watson::new_ngrdnt();
    c[key_two] = watson::new_ngrdnt(true);
    c[key_three] = watson::new_ngrdnt(false);
    c[key_four] = watson::new_ngrdnt("Testing");
    watson::Header obj(std::move(c));
    watson::Ngrdnt i(watson::new_ngrdnt(obj));

    for (int h = 0; h < i.size(); ++h)
    {
        std::ostringstream oss;
        oss << "h=" << h << " result=" << ((int)i.data()[h]);
        oss << " expected=" << ((int)test_header[h]);
        TEST_ASSERT_MSG(oss.str(), i.data()[h] == test_header[h]);
    }
}

const Test_entry tests[] = {
    PREPARE_TEST(test_Header_default_ctr),
    PREPARE_TEST(test_Header_copy_ctr),
    PREPARE_TEST(test_Header_ingredient_ctr),
    PREPARE_TEST(test_Header_move_semantics),
    PREPARE_TEST(test_Header_adoption_ctr),
    {0, ""}
};

int main(int argc, char** argv)
{
    return Test_util::runner("watson::Header", tests);
}

