#ifndef MARKET_TICK_H
#define MARKET_TICK_H

#include <string>
#include <chrono>
#include <cstdint>

namespace market {

/**
 * @brief Represents a single market tick data point
 * 
 * Contains symbol information, price, volume, side (buy/sell),
 * and high-precision timestamp for latency measurement.
 */
struct MarketTick {
    std::string symbol;      // Ticker symbol (e.g., "SPY", "AAPL")
    double price;            // Trade price in dollars
    int volume;              // Number of shares traded
    char side;               // 'B' for buy, 'S' for sell
    uint64_t timestamp_ns;   // Nanosecond timestamp for latency tracking
    
    /**
     * @brief Default constructor
     */
    MarketTick() 
        : symbol(""), price(0.0), volume(0), side('B'), timestamp_ns(0) {}
    
    /**
     * @brief Parameterized constructor
     */
    MarketTick(const std::string& sym, double p, int vol, char s, uint64_t ts)
        : symbol(sym), price(p), volume(vol), side(s), timestamp_ns(ts) {}
};

/**
 * @brief Get current time in nanoseconds since epoch
 * @return uint64_t Nanosecond timestamp
 */
inline uint64_t getCurrentTimeNanos() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

/**
 * @brief Calculate latency in microseconds between two nanosecond timestamps
 * @param start_ns Start timestamp in nanoseconds
 * @param end_ns End timestamp in nanoseconds
 * @return double Latency in microseconds
 */
inline double calculateLatencyMicros(uint64_t start_ns, uint64_t end_ns) {
    return static_cast<double>(end_ns - start_ns) / 1000.0;
}

} // namespace market

#endif // MARKET_TICK_H
