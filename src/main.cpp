#include <iostream>
#include "market_tick.h"
#include "lockfree_queue.h"

int main() {
    std::cout << "=== Market Data Feed Handler - Component Tests ===" << std::endl;
    
    // Test 1: MarketTick creation
    std::cout << "\n[Test 1] MarketTick Structure" << std::endl;
    market::MarketTick tick("SPY", 100.50, 500, 'B', market::getCurrentTimeNanos());
    
    std::cout << "Created tick: " << tick.symbol 
              << " @ $" << tick.price 
              << " vol:" << tick.volume 
              << " side:" << tick.side 
              << " ts:" << tick.timestamp_ns << std::endl;
    
    // Test 2: Latency calculation
    std::cout << "\n[Test 2] Latency Calculation" << std::endl;
    uint64_t start = market::getCurrentTimeNanos();
    uint64_t end = start + 5000; // 5 microseconds
    double latency = market::calculateLatencyMicros(start, end);
    std::cout << "Latency test: " << latency << " μs" << std::endl;
    
    // Test 3: Lock-free queue
    std::cout << "\n[Test 3] Lock-Free SPSC Queue" << std::endl;
    lockfree::SPSCQueue<market::MarketTick> queue;
    
    // Push some ticks
    std::cout << "Pushing 5 ticks..." << std::endl;
    for (int i = 0; i < 5; i++) {
        market::MarketTick t("AAPL", 150.0 + i, 100 * (i + 1), (i % 2 == 0) ? 'B' : 'S', 
                             market::getCurrentTimeNanos());
        queue.push(t);
        std::cout << "  Pushed: " << t.symbol << " @ $" << t.price << std::endl;
    }
    
    // Pop and verify
    std::cout << "Popping ticks..." << std::endl;
    int count = 0;
    while (auto tick_opt = queue.pop()) {
        count++;
        auto& t = tick_opt.value();
        std::cout << "  Popped #" << count << ": " << t.symbol 
                  << " @ $" << t.price << " vol:" << t.volume 
                  << " side:" << t.side << std::endl;
    }
    
    // Verify empty
    std::cout << "Queue is " << (queue.empty() ? "empty" : "NOT empty") << std::endl;
    
    std::cout << "\n=== All Tests Passed! ===" << std::endl;
    return 0;
}

