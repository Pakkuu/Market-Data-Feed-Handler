#include "benchmark.h"
#include "market_tick.h"
#include "tick_generator.h"
#include "analytics.h"
#include "lockfree_queue.h"
#include "mutex_queue.h"
#include <thread>
#include <atomic>
#include <iostream>
#include <iomanip>
#include <memory>

namespace benchmark {

/**
 * @brief Producer thread function - generates and pushes ticks to queue
 */
template<typename QueueType>
void producerThread(QueueType& queue, 
                   size_t num_ticks,
                   std::atomic<bool>& done,
                   const std::string& symbol = "SPY") {
    market::TickGenerator generator(symbol, 100.0, 0.01, 100, 1000);
    
    for (size_t i = 0; i < num_ticks; i++) {
        auto tick = generator.generateTick();
        queue.push(tick);
    }
    
    done.store(true, std::memory_order_release);
}

/**
 * @brief Consumer thread function - pops ticks, calculates analytics and latency
 */
template<typename QueueType>
void consumerThread(QueueType& queue,
                   std::atomic<bool>& producer_done,
                   market::AnalyticsEngine& analytics,
                   LatencyTracker& latency_tracker,
                   ThroughputMeter& throughput) {
    throughput.start();
    
    while (true) {
        auto tick_opt = queue.pop();
        
        if (tick_opt.has_value()) {
            auto& tick = tick_opt.value();
            
            // Calculate latency
            uint64_t now = market::getCurrentTimeNanos();
            double latency_us = market::calculateLatencyMicros(tick.timestamp_ns, now);
            latency_tracker.addLatency(latency_us);
            
            // Process analytics
            analytics.processTick(tick);
            
            // Update throughput
            throughput.addItem();
        } 
        else {
            // Queue is empty, check if producer is done
            if (producer_done.load(std::memory_order_acquire)) {
                // Double-check queue is truly empty before exiting
                if (queue.empty()) {
                    break;
                }
            }
            // Brief yield to avoid busy-waiting
            std::this_thread::yield();
        }
    }
    
    throughput.stop();
}

/**
 * @brief Run benchmark with specified queue type
 */
template<typename QueueType>
BenchmarkResults runBenchmark(const std::string& name, size_t num_ticks) {
    std::cout << "\nRunning benchmark: " << name << " (" << num_ticks << " ticks)" << std::endl;
    
    // Create queue and synchronization primitives
    QueueType queue;
    std::atomic<bool> producer_done{false};
    
    // Create analytics and measurement tools
    market::AnalyticsEngine analytics(100);
    LatencyTracker latency_tracker;
    ThroughputMeter throughput;
    
    // Launch threads
    std::thread producer(producerThread<QueueType>, std::ref(queue), num_ticks, 
                        std::ref(producer_done), "SPY");
    std::thread consumer(consumerThread<QueueType>, std::ref(queue), 
                        std::ref(producer_done), std::ref(analytics), 
                        std::ref(latency_tracker), std::ref(throughput));
    
    // Wait for completion
    producer.join();
    consumer.join();
    
    // Collect results
    BenchmarkResults results;
    results.name = name;
    results.ticks_processed = latency_tracker.getCount();
    results.throughput_tps = throughput.getThroughput();
    results.latency_mean = latency_tracker.getMean();
    results.latency_p50 = latency_tracker.getP50();
    results.latency_p99 = latency_tracker.getP99();
    results.latency_p999 = latency_tracker.getP999();
    results.latency_min = latency_tracker.getMin();
    results.latency_max = latency_tracker.getMax();
    results.elapsed_seconds = throughput.getElapsedSeconds();
    
    std::cout << "  Completed: " << results.ticks_processed << " ticks in " 
              << results.elapsed_seconds << " seconds" << std::endl;
    std::cout << "  Throughput: " << static_cast<int>(results.throughput_tps) << " ticks/sec" << std::endl;
    std::cout << "  Latency P99: " << results.latency_p99 << " μs" << std::endl;
    
    return results;
}

/**
 * @brief Run comprehensive benchmarks comparing lock-free vs mutex queues
 */
void runComprehensiveBenchmarks() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Market Data Feed Handler - Benchmarks" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::vector<BenchmarkResults> all_results;
    
    // Test with different tick counts
    std::vector<size_t> test_sizes = {10000, 50000, 100000, 500000, 1000000};
    
    for (size_t size : test_sizes) {
        std::cout << "\n--- Testing with " << size << " ticks ---" << std::endl;
        
        // Lock-free queue
        auto lockfree_results = runBenchmark<lockfree::SPSCQueue<market::MarketTick>>(
            "Lock-Free SPSC (" + std::to_string(size) + ")", size);
        all_results.push_back(lockfree_results);
        
        // Mutex queue
        auto mutex_results = runBenchmark<lockfree::MutexQueue<market::MarketTick>>(
            "Mutex Queue (" + std::to_string(size) + ")", size);
        all_results.push_back(mutex_results);
        
        // Show comparison
        std::cout << "\n  Speedup: " << std::fixed << std::setprecision(2)
                  << (lockfree_results.throughput_tps / mutex_results.throughput_tps) << "x faster" << std::endl;
        std::cout << "  Latency improvement: " 
                  << (mutex_results.latency_p99 / lockfree_results.latency_p99) << "x better P99" << std::endl;
    }
    
    // Print summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "Summary of All Benchmarks" << std::endl;
    std::cout << "========================================" << std::endl;
    
    for (const auto& r : all_results) {
        r.print();
    }
    
    // Export to CSV
    BenchmarkResults::exportToCSV(all_results, "benchmark_results.csv");
    std::cout << "\nResults exported to: benchmark_results.csv" << std::endl;
}

} // namespace benchmark
