#pragma once

#include <memory_resource>
#include <iterator>
#include <stdexcept>

#include "ListIterator.h"

template <typename T>
class ForwardList {
    public:
        using Alloc = std::pmr::polymorphic_allocator<Node<T>>;

        ForwardList(std::pmr::memory_resource* res) : alloc(res) {}

        ~ForwardList() {
            Node<T>* curr = head;
            
            while (curr) {
                Node<T>* tmp = curr->next;
                destroy_node(curr);
                curr = tmp;
            }
            
            head = nullptr;
        }

        void push_front(const T& value) {
            Node<T>* new_node = alloc.allocate(1);
            alloc.construct(new_node, Node<T>{value, head});
            head = new_node;
        }

        void pop_front() {
            if (!head)
                return;
            
            Node<T>* old = head;
            head = head->next;

            destroy_node(old);
        }

        void push_back(const T& value) {
            Node<T>* new_node = alloc.allocate(1);
            alloc.construct(new_node, Node<T>{value, nullptr});

            if (!head) {
                head = new_node;
                return;
            }

            Node<T>* curr = head;
            while (curr->next)
                curr = curr->next;
            
            curr->next = new_node;
        }

        void pop_back() {
            if (!head)
                return;

            if (!head->next) {
                pop_front();
                return;
            }

            Node<T>* curr = head;
            while (curr->next && curr->next->next)
                curr = curr->next;
            
            Node<T>* old = curr->next;
            curr->next = nullptr;

            destroy_node(old);
        }

        Node<T>* get_head() const {
            return head;
        }

        ListIterator<T> begin() {
            return ListIterator<T>(head);
        }

        ListIterator<T> end() {
            return ListIterator<T>(nullptr);
        }

    private:
        void destroy_node(Node<T>* node) {
            std::allocator_traits<Alloc>::destroy(alloc, node);
            alloc.deallocate(node, 1);
        }

        Node<T>* head = nullptr;
        Alloc alloc;
};