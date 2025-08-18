#include <thread>
#include <chrono>

#include <RApiPlus.h>


class TradeUploader {
public:
    TradeUploader(ClickHouseConfig cfg,
                  std::size_t flushThreshold = 10000,
                  std::chrono::milliseconds maxDelay = std::chrono::minutes(1))
        : cfg_(std::move(cfg)),
          threshold_(flushThreshold),
          maxDelay_(maxDelay),
          stop_(false) {}

    void start() { worker_ = std::thread([this]{ run(); }); }
    void stop()  { stop_.store(true); if (worker_.joinable()) worker_.join(); }

private:
    ClickHouseConfig cfg_;
    const std::size_t threshold_;
    const std::chrono::milliseconds maxDelay_;
    std::atomic<bool> stop_;
    std::thread worker_;

    void run() {
        std::vector<RApi::TradeInfo> batch;
        batch.reserve(threshold_);
        auto lastFlush = std::chrono::steady_clock::now();

        while (!stop_.load(std::memory_order_relaxed)) {
            // drain some; you can choose a higher max to catch up quickly
            std::size_t drained = g_tradeQueue.bulk_pop(batch, threshold_ - batch.size());

            auto now = std::chrono::steady_clock::now();
            bool sizeTrigger = batch.size() >= threshold_;
            bool timeTrigger = !batch.empty() && (now - lastFlush) >= maxDelay_;

            if (sizeTrigger || timeTrigger) {
                if (uploadBatch(batch)) {
                    batch.clear();
                    lastFlush = std::chrono::steady_clock::now();
                } else {
                    // backoff on failure
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            } else {
                // light sleep to avoid busy spin; adjust if latency-critical
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }

        // Flush remaining on stop
        if (!batch.empty()) {
            uploadBatch(batch);
            batch.clear();
        }
        // Also drain remaining items, if you want a clean shutdown
        std::vector<RApi::TradeInfo> tail;
        while (g_tradeQueue.bulk_pop(tail, 65536)) {
            if (!tail.empty()) {
                uploadBatch(tail);
                tail.clear();
            }
        }
    }
};
