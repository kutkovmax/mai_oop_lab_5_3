#pragma once

#include "Node.h"

template <typename T>
class ListIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        ListIterator(Node<T>* ptr) : current(ptr) {}

        reference operator*() const {
            return current->value;
        }

        pointer operator->() const {
            return &current->value;
        }

        ListIterator& operator++() {
            current = current->next;
            return *this;
        }

        bool operator!=(const ListIterator& other) const {
            return current != other.current;
        }

        bool operator==(const ListIterator& other) const {
            return current == other.current;
        }

    private:
        Node<T>* current = nullptr;
};