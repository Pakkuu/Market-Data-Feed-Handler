#ifndef LOCKFREE_QUEUE_H
#define LOCKFREE_QUEUE_H

#include <atomic>
#include <memory>
#include <optional>

namespace lockfree {

/**
 * @brief Lock-free Single Producer Single Consumer (SPSC) Queue
 * 
 * Uses atomic operations and compare-and-swap for thread-safe
 * lock-free enqueueing and dequeueing of elements.
 * 
 * @tparam T Type of elements stored in the queue
 */
template<typename T>
class SPSCQueue {
private:
    /**
     * @brief Node structure for linked list
     */
    struct Node {
        T data;
        std::atomic<Node*> next;
        
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    std::atomic<Node*> head_;  // Consumer reads from head
    std::atomic<Node*> tail_;  // Producer writes to tail
    
public:
    /**
     * @brief Constructor - initializes with a dummy node
     */
    SPSCQueue() {
        Node* dummy = new Node(T{});
        head_.store(dummy, std::memory_order_relaxed);
        tail_.store(dummy, std::memory_order_relaxed);
    }
    
    /**
     * @brief Destructor - cleans up all nodes
     */
    ~SPSCQueue() {
        // Drain the queue
        while (pop()) {}
        
        // Delete the dummy node
        Node* head = head_.load(std::memory_order_relaxed);
        delete head;
    }
    
    // Disable copy and move
    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;
    
    /**
     * @brief Push an element to the queue (called by producer)
     * @param value Element to push
     */
    void push(const T& value) {
        Node* new_node = new Node(value);
        new_node->next.store(nullptr, std::memory_order_relaxed);
        
        // Get current tail and update it atomically
        Node* old_tail = tail_.load(std::memory_order_acquire);
        old_tail->next.store(new_node, std::memory_order_release);
        tail_.store(new_node, std::memory_order_release);
    }
    
    /**
     * @brief Pop an element from the queue (called by consumer)
     * @return std::optional<T> The popped element, or nullopt if queue is empty
     */
    std::optional<T> pop() {
        Node* old_head = head_.load(std::memory_order_acquire);
        Node* next = old_head->next.load(std::memory_order_acquire);
        
        // If next is null, queue is empty
        if (next == nullptr) {
            return std::nullopt;
        }
        
        // Extract value before moving head
        T value = next->data;
        
        // Move head forward
        head_.store(next, std::memory_order_release);
        
        // Delete old head (which was always a dummy node)
        delete old_head;
        
        return value;
    }
    
    /**
     * @brief Check if queue is empty
     * @return true if empty, false otherwise
     */
    bool empty() const {
        Node* head = head_.load(std::memory_order_acquire);
        Node* next = head->next.load(std::memory_order_acquire);
        return next == nullptr;
    }
};

} // namespace lockfree

#endif // LOCKFREE_QUEUE_H
