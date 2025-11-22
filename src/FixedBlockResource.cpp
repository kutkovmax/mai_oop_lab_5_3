#include "FixedBlockResource.h"

FixedBlockResource::FixedBlockResource(size_t size)
    : buffer_size(size), offset(0)
{
    buffer = static_cast<char*>(std::malloc(size));
    if (!buffer)
        throw std::bad_alloc();
}

FixedBlockResource::~FixedBlockResource() {
    if (buffer)
        std::free(buffer);
}

void* FixedBlockResource::do_allocate(size_t bytes, size_t alignment) {
    // 1) Try reuse freed blocks
    for (auto& [ptr, info] : blocks) {
        if (info.free && info.size >= bytes) {
            info.free = false;
            return ptr;
        }
    }

    // 2) Allocate new block inside fixed buffer
    size_t remainder = offset % alignment;
    if (remainder)
        offset += (alignment - remainder);

    if (offset + bytes > buffer_size)
        throw std::bad_alloc();

    void* ptr = buffer + offset;

    blocks[ptr] = BlockInfo{bytes, false};
    offset += bytes;

    return ptr;
}

void FixedBlockResource::do_deallocate(void* ptr, size_t bytes, size_t alignment) {
    (void)bytes;
    (void)alignment;

    auto it = blocks.find(ptr);
    if (it == blocks.end())
        throw std::logic_error("Attempt to free unallocated block");

    it->second.free = true;
}

bool FixedBlockResource::do_is_equal(const memory_resource& other) const noexcept {
    return this == &other;
}

size_t FixedBlockResource::used_blocks_count() const {
    return blocks.size();
}
