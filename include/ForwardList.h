#pragma once

#include <memory_resource>
#include <stdexcept>

#include "ListIterator.h"
#include "Node.h"

template <typename T>
class ForwardList {
public:
    using Alloc = std::pmr::polymorphic_allocator<Node<T>>;
    using iterator = ListIterator<T>;
    using const_iterator = ListIterator<const T>;

    // Конструктор: инициализируем alloc первым (порядок полей см. ниже)
    ForwardList(std::pmr::memory_resource* res)
        : alloc(res), head(nullptr), before_head{T{}, nullptr}
    {}

    ~ForwardList() {
        clear();
    }

    // Добавить в начало
    void push_front(const T& value) {
        Node<T>* node = alloc.allocate(1);
        std::allocator_traits<Alloc>::construct(alloc, node, Node<T>{value, head});
        head = node;
        before_head.next = head; // поддерживаем before_head
    }

    // Удалить первый элемент
    void pop_front() {
        if (!head) return;
        Node<T>* old = head;
        head = head->next;
        this->destroy_node(old);
        before_head.next = head; // обновляем before_head
    }

    // Добавить в конец
    void push_back(const T& value) {
        Node<T>* node = alloc.allocate(1);
        std::allocator_traits<Alloc>::construct(alloc, node, Node<T>{value, nullptr});

        if (!head) {
            head = node;
            before_head.next = head; // список был пуст — обновляем before_head
            return;
        }

        Node<T>* curr = head;
        while (curr->next)
            curr = curr->next;

        curr->next = node;
    }

    // Удалить последний элемент
    void pop_back() {
        if (!head) return;

        if (!head->next) {
            pop_front();
            return;
        }

        Node<T>* curr = head;
        while (curr->next->next)
            curr = curr->next;

        Node<T>* old = curr->next;
        curr->next = nullptr;
        this->destroy_node(old);
    }

    // Вставить после позиции
    iterator insert_after(iterator pos, const T& value) {
        Node<T>* p = pos.nodeptr();

        if (!p) // неверный итератор
            throw std::logic_error("insert_after: invalid iterator");

        // Явно обрабатываем вставку после before_head (это — вставка в начало)
        if (p == reinterpret_cast<Node<T>*>(&before_head)) {
            // эквивалент push_front, но без повторного обновления before_head (ниже мы обновим)
            Node<T>* node = alloc.allocate(1);
            std::allocator_traits<Alloc>::construct(alloc, node, Node<T>{value, head});
            head = node;
            before_head.next = head;
            return iterator(node);
        }

        // Обычная вставка после реального узла
        Node<T>* node = alloc.allocate(1);
        std::allocator_traits<Alloc>::construct(alloc, node, Node<T>{value, p->next});
        p->next = node;

        return iterator(node);
    }

    // Удалить элемент после позиции
    iterator erase_after(iterator pos) {
        Node<T>* p = pos.nodeptr();
        if (!p || !p->next)
            return end();

        Node<T>* victim = p->next;
        p->next = victim->next;
        this->destroy_node(victim);

        // если удалили первый реальный элемент, скорректируем head/before_head
        if (p == reinterpret_cast<Node<T>*>(&before_head))
            head = p->next;

        before_head.next = head;
        return iterator(p->next);
    }

    // Итераторы
    iterator begin() { return iterator(head); }
    iterator end()   { return iterator(nullptr); }

    const_iterator begin() const { return const_iterator(head); }
    const_iterator end()   const { return const_iterator(nullptr); }

    const_iterator cbegin() const { return const_iterator(head); }
    const_iterator cend()   const { return const_iterator(nullptr); }

    // Перед началом (нужно для insert_after)
    iterator before_begin() { return iterator(reinterpret_cast<Node<T>*>(&before_head)); }
    const_iterator before_begin() const { return const_iterator(reinterpret_cast<Node<T>*>(&before_head)); }

    // Очистить список
    void clear() {
        Node<T>* curr = head;
        while (curr) {
            Node<T>* nxt = curr->next;
            this->destroy_node(curr);
            curr = nxt;
        }
        head = nullptr;
        before_head.next = nullptr; // не забываем обновить технический узел
    }

private:
    Alloc alloc;
    Node<T>* head = nullptr;
    Node<T> before_head { T{}, nullptr };

    // Уничтожение одного узла
    void destroy_node(Node<T>* node) {
        std::allocator_traits<Alloc>::destroy(alloc, node);
        alloc.deallocate(node, 1);
    }
};
