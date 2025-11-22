#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <memory_resource>

#include "FixedBlockResource.h"
#include "ForwardList.h"

// ============================================================================
//  BASIC PUSH/POP TESTS
// ============================================================================

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

TEST(ForwardListTest, PopBackSingleElement) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_back(99);
    list.pop_back();

    EXPECT_EQ(list.begin(), list.end());
}

// ============================================================================
//  INSERT / ERASE TESTS
// ============================================================================

TEST(ForwardListTest, InsertAfterInMiddle) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_back(1);
    list.push_back(3);

    auto it = list.begin();
    list.insert_after(it, 2);

    std::vector<int> out;
    for (int v : list) out.push_back(v);

    ASSERT_EQ(out.size(), 3);
    EXPECT_EQ(out[0], 1);
    EXPECT_EQ(out[1], 2);
    EXPECT_EQ(out[2], 3);
}

TEST(ForwardListTest, InsertAfterBeforeBeginAddsFirst) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    auto it = list.before_begin();
    list.insert_after(it, 42);

    ASSERT_EQ(*list.begin(), 42);
}

TEST(ForwardListTest, EraseAfterWorks) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_back(10);
    list.push_back(20);
    list.push_back(30);

    auto it = list.begin();
    list.erase_after(it);  // remove 20

    std::vector<int> out;
    for (int v : list) out.push_back(v);

    ASSERT_EQ(out.size(), 2);
    EXPECT_EQ(out[0], 10);
    EXPECT_EQ(out[1], 30);
}

TEST(ForwardListTest, EraseAfterBeforeBeginRemovesFirst) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_back(1);
    list.push_back(2);

    list.erase_after(list.before_begin());

    ASSERT_EQ(*list.begin(), 2);
}

TEST(ForwardListTest, EraseAfterLastReturnsEnd) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_back(5);

    auto it = list.begin();
    auto res = list.erase_after(it);

    EXPECT_EQ(res, list.end());
}

// ============================================================================
//  CLEAR TEST
// ============================================================================

TEST(ForwardListTest, ClearEmptiesList) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_back(1);
    list.push_back(2);

    list.clear();

    EXPECT_EQ(list.begin(), list.end());
}

// ============================================================================
//  ITERATOR TESTS
// ============================================================================

TEST(ForwardListIteratorTest, IteratorTraversesCorrectly) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    for (int i = 1; i <= 5; i++)
        list.push_back(i);

    int expected = 1;
    for (auto it = list.begin(); it != list.end(); ++it)
        EXPECT_EQ(*it, expected++);

    EXPECT_EQ(expected, 6);
}

TEST(ForwardListIteratorTest, ConstIteratorWorks) {
    FixedBlockResource mem(2048);
    ForwardList<int> list(&mem);

    list.push_back(7);
    list.push_back(8);

    const auto& cref = list;
    auto it = cref.begin();

    EXPECT_EQ(*it, 7);
    ++it;
    EXPECT_EQ(*it, 8);
}

// ============================================================================
//  COMPLEX TYPE TEST
// ============================================================================

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

// ============================================================================
//  MEMORY RESOURCE TESTS
// ============================================================================

TEST(MemoryResourceTest, MemoryReuseWorks) {
    FixedBlockResource mem(4096);

    size_t before_blocks = mem.used_blocks_count();

    {
        ForwardList<int> temp(&mem);
        temp.push_back(1);
        temp.push_back(2);
        temp.push_back(3);
    }

    size_t after_blocks = mem.used_blocks_count();

    EXPECT_GT(after_blocks, before_blocks);

    ForwardList<int> again(&mem);
    again.push_back(123);

    EXPECT_EQ(mem.used_blocks_count(), after_blocks);  // reused
}

TEST(MemoryResourceTest, ReuseAfterManyAllocations) {
    FixedBlockResource mem(4096);

    for (int cycle = 0; cycle < 10; ++cycle) {
        ForwardList<int> list(&mem);

        for (int i = 0; i < 50; i++)
            list.push_back(i);

        list.clear();
    }

    EXPECT_LT(mem.used_blocks_count(), 100);  // должно быть мало блоков
}
