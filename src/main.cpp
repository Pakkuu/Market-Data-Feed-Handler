#include <iostream>
#include "market_tick.h"

int main() {
    std::cout << "Market Data Feed Handler - Testing MarketTick" << std::endl;
    
    // Test MarketTick creation
    market::MarketTick tick("SPY", 100.50, 500, 'B', market::getCurrentTimeNanos());
    
    std::cout << "Created tick: " << tick.symbol 
              << " @ $" << tick.price 
              << " vol:" << tick.volume 
              << " side:" << tick.side 
              << " ts:" << tick.timestamp_ns << std::endl;
    
    // Test latency calculation
    uint64_t start = market::getCurrentTimeNanos();
    uint64_t end = start + 5000; // 5 microseconds
    double latency = market::calculateLatencyMicros(start, end);
    std::cout << "Latency test: " << latency << " μs" << std::endl;
    
    return 0;
}

