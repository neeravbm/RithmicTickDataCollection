#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>
#include <type_traits>
#include <algorithm>

template <typename T, std::size_t CapacityPow2>
class SpscRingBuffer {
    static_assert((CapacityPow2 & (CapacityPow2 - 1)) == 0, "Capacity must be a power of two.");
public:
    SpscRingBuffer() : head_(0), tail_(0), buffer_(std::make_unique<T[]>(CapacityPow2)) {}


    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;
    SpscRingBuffer(SpscRingBuffer&&) = delete;
    SpscRingBuffer& operator=(SpscRingBuffer&&) = delete;

    // Single producer thread
    bool try_push(const T& v) {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t next = (head + 1) & mask_;
        if (next == tail_.load(std::memory_order_acquire)) {
            return false; // full
        }
        buffer_[head] = v;                               // write item
        head_.store(next, std::memory_order_release);    // publish
        return true;
    }

    // Single consumer thread: move up to max_items into out (appends)
    std::size_t bulk_pop(std::vector<T>& out, std::size_t max_items) {
        std::size_t tail = tail_.load(std::memory_order_relaxed);
        const std::size_t head = head_.load(std::memory_order_acquire);

        std::size_t available = (head + CapacityPow2 - tail) & mask_;
        std::size_t n = std::min(available, max_items);

        for (std::size_t i = 0; i < n; ++i) {
            out.emplace_back(buffer_[(tail + i) & mask_]);
        }
        tail = (tail + n) & mask_;
        tail_.store(tail, std::memory_order_release);
        return n;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    std::size_t capacity() const { return CapacityPow2 - 1; } // one slot kept free

private:
    static constexpr std::size_t mask_ = CapacityPow2 - 1;
    alignas(64) std::atomic<std::size_t> head_; // producer writes
    alignas(64) std::atomic<std::size_t> tail_; // consumer writes
    std::unique_ptr<T[]> buffer_;  // <-- heap
};

