#ifndef PMR_MAP_RESOURCE_HPP
#define PMR_MAP_RESOURCE_HPP

#include <memory_resource>
#include <map>
#include <cstddef>
#include <mutex>
#include <new>

class pmr_map_resource : public std::pmr::memory_resource {
public:
    // Constructor with internal memory allocation
    explicit pmr_map_resource(std::size_t bytes)
        : base_(::operator new(bytes)), size_(align_up(bytes)), owns_memory_(true)
    {
        free_ranges_.emplace(0, size_);
    }

    // Constructor from external buffer
    pmr_map_resource(void* buffer, std::size_t bytes)
        : base_(buffer), size_(align_up(bytes)), owns_memory_(false)
    {
        free_ranges_.emplace(0, size_);
    }

    ~pmr_map_resource() override {
        std::lock_guard lock(mutex_);

        if (owns_memory_)
            ::operator delete(base_);

        free_ranges_.clear();
        allocated_.clear();
    }

    pmr_map_resource(const pmr_map_resource&) = delete;
    pmr_map_resource& operator=(const pmr_map_resource&) = delete;

protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        std::lock_guard lock(mutex_);

        for (auto it = free_ranges_.begin(); it != free_ranges_.end(); ++it) {
            std::size_t offset = it->first;
            std::size_t block_size = it->second;

            void* raw_ptr = static_cast<char*>(base_) + offset;

            std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(raw_ptr);
            std::uintptr_t aligned = (addr + alignment - 1) & ~(alignment - 1);
            std::size_t padding = aligned - addr;

            if (padding + bytes > block_size)
                continue;

            // Update free ranges
            free_ranges_.erase(it);

            if (padding > 0)
                free_ranges_.emplace(offset, padding);

            if (block_size > padding + bytes)
                free_ranges_.emplace(aligned - reinterpret_cast<std::uintptr_t>(base_) + bytes,
                                    block_size - padding - bytes);

            void* result = reinterpret_cast<void*>(aligned);
            allocated_[result] = bytes + padding;
            return result;
        }

        throw std::bad_alloc();
    }

    void do_deallocate(void* p, std::size_t, std::size_t) override {
        if (!p) return;

        std::lock_guard lock(mutex_);

        auto it = allocated_.find(p);
        if (it == allocated_.end())
            return;  // double free protection

        std::size_t block_size = it->second;
        allocated_.erase(it);

        std::uintptr_t base_addr = reinterpret_cast<std::uintptr_t>(base_);
        std::uintptr_t ptr_addr = reinterpret_cast<std::uintptr_t>(p);

        std::size_t offset = ptr_addr - base_addr;
        free_ranges_.emplace(offset, block_size);

        merge_blocks(offset);
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

private:
    static std::size_t align_up(std::size_t n) {
        constexpr std::size_t page = 4096;
        return (n + page - 1) / page * page;
    }

    void merge_blocks(std::size_t offset) {
    auto it = free_ranges_.find(offset);
    if (it == free_ranges_.end()) return;

    // Try to merge with previous block
    if (it != free_ranges_.begin()) {
        auto prev = std::prev(it);
        if (prev->first + prev->second == it->first) {
            size_t new_offset = prev->first;
            size_t new_size = prev->second + it->second;

            free_ranges_.erase(prev);
            free_ranges_.erase(it);

            it = free_ranges_.emplace(new_offset, new_size).first;
        }
    }

    // Try to merge with next block
    auto next = std::next(it);
    if (next != free_ranges_.end() && it->first + it->second == next->first) {
        size_t new_offset = it->first;
        size_t new_size = it->second + next->second;

        free_ranges_.erase(it);
        free_ranges_.erase(next);

        free_ranges_.emplace(new_offset, new_size);
    }
}

private:
    void* base_;
    std::size_t size_;
    bool owns_memory_;

    std::map<std::size_t, std::size_t> free_ranges_;
    std::map<void*, std::size_t> allocated_;

    mutable std::mutex mutex_;
};

#endif
