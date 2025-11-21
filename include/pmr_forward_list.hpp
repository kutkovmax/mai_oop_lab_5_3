#ifndef PMR_FORWARD_LIST_HPP
#define PMR_FORWARD_LIST_HPP

#include <memory_resource>
#include <iterator>
#include <utility>
#include <memory>

template<typename T>
class pmr_forward_list {
private:
    struct Node {
        T value;
        Node* next = nullptr;

        template<class... Args>
        explicit Node(Args&&... args)
            : value(std::forward<Args>(args)...) {}
    };

public:
    using value_type = T;
    using allocator_type = std::pmr::polymorphic_allocator<Node>;

    class iterator {
        Node* node_ = nullptr;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        iterator() = default;
        explicit iterator(Node* n) : node_(n) {}

        reference operator*() const { return node_->value; }
        pointer operator->() const { return &node_->value; }

        iterator& operator++() { node_ = node_->next; return *this; }
        iterator operator++(int) { iterator tmp(*this); ++(*this); return tmp; }

        friend bool operator==(const iterator& a, const iterator& b) {
            return a.node_ == b.node_;
        }
        friend bool operator!=(const iterator& a, const iterator& b) {
            return !(a == b);
        }
    };

public:
    explicit pmr_forward_list(std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : alloc_(mr) {}

    explicit pmr_forward_list(const allocator_type& alloc)
        : alloc_(alloc) {}

    ~pmr_forward_list() { clear(); }

    pmr_forward_list(const pmr_forward_list&) = delete;
    pmr_forward_list& operator=(const pmr_forward_list&) = delete;

    // ------------------------
    // Modifiers
    // ------------------------

    void push_front(const T& value) {
        Node* n = create_node(value);
        link_front(n);
    }

    void push_front(T&& value) {
        Node* n = create_node(std::move(value));
        link_front(n);
    }

    template<class... Args>
    void emplace_front(Args&&... args) {
        Node* n = create_node(std::forward<Args>(args)...);
        link_front(n);
    }

    void pop_front() {
        if (!head_) return;
        Node* next = head_->next;
        destroy_node(head_);
        head_ = next;
        --size_;
    }

    void clear() {
        while (head_) pop_front();
    }

    // ------------------------
    // Iterators
    // ------------------------
    iterator begin() { return iterator(head_); }
    iterator end() { return iterator(nullptr); }

    // ------------------------
    // Observers
    // ------------------------
    std::size_t size() const { return size_; }
    bool empty() const { return head_ == nullptr; }

private:
    template<class... Args>
    Node* create_node(Args&&... args) {
        Node* n = alloc_.allocate(1);

        try {
            std::allocator_traits<allocator_type>::construct(
                alloc_, n, std::forward<Args>(args)...);
        } catch (...) {
            alloc_.deallocate(n, 1);
            throw;
        }

        return n;
    }

    void destroy_node(Node* n) {
        std::allocator_traits<allocator_type>::destroy(alloc_, n);
        alloc_.deallocate(n, 1);
    }

    void link_front(Node* n) {
        n->next = head_;
        head_ = n;
        ++size_;
    }

private:
    allocator_type alloc_;
    Node* head_ = nullptr;
    std::size_t size_ = 0;
};

#endif
