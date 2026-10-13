#pragma once

#include <array>
#include <atomic>
#include <cstddef>

namespace alors
{

template <typename T, std::size_t Capacity>
class SpscQueue
{
    static_assert ((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two");

public:
    bool push (const T& item)
    {
        const auto tail = tailIndex.load (std::memory_order_relaxed);

        if (tail - headIndex.load (std::memory_order_acquire) == Capacity)
            return false;

        slots[tail & (Capacity - 1)] = item;
        tailIndex.store (tail + 1, std::memory_order_release);
        return true;
    }

    const T* front() const
    {
        const auto head = headIndex.load (std::memory_order_relaxed);

        if (head == tailIndex.load (std::memory_order_acquire))
            return nullptr;

        return &slots[head & (Capacity - 1)];
    }

    void pop()
    {
        headIndex.store (headIndex.load (std::memory_order_relaxed) + 1, std::memory_order_release);
    }

    void clear()
    {
        headIndex.store (tailIndex.load (std::memory_order_acquire), std::memory_order_release);
    }

private:
    std::array<T, Capacity> slots {};
    std::atomic<std::size_t> headIndex { 0 };
    std::atomic<std::size_t> tailIndex { 0 };
};

}
