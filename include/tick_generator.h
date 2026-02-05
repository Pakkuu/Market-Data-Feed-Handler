#ifndef TICK_GENERATOR_H
#define TICK_GENERATOR_H

#include "market_tick.h"
#include <vector>
#include <random>
#include <string>

namespace market {

/**
 * @brief Generates synthetic market tick data using random walk
 * 
 * Simulates realistic market data with:
 * - Random walk price movements around a base price
 * - Random volumes within specified range
 * - Random buy/sell sides
 * - High-precision timestamps
 */
class TickGenerator {
private:
    std::string symbol_;
    double current_price_;
    double price_step_;        // Maximum price change per tick
    int min_volume_;
    int max_volume_;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> price_dist_;
    std::uniform_int_distribution<int> volume_dist_;
    std::uniform_int_distribution<int> side_dist_;
    
public:
    /**
     * @brief Constructor
     * @param symbol Ticker symbol (e.g., "SPY")
     * @param base_price Starting price (e.g., 100.0)
     * @param price_step Maximum price change per tick (e.g., 0.01)
     * @param min_volume Minimum trade volume
     * @param max_volume Maximum trade volume
     * @param seed Random seed for reproducibility (0 for random)
     */
    TickGenerator(const std::string& symbol = "SPY",
                  double base_price = 100.0,
                  double price_step = 0.01,
                  int min_volume = 100,
                  int max_volume = 1000,
                  unsigned int seed = 0);
    
    /**
     * @brief Generate a single tick
     * @return MarketTick Generated tick with current timestamp
     */
    MarketTick generateTick();
    
    /**
     * @brief Generate multiple ticks
     * @param count Number of ticks to generate
     * @return std::vector<MarketTick> Vector of generated ticks
     */
    std::vector<MarketTick> generateTicks(size_t count);
    
    /**
     * @brief Get current price
     * @return double Current price
     */
    double getCurrentPrice() const { return current_price_; }
    
    /**
     * @brief Reset price to base value
     * @param base_price New base price
     */
    void resetPrice(double base_price);
};

} // namespace market

#endif // TICK_GENERATOR_H
