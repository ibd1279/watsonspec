/*!
 \file test/Map_test.cpp
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

const uint8_t test_map[] = {
        'M', //< Type
        0x1E,
        0x00, 0x00, 0x00, 0x00, //< key 0
        '?', //< value null
        0x01, 0x00, 0x00, 0x00, //< key 1
        '1', //< value true
        0x02, 0x00, 0x00, 0x00, //< key 2
        '0', //< value false
        0x03, 0x00, 0x00, 0x00, //< key 3
        's', 0x09,
        84, 101, 115, 116, 105, 110, 103 //< data: Testing
};

const std::string expected_string("Testing");

void test_Map_default_ctr()
{
    watson::Map m;

    TEST_ASSERT(m.children().size() == 0);
}

void test_Map_copy_ctr()
{
    watson::Map obj(watson::Ngrdnt::temp(test_map));

    watson::Map b(obj);
    TEST_ASSERT_MSG(std::to_string(b.size()), b.size() == 4);
    TEST_ASSERT(watson::ngrdnt_type(b[0]->type_marker()) == watson::Ngrdnt_type::k_null);
    TEST_ASSERT(watson::ngrdnt_type(b[1]->type_marker()) == watson::Ngrdnt_type::k_true);
    TEST_ASSERT(watson::ngrdnt_type(b[2]->type_marker()) == watson::Ngrdnt_type::k_false);
    TEST_ASSERT(watson::ngrdnt_type(b[3]->type_marker()) == watson::Ngrdnt_type::k_string);
    TEST_ASSERT(watson::to_string(b[3]).compare(expected_string) == 0);

    watson::Map c = b;
    TEST_ASSERT(c.size() == 4);
    TEST_ASSERT(watson::ngrdnt_type(c[0]->type_marker()) == watson::Ngrdnt_type::k_null);
    TEST_ASSERT(watson::ngrdnt_type(c[1]->type_marker()) == watson::Ngrdnt_type::k_true);
    TEST_ASSERT(watson::ngrdnt_type(c[2]->type_marker()) == watson::Ngrdnt_type::k_false);
    TEST_ASSERT(watson::ngrdnt_type(c[3]->type_marker()) == watson::Ngrdnt_type::k_string);
    TEST_ASSERT(watson::to_string(c[3]).compare(expected_string) == 0);

    // verify that the Ngrdnt objects inside the map don't point
    // to the original test_map. This is to ensure the copy constructor
    // doesn't shallow copy temp Ngrdnts.
    const uint8_t* begin = test_map;
    const uint8_t* end = test_map + sizeof(test_map);
    TEST_ASSERT_MSG("Copy constructor resulted in a shallow copy.", !(obj[0]->data() > begin && obj[0]->data() < end));
}

void test_Map_ingredient_ctr()
{
    watson::Map m(watson::Ngrdnt::temp(test_map));

    TEST_ASSERT(m.size() == 4);
    TEST_ASSERT(watson::ngrdnt_type(m[0]->type_marker()) == watson::Ngrdnt_type::k_null);
    TEST_ASSERT(watson::ngrdnt_type(m[1]->type_marker()) == watson::Ngrdnt_type::k_true);
    TEST_ASSERT(watson::ngrdnt_type(m[2]->type_marker()) == watson::Ngrdnt_type::k_false);
    TEST_ASSERT(watson::ngrdnt_type(m[3]->type_marker()) == watson::Ngrdnt_type::k_string);
    watson::Ngrdnt::Ptr s1(watson::Ngrdnt::clone(m[3]));
    TEST_ASSERT(watson::to_string(s1).compare(expected_string) == 0);
}

void test_Map_move_semantics()
{
    watson::Map m1(watson::Ngrdnt::temp(test_map));
    watson::Map m2;

    TEST_ASSERT(m1.size() == 4);
    TEST_ASSERT(m2.size() == 0);

    m2 = std::move(m1);
    TEST_ASSERT(m1.size() == 0);
    TEST_ASSERT(m2.size() == 4);

    watson::Map m3(std::move(m2));
    TEST_ASSERT(m2.size() == 0);
    TEST_ASSERT(m3.size() == 4);
    TEST_ASSERT(watson::ngrdnt_type(m3[0]->type_marker()) == watson::Ngrdnt_type::k_null);
    TEST_ASSERT(watson::ngrdnt_type(m3[1]->type_marker()) == watson::Ngrdnt_type::k_true);
    TEST_ASSERT(watson::ngrdnt_type(m3[2]->type_marker()) == watson::Ngrdnt_type::k_false);
    TEST_ASSERT(watson::ngrdnt_type(m3[3]->type_marker()) == watson::Ngrdnt_type::k_string);
    watson::Ngrdnt::Ptr s1(watson::Ngrdnt::clone(m3[3]));
    TEST_ASSERT(watson::to_string(s1).compare(expected_string) == 0);
}

void test_Map_adoption_ctr()
{
    watson::Map::Children c;
    c[0] = watson::new_ngrdnt();
    c[1] = watson::new_ngrdnt(true);
    c[2] = watson::new_ngrdnt(false);
    c[3] = watson::new_ngrdnt("Testing");
    watson::Map m(std::move(c));
    watson::Ngrdnt::Ptr i(watson::new_ngrdnt(m));

    for (int h = 0; h < i->size(); ++h)
    {
        std::ostringstream oss;
        oss << "h=" << h << " result=" << ((int)i->data()[h]);
        oss << " expected=" << ((int)test_map[h]);
        TEST_ASSERT_MSG(oss.str(), i->data()[h] == test_map[h]);
    }
}

const Test_entry tests[] = {
    PREPARE_TEST(test_Map_default_ctr),
    PREPARE_TEST(test_Map_copy_ctr),
    PREPARE_TEST(test_Map_ingredient_ctr),
    PREPARE_TEST(test_Map_move_semantics),
    PREPARE_TEST(test_Map_adoption_ctr),
    {0, ""}
};

int main(int argc, char** argv)
{
    return Test_util::runner("watson::Map", tests);
}

