#pragma once

#include <memory_resource>
#include <map>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>

class FixedBlockResource : public std::pmr::memory_resource {
public:
    FixedBlockResource(size_t size);
    ~FixedBlockResource();

    size_t used_blocks_count() const;

protected:
    void* do_allocate(size_t bytes, size_t alignment) override;
    void do_deallocate(void* ptr, size_t bytes, size_t alignment) override;
    bool do_is_equal(const memory_resource& other) const noexcept override;

private:
    struct BlockInfo {
        size_t size;
        bool free;
    };

    char* buffer;
    size_t buffer_size;
    size_t offset;

    // key: pointer, value: block info
    std::map<void*, BlockInfo> blocks;
};
