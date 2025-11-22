#include <gtest/gtest.h>

#include <memory_resource>
#include "FixedBlockResource.h"
#include "ForwardList.h"

// ------------------------------------------------------------
// ForwardList push/pop operations
// ------------------------------------------------------------
TEST(ForwardListTest, PushBackAddsElements) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    std::vector<int> result;
    for (int v : list) result.push_back(v);

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 3);
}

TEST(ForwardListTest, PushFrontWorks) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_front(10);
    list.push_front(20);

    auto it = list.begin();
    ASSERT_NE(it, list.end());
    EXPECT_EQ(*it, 20);

    ++it;
    ASSERT_NE(it, list.end());
    EXPECT_EQ(*it, 10);
}

TEST(ForwardListTest, PopFrontRemovesFirstElement) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_back(5);
    list.push_back(6);

    list.pop_front();

    auto it = list.begin();
    ASSERT_NE(it, list.end());
    EXPECT_EQ(*it, 6);
}

TEST(ForwardListTest, PopBackRemovesLastElement) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    list.pop_back();

    std::vector<int> result;
    for (int v : list) result.push_back(v);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
}

// ------------------------------------------------------------
// Complex type test
// ------------------------------------------------------------
TEST(ForwardListComplexTest, WorksWithStruct) {
    struct Obj {
        std::string s;
        int n;
    };

    FixedBlockResource mem(2048);
    ForwardList<Obj> list(&mem);

    list.push_back({"A", 10});
    list.push_back({"B", 20});

    auto it = list.begin();
    ASSERT_NE(it, list.end());
    EXPECT_EQ(it->s, "A");
    EXPECT_EQ(it->n, 10);

    ++it;
    ASSERT_NE(it, list.end());
    EXPECT_EQ(it->s, "B");
    EXPECT_EQ(it->n, 20);
}

// ------------------------------------------------------------
// Memory reuse test
// ------------------------------------------------------------
TEST(MemoryResourceTest, MemoryReuseWorks) {
    FixedBlockResource mem(4096);

    size_t before_blocks = mem.used_blocks_count();

    {
        ForwardList<int> temp(&mem);
        temp.push_back(1);
        temp.push_back(2);
    }

    size_t after_blocks = mem.used_blocks_count();

    EXPECT_GT(after_blocks, before_blocks);  // blocks allocated

    ForwardList<int> again(&mem);
    again.push_back(123);

    EXPECT_EQ(mem.used_blocks_count(), after_blocks);  // reuse confirmed
}
