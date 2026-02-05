#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "market_tick.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>

namespace benchmark {

/**
 * @brief Tracks latency measurements for statistical analysis
 */
class LatencyTracker {
private:
    std::vector<double> latencies_micros_;  // Store all latencies in microseconds
    
public:
    /**
     * @brief Add a latency measurement
     * @param latency_micros Latency in microseconds
     */
    void addLatency(double latency_micros) {
        latencies_micros_.push_back(latency_micros);
    }
    
    /**
     * @brief Calculate percentile
     * @param percentile Percentile to calculate (0.0 to 1.0)
     * @return double Latency at given percentile in microseconds
     */
    double getPercentile(double percentile) const {
        if (latencies_micros_.empty()) return 0.0;
        
        std::vector<double> sorted = latencies_micros_;
        std::sort(sorted.begin(), sorted.end());
        
        size_t index = static_cast<size_t>(percentile * sorted.size());
        if (index >= sorted.size()) index = sorted.size() - 1;
        
        return sorted[index];
    }
    
    /**
     * @brief Get P50 (median) latency
     */
    double getP50() const { return getPercentile(0.50); }
    
    /**
     * @brief Get P99 latency
     */
    double getP99() const { return getPercentile(0.99); }
    
    /**
     * @brief Get P999 latency
     */
    double getP999() const { return getPercentile(0.999); }
    
    /**
     * @brief Get mean latency
     */
    double getMean() const {
        if (latencies_micros_.empty()) return 0.0;
        return std::accumulate(latencies_micros_.begin(), latencies_micros_.end(), 0.0) / latencies_micros_.size();
    }
    
    /**
     * @brief Get minimum latency
     */
    double getMin() const {
        if (latencies_micros_.empty()) return 0.0;
        return *std::min_element(latencies_micros_.begin(), latencies_micros_.end());
    }
    
    /**
     * @brief Get maximum latency
     */
    double getMax() const {
        if (latencies_micros_.empty()) return 0.0;
        return *std::max_element(latencies_micros_.begin(), latencies_micros_.end());
    }
    
    /**
     * @brief Get number of samples
     */
    size_t getCount() const { return latencies_micros_.size(); }
    
    /**
     * @brief Reset all measurements
     */
    void reset() {
        latencies_micros_.clear();
    }
};

/**
 * @brief Measures throughput (items per second)
 */
class ThroughputMeter {
private:
    size_t item_count_;
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point end_time_;
    bool running_;
    
public:
    ThroughputMeter() : item_count_(0), running_(false) {}
    
    /**
     * @brief Start measuring
     */
    void start() {
        start_time_ = std::chrono::steady_clock::now();
        item_count_ = 0;
        running_ = true;
    }
    
    /**
     * @brief Stop measuring
     */
    void stop() {
        end_time_ = std::chrono::steady_clock::now();
        running_ = false;
    }
    
    /**
     * @brief Increment item count
     */
    void addItem() {
        item_count_++;
    }
    
    /**
     * @brief Add multiple items
     */
    void addItems(size_t count) {
        item_count_ += count;
    }
    
    /**
     * @brief Get throughput in items per second
     */
    double getThroughput() const {
        if (item_count_ == 0) return 0.0;
        
        auto duration = running_ 
            ? std::chrono::steady_clock::now() - start_time_
            : end_time_ - start_time_;
            
        double seconds = std::chrono::duration<double>(duration).count();
        if (seconds == 0.0) return 0.0;
        
        return item_count_ / seconds;
    }
    
    /**
     * @brief Get elapsed time in seconds
     */
    double getElapsedSeconds() const {
        auto duration = running_ 
            ? std::chrono::steady_clock::now() - start_time_
            : end_time_ - start_time_;
        return std::chrono::duration<double>(duration).count();
    }
    
    /**
     * @brief Get item count
     */
    size_t getItemCount() const { return item_count_; }
};

/**
 * @brief Combined benchmark results
 */
struct BenchmarkResults {
    std::string name;
    size_t ticks_processed;
    double throughput_tps;  // Ticks per second
    double latency_mean;
    double latency_p50;
    double latency_p99;
    double latency_p999;
    double latency_min;
    double latency_max;
    double elapsed_seconds;
    
    /**
     * @brief Print results to console
     */
    void print() const {
        std::cout << "\n=== " << name << " ===" << std::endl;
        std::cout << "Ticks Processed:   " << ticks_processed << std::endl;
        std::cout << "Elapsed Time:      " << elapsed_seconds << " seconds" << std::endl;
        std::cout << "Throughput:        " << static_cast<int>(throughput_tps) << " ticks/sec" << std::endl;
        std::cout << "\nLatency Statistics (microseconds):" << std::endl;
        std::cout << "  Mean:  " << latency_mean << " μs" << std::endl;
        std::cout << "  Min:   " << latency_min << " μs" << std::endl;
        std::cout << "  P50:   " << latency_p50 << " μs" << std::endl;
        std::cout << "  P99:   " << latency_p99 << " μs" << std::endl;
        std::cout << "  P999:  " << latency_p999 << " μs" << std::endl;
        std::cout << "  Max:   " << latency_max << " μs" << std::endl;
    }
    
    /**
     * @brief Export to CSV
     */
    static void exportToCSV(const std::vector<BenchmarkResults>& results, const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) return;
        
        // Header
        file << "Name,Ticks,Throughput_TPS,Latency_Mean,Latency_P50,Latency_P99,Latency_P999,Latency_Min,Latency_Max,Elapsed_Sec\n";
        
        // Data
        for (const auto& r : results) {
            file << r.name << ","
                 << r.ticks_processed << ","
                 << r.throughput_tps << ","
                 << r.latency_mean << ","
                 << r.latency_p50 << ","
                 << r.latency_p99 << ","
                 << r.latency_p999 << ","
                 << r.latency_min << ","
                 << r.latency_max << ","
                 << r.elapsed_seconds << "\n";
        }
        
        file.close();
    }
};

/**
 * @brief Run comprehensive benchmarks comparing lock-free vs mutex queues
 */
void runComprehensiveBenchmarks();

} // namespace benchmark

#endif // BENCHMARK_H
