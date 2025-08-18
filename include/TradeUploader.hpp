#include <chrono>
#include <iostream>
#include <thread>

#include <clickhouse/client.h>

#include "ClickhouseUtils.hpp"
#include "Rithmic.hpp"


class TradeUploader {
public:
    explicit TradeUploader(Rithmic* r, const std::size_t flushThreshold = 10000, const std::chrono::milliseconds maxDelay = std::chrono::minutes(1));
    void start();
    void stop();

private:
    const std::string TABLE_NAME = "trades";
    Rithmic* r;
    const std::size_t threshold_;
    const std::chrono::milliseconds maxDelay_;
    std::atomic<bool> stop_;
    std::thread worker_;

    void run() const;
    static clickhouse::Block BuildBlockForTrades(const std::vector<RApi::TradeInfo>& batch);
};
