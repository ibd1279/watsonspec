/*!
 \file test/Recipe_test.cpp
 \brief WatSON Recipe Test Suite

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
    watson::Ngrdnt::Ptr produce()
    {
        watson::Library l;
        l.mutable_children().push_back("first");
        l.mutable_children().push_back("second");
        l.mutable_children().push_back("third");
        l.mutable_children().push_back("third-first");

        watson::Map cm;
        cm.mutable_children()[3] = watson::new_ngrdnt("First Child of the Third Element");

        watson::Map m;
        m.mutable_children()[0] = watson::new_ngrdnt("First Element");
        m.mutable_children()[1] = watson::new_ngrdnt("Second Element");
        m.mutable_children()[2] = watson::new_ngrdnt(cm);

        watson::Container c;
        c.mutable_children().push_back(watson::new_ngrdnt(l));
        c.mutable_children().push_back(watson::new_ngrdnt(m));

        return new_ngrdnt(c);
    }

    bool verify(const watson::Recipe& a)
    {
        return true;
    }
};

void test_xlate_string_to_int()
{
    watson::Recipe r(std::move(produce()));

    // Test the first element.
    auto keys = watson::xlate(r.glossary(), std::list<std::string>{"first"});
    TEST_ASSERT(keys.size() == 1);
    TEST_ASSERT(keys.front() == 0);

    // Test the second element.
    keys = watson::xlate(r.glossary(), std::list<std::string>{"second"});
    TEST_ASSERT(keys.size() == 1);
    TEST_ASSERT(keys.front() == 1);

    // Test multiple elements.
    keys = watson::xlate(r.glossary(), std::list<std::string>{"third", "second", "third-first"});
    TEST_ASSERT(keys.size() == 3);
    TEST_ASSERT(keys.front() == 2);
    TEST_ASSERT(keys.back() == 3);

    // Test an unknown element.
    keys = watson::xlate(r.glossary(), std::list<std::string>{"unknown"});
    TEST_ASSERT(keys.size() == 1);
    TEST_ASSERT(keys.front() == 0);

}

void test_xlate_int_to_string()
{
    watson::Recipe r(std::move(produce()));

    auto keys = watson::xlate(r.glossary(), std::list<uint32_t>{0});
    TEST_ASSERT(keys.size() == 1);
    TEST_ASSERT(keys.front().compare("first") == 0);

    keys = watson::xlate(r.glossary(), std::list<uint32_t>{1});
    TEST_ASSERT(keys.size() == 1);
    TEST_ASSERT(keys.front().compare("second") == 0);

    keys = watson::xlate(r.glossary(), std::list<uint32_t>{2,1,3});
    TEST_ASSERT(keys.size() == 3);
    TEST_ASSERT(keys.front().compare("third") == 0);
    TEST_ASSERT(keys.back().compare("third-first") == 0);

    keys = watson::xlate(r.glossary(), std::list<uint32_t>{99});
    TEST_ASSERT(keys.size() == 1);
    TEST_ASSERT(keys.front().compare("") == 0);
}

void test_Recipe_default_ctr()
{
    watson::Recipe r;

    TEST_ASSERT(r.container().size() == 0);
    TEST_ASSERT(r.glossary().names.size() == 0);
    TEST_ASSERT(r.glossary().index.size() == 0);
}

void test_Recipe_copy_ctr()
{
}

const Test_entry tests[] = {
    PREPARE_TEST(test_xlate_string_to_int),
    PREPARE_TEST(test_xlate_int_to_string),
    PREPARE_TEST(test_Recipe_default_ctr),
    {0, ""}
};

int main(int argc, char** argv)
{
    return Test_util::runner("watson::Recipe", tests);
}

