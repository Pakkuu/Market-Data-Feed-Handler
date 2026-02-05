#ifndef MUTEX_QUEUE_H
#define MUTEX_QUEUE_H

#include <queue>
#include <mutex>
#include <optional>

namespace lockfree {

/**
 * @brief Mutex-based queue for performance comparison
 * 
 * This implementation uses std::queue with std::mutex for thread safety.
 * Used as a baseline to compare against the lock-free queue.
 * 
 * @tparam T Type of elements stored in the queue
 */
template<typename T>
class MutexQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    
public:
    MutexQueue() = default;
    
    // Disable copy and move
    MutexQueue(const MutexQueue&) = delete;
    MutexQueue& operator=(const MutexQueue&) = delete;
    
    /**
     * @brief Push an element to the queue
     * @param value Element to push
     */
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
    }
    
    /**
     * @brief Pop an element from the queue
     * @return std::optional<T> The popped element, or nullopt if queue is empty
     */
    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (queue_.empty()) {
            return std::nullopt;
        }
        
        T value = queue_.front();
        queue_.pop();
        return value;
    }
    
    /**
     * @brief Check if queue is empty
     * @return true if empty, false otherwise
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};

} // namespace lockfree

#endif // MUTEX_QUEUE_H
