#pragma once

#include <type_traits>
#include "Node.h"

template <typename T>
class ListIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = T*;
    using reference         = T&;

    // Node<remove_const_t<T>> — узлы всегда НЕ const
    using NodeType = Node<std::remove_const_t<T>>;

    ListIterator(NodeType* ptr) : current(ptr) {}

    // разыменование
    reference operator*() const {
        return current->value;
    }

    pointer operator->() const {
        return &current->value;
    }

    // ++it
    ListIterator& operator++() {
        if (current)
            current = current->next;
        return *this;
    }

    // сравнение
    bool operator==(const ListIterator& other) const {
        return current == other.current;
    }

    bool operator!=(const ListIterator& other) const {
        return current != other.current;
    }

    NodeType* nodeptr() const {
        return current;
    }

private:
    NodeType* current = nullptr;
};
