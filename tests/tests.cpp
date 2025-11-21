#include <gtest/gtest.h>
#include <array>
#include <memory_resource>

#include "pmr_map_resource.hpp"
#include "pmr_forward_list.hpp"

struct Data {
    std::string label;
    int value;
};

TEST(PMRList, PushAndIterateInts) {
    static std::array<std::byte, 4096> buffer;

    pmr_map_resource mem(buffer.data(), buffer.size());
    pmr_forward_list<int> lst(&mem);

    lst.push_front(3);
    lst.push_front(2);
    lst.push_front(1);

    int expected = 1;
    for (int v : lst)
        EXPECT_EQ(v, expected++);
}

TEST(PMRList, PushAndIterateStructs) {
    static std::array<std::byte, 4096> buffer;

    pmr_map_resource mem(buffer.data(), buffer.size());
    pmr_forward_list<Data> lst(&mem);

    lst.emplace_front("C", 3);
    lst.emplace_front("B", 2);
    lst.emplace_front("A", 1);

    std::vector<std::string> labels;
    for (const auto& d : lst)
        labels.push_back(d.label);

    ASSERT_EQ(labels.size(), 3);
    EXPECT_EQ(labels[0], "A");
    EXPECT_EQ(labels[1], "B");
    EXPECT_EQ(labels[2], "C");
}

TEST(PMRList, MemoryReusedAfterDestruction) {
    static std::array<std::byte, 4096> buffer;

    pmr_map_resource mem(buffer.data(), buffer.size());

    { pmr_forward_list<int> tmp(&mem);
      for (int i = 0; i < 10; i++) tmp.push_front(i);
    }

    pmr_forward_list<int> lst(&mem);
    lst.push_front(42);

    EXPECT_EQ(*lst.begin(), 42);
}

TEST(PMRList, IteratorForward) {
    static std::array<std::byte, 4096> buffer;

    pmr_map_resource mem(buffer.data(), buffer.size());
    pmr_forward_list<int> lst(&mem);

    lst.push_front(2);
    lst.push_front(1);

    auto it = lst.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(it, lst.end());
}

TEST(PMRList, SizeAndEmpty) {
    static std::array<std::byte, 4096> buffer;

    pmr_map_resource mem(buffer.data(), buffer.size());
    pmr_forward_list<int> lst(&mem);

    EXPECT_TRUE(lst.empty());
    EXPECT_EQ(lst.size(), 0);

    lst.push_front(7);

    EXPECT_FALSE(lst.empty());
    EXPECT_EQ(lst.size(), 1);

    lst.pop_front();

    EXPECT_TRUE(lst.empty());
    EXPECT_EQ(lst.size(), 0);
}
