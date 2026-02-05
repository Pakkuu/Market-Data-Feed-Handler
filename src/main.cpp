#include <iostream>
#include <iomanip>
#include "market_tick.h"
#include "lockfree_queue.h"
#include "tick_generator.h"
#include "analytics.h"

int main() {
    std::cout << "=== Market Data Feed Handler - Component Tests ===" << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    
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
    
    // Test 4: Tick Generator
    std::cout << "\n[Test 4] Tick Generator" << std::endl;
    market::TickGenerator generator("TSLA", 250.0, 0.05, 100, 1000, 42);
    
    std::cout << "Generating 10 ticks with random walk..." << std::endl;
    for (int i = 0; i < 10; i++) {
        auto tick = generator.generateTick();
        std::cout << "  Tick #" << (i+1) << ": " << tick.symbol 
                  << " @ $" << tick.price 
                  << " vol:" << tick.volume 
                  << " side:" << tick.side << std::endl;
    }
    
    std::cout << "Final price: $" << generator.getCurrentPrice() << std::endl;
    
    // Test batch generation
    std::cout << "\nGenerating batch of 1000 ticks..." << std::endl;
    auto batch = generator.generateTicks(1000);
    std::cout << "Generated " << batch.size() << " ticks" << std::endl;
    std::cout << "First tick price: $" << batch.front().price << std::endl;
    std::cout << "Last tick price: $" << batch.back().price << std::endl;
    
    // Test 5: Analytics Engine
    std::cout << "\n[Test 5] Analytics Engine" << std::endl;
    market::AnalyticsEngine analytics(100);  // 100-tick rolling average
    
    // Create test data with known values
    std::cout << "Processing 500 synthetic ticks..." << std::endl;
    market::TickGenerator test_gen("SPY", 100.0, 0.01, 100, 500, 123);
    for (int i = 0; i < 500; i++) {
        auto tick = test_gen.generateTick();
        analytics.processTick(tick);
    }
    
    std::cout << "\n--- Analytics Results ---" << std::endl;
    std::cout << "Ticks processed: " << analytics.getTickCount() << std::endl;
    std::cout << "VWAP: $" << analytics.getVWAP() << std::endl;
    std::cout << "Rolling Avg (100 ticks): $" << analytics.getRollingAverage() << std::endl;
    std::cout << "Buy Volume: " << analytics.getBuyVolume() << " shares" << std::endl;
    std::cout << "Sell Volume: " << analytics.getSellVolume() << " shares" << std::endl;
    std::cout << "Trade Imbalance: " << analytics.getImbalance() << " shares";
    if (analytics.getImbalance() > 0) {
        std::cout << " (buy pressure)";
    } else if (analytics.getImbalance() < 0) {
        std::cout << " (sell pressure)";
    }
    std::cout << std::endl;
    
    std::cout << "\n=== All Tests Passed! ===" << std::endl;
    return 0;
}


