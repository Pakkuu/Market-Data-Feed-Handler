#include "tick_generator.h"

namespace market {

TickGenerator::TickGenerator(const std::string& symbol,
                             double base_price,
                             double price_step,
                             int min_volume,
                             int max_volume,
                             unsigned int seed)
    : symbol_(symbol),
      current_price_(base_price),
      price_step_(price_step),
      min_volume_(min_volume),
      max_volume_(max_volume),
      rng_(seed == 0 ? std::random_device{}() : seed),
      price_dist_(-1.0, 1.0),
      volume_dist_(min_volume, max_volume),
      side_dist_(0, 1)
{
}

MarketTick TickGenerator::generateTick() {
    // Random walk: price moves up or down by a small amount
    double price_change = price_dist_(rng_) * price_step_;
    current_price_ += price_change;
    
    // Ensure price doesn't go negative
    if (current_price_ < 0.01) {
        current_price_ = 0.01;
    }
    
    // Generate random volume
    int volume = volume_dist_(rng_);
    
    // Random buy/sell side
    char side = (side_dist_(rng_) == 0) ? 'B' : 'S';
    
    // Current timestamp
    uint64_t timestamp = getCurrentTimeNanos();
    
    return MarketTick(symbol_, current_price_, volume, side, timestamp);
}

std::vector<MarketTick> TickGenerator::generateTicks(size_t count) {
    std::vector<MarketTick> ticks;
    ticks.reserve(count);
    
    for (size_t i = 0; i < count; i++) {
        ticks.push_back(generateTick());
    }
    
    return ticks;
}

void TickGenerator::resetPrice(double base_price) {
    current_price_ = base_price;
}

} // namespace market
