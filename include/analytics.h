#ifndef ANALYTICS_H
#define ANALYTICS_H

#include "market_tick.h"
#include <deque>
#include <cstdint>

namespace market {

/**
 * @brief Calculates Volume-Weighted Average Price (VWAP)
 * 
 * VWAP = Σ(price * volume) / Σ(volume)
 */
class VWAPCalculator {
private:
    double total_price_volume_;  // Σ(price * volume)
    int64_t total_volume_;       // Σ(volume)
    
public:
    VWAPCalculator() : total_price_volume_(0.0), total_volume_(0) {}
    
    /**
     * @brief Add a tick to the VWAP calculation
     * @param tick Market tick to process
     */
    void addTick(const MarketTick& tick) {
        total_price_volume_ += tick.price * tick.volume;
        total_volume_ += tick.volume;
    }
    
    /**
     * @brief Get current VWAP
     * @return double VWAP value, 0.0 if no volume
     */
    double getVWAP() const {
        if (total_volume_ == 0) return 0.0;
        return total_price_volume_ / total_volume_;
    }
    
    /**
     * @brief Reset calculator
     */
    void reset() {
        total_price_volume_ = 0.0;
        total_volume_ = 0;
    }
    
    /**
     * @brief Get total volume processed
     * @return int64_t Total volume
     */
    int64_t getTotalVolume() const { return total_volume_; }
};

/**
 * @brief Tracks trade imbalance (buy volume - sell volume)
 */
class TradeImbalanceCalculator {
private:
    int64_t buy_volume_;
    int64_t sell_volume_;
    
public:
    TradeImbalanceCalculator() : buy_volume_(0), sell_volume_(0) {}
    
    /**
     * @brief Add a tick to imbalance calculation
     * @param tick Market tick to process
     */
    void addTick(const MarketTick& tick) {
        if (tick.side == 'B') {
            buy_volume_ += tick.volume;
        } else if (tick.side == 'S') {
            sell_volume_ += tick.volume;
        }
    }
    
    /**
     * @brief Get trade imbalance
     * @return int64_t Buy volume - Sell volume (positive = buy pressure)
     */
    int64_t getImbalance() const {
        return buy_volume_ - sell_volume_;
    }
    
    /**
     * @brief Get buy volume
     */
    int64_t getBuyVolume() const { return buy_volume_; }
    
    /**
     * @brief Get sell volume
     */
    int64_t getSellVolume() const { return sell_volume_; }
    
    /**
     * @brief Reset calculator
     */
    void reset() {
        buy_volume_ = 0;
        sell_volume_ = 0;
    }
};

/**
 * @brief Calculates rolling average price over N ticks
 */
class RollingAverageCalculator {
private:
    std::deque<double> prices_;
    size_t window_size_;
    double sum_;
    
public:
    /**
     * @brief Constructor
     * @param window_size Number of ticks to average over
     */
    RollingAverageCalculator(size_t window_size = 100) 
        : window_size_(window_size), sum_(0.0) {}
    
    /**
     * @brief Add a tick to rolling average
     * @param tick Market tick to process
     */
    void addTick(const MarketTick& tick) {
        prices_.push_back(tick.price);
        sum_ += tick.price;
        
        // Remove oldest if window is full
        if (prices_.size() > window_size_) {
            sum_ -= prices_.front();
            prices_.pop_front();
        }
    }
    
    /**
     * @brief Get current rolling average
     * @return double Average price, 0.0 if no data
     */
    double getAverage() const {
        if (prices_.empty()) return 0.0;
        return sum_ / prices_.size();
    }
    
    /**
     * @brief Get number of ticks in current window
     */
    size_t getCount() const { return prices_.size(); }
    
    /**
     * @brief Reset calculator
     */
    void reset() {
        prices_.clear();
        sum_ = 0.0;
    }
};

/**
 * @brief Combined analytics engine
 */
class AnalyticsEngine {
private:
    VWAPCalculator vwap_;
    TradeImbalanceCalculator imbalance_;
    RollingAverageCalculator rolling_avg_;
    size_t tick_count_;
    
public:
    /**
     * @brief Constructor
     * @param rolling_window Size of rolling average window
     */
    AnalyticsEngine(size_t rolling_window = 100)
        : rolling_avg_(rolling_window), tick_count_(0) {}
    
    /**
     * @brief Process a tick through all analytics
     * @param tick Market tick to process
     */
    void processTick(const MarketTick& tick) {
        vwap_.addTick(tick);
        imbalance_.addTick(tick);
        rolling_avg_.addTick(tick);
        tick_count_++;
    }
    
    /**
     * @brief Get VWAP
     */
    double getVWAP() const { return vwap_.getVWAP(); }
    
    /**
     * @brief Get trade imbalance
     */
    int64_t getImbalance() const { return imbalance_.getImbalance(); }
    
    /**
     * @brief Get rolling average
     */
    double getRollingAverage() const { return rolling_avg_.getAverage(); }
    
    /**
     * @brief Get total tick count
     */
    size_t getTickCount() const { return tick_count_; }
    
    /**
     * @brief Get buy/sell volumes
     */
    int64_t getBuyVolume() const { return imbalance_.getBuyVolume(); }
    int64_t getSellVolume() const { return imbalance_.getSellVolume(); }
    
    /**
     * @brief Reset all analytics
     */
    void reset() {
        vwap_.reset();
        imbalance_.reset();
        rolling_avg_.reset();
        tick_count_ = 0;
    }
};

} // namespace market

#endif // ANALYTICS_H
