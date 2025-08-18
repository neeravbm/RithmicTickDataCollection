#include <chrono>
#include <iostream>
#include <thread>

#include <clickhouse/client.h>

#include "ClickhouseUtils.hpp"
#include "Rithmic.hpp"
#include "TradeUploader.hpp"


TradeUploader::TradeUploader(Rithmic* r, const std::size_t flushThreshold, const std::chrono::milliseconds maxDelay) : r(r), threshold_(flushThreshold), maxDelay_(maxDelay), stop_(false) {}

void TradeUploader::start() {
    worker_ = std::thread([this] { run(); });
}

void TradeUploader::stop() {
    stop_.store(true);
    if (worker_.joinable()) worker_.join();
}

void TradeUploader::run() const {
    clickhouse::Client client(
        clickhouse::ClientOptions().SetHost(ClickhouseUtils::DB_HOST).SetDefaultDatabase(ClickhouseUtils::DB_NAME).SetUser(ClickhouseUtils::DB_USER).SetPassword(ClickhouseUtils::DB_PASSWORD));

    std::vector<RApi::TradeInfo> batch;
    batch.reserve(threshold_);
    auto lastFlush = std::chrono::steady_clock::now();

    while (!stop_.load(std::memory_order_relaxed)) {
        // Drain from your SPSC queue (implementation from earlier message)
        (void)r->g_tradeQueue.bulk_pop(batch, threshold_ - batch.size());

        const auto now = std::chrono::steady_clock::now();
        const bool sizeTrigger = batch.size() >= threshold_;
        const bool timeTrigger = !batch.empty() && (now - lastFlush) >= maxDelay_;

        if (!sizeTrigger && !timeTrigger) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }

        try {
            clickhouse::Block block = BuildBlockForTrades(batch);
            if (block.GetRowCount() > 0) {
                bool success = false;
                bool resetConnection = false;
                while (!success) {
                    try {
                        if (resetConnection) {
                            client.ResetConnection();
                        }
                        client.Insert(TABLE_NAME, block);
                    }
                    catch (std::exception& e) {
                        std::cout << e.what() << std::endl;
                        std::cout << "Inserting results into Clickhouse failed. Will return in 10 seconds." << std::endl << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(10));
                        resetConnection = true;
                        continue;
                    }

                    success = true;
                };
            }

            batch.clear();
            lastFlush = std::chrono::steady_clock::now();
        }
        catch (const std::exception& ex) {
            std::cerr << "[TradeUploaderCH] Insert failed: " << ex.what() << "\n";
            try { client.ResetConnection(); }
            catch (...) {}
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    // Final flush
    if (!batch.empty()) {
        try {
            if (clickhouse::Block block = BuildBlockForTrades(batch); block.GetRowCount() > 0) {
                bool success = false;
                bool resetConnection = false;
                while (!success) {
                    try {
                        if (resetConnection) {
                            client.ResetConnection();
                        }
                        client.Insert(TABLE_NAME, block);
                    }
                    catch (std::exception& e) {
                        std::cout << e.what() << std::endl;
                        std::cout << "Inserting results into Clickhouse failed. Will return in 10 seconds." << std::endl << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(10));
                        resetConnection = true;
                        continue;
                    }

                    success = true;
                };
            }
        }
        catch (...) {}
    }
}

clickhouse::Block TradeUploader::BuildBlockForTrades(const std::vector<RApi::TradeInfo>& batch) {
    clickhouse::Block block;
    const size_t n = batch.size();

    // ---------- Non-nullable columns: prepare std::vector<T> ----------
    std::vector<std::string> v_sExchange;
    v_sExchange.reserve(n);
    std::vector<std::string> v_sTicker;
    v_sTicker.reserve(n);
    std::vector<double> v_dPrice;
    v_dPrice.reserve(n);

    std::vector<uint8_t> v_bNetChangeFlag;
    v_bNetChangeFlag.reserve(n);
    std::vector<uint8_t> v_bPercentChangeFlag;
    v_bPercentChangeFlag.reserve(n);
    std::vector<uint8_t> v_bPriceFlag;
    v_bPriceFlag.reserve(n);
    std::vector<uint8_t> v_bVolumeBoughtFlag;
    v_bVolumeBoughtFlag.reserve(n);
    std::vector<uint8_t> v_bVolumeSoldFlag;
    v_bVolumeSoldFlag.reserve(n);
    std::vector<uint8_t> v_bVwapFlag;
    v_bVwapFlag.reserve(n);
    std::vector<uint8_t> v_bVwapLongFlag;
    v_bVwapLongFlag.reserve(n);

    std::vector<int32_t> v_iJopSsboe;
    v_iJopSsboe.reserve(n);
    std::vector<int32_t> v_iJopNsecs;
    v_iJopNsecs.reserve(n);
    std::vector<int32_t> v_iSourceSsboe;
    v_iSourceSsboe.reserve(n);
    std::vector<int32_t> v_iSourceUsecs;
    v_iSourceUsecs.reserve(n);
    std::vector<int32_t> v_iSourceNsecs;
    v_iSourceNsecs.reserve(n);
    std::vector<int32_t> v_iSsboe;
    v_iSsboe.reserve(n);
    std::vector<int32_t> v_iUsecs;
    v_iUsecs.reserve(n);

    std::vector<uint64_t> v_llSize;
    v_llSize.reserve(n);

    // ---------- Nullable columns: build (data,nulls) column pairs ----------
    // Float64 nullables
    auto c_dNetChange_data = std::make_shared<clickhouse::ColumnFloat64>();
    c_dNetChange_data->Reserve(n);
    auto c_dNetChange_nulls = std::make_shared<clickhouse::ColumnUInt8>();
    c_dNetChange_nulls->Reserve(n);
    auto c_dPercentChange_data = std::make_shared<clickhouse::ColumnFloat64>();
    c_dPercentChange_data->Reserve(n);
    auto c_dPercentChange_nulls = std::make_shared<clickhouse::ColumnUInt8>();
    c_dPercentChange_nulls->Reserve(n);
    auto c_dVwap_data = std::make_shared<clickhouse::ColumnFloat64>();
    c_dVwap_data->Reserve(n);
    auto c_dVwap_nulls = std::make_shared<clickhouse::ColumnUInt8>();
    c_dVwap_nulls->Reserve(n);
    auto c_dVwapLong_data = std::make_shared<clickhouse::ColumnFloat64>();
    c_dVwapLong_data->Reserve(n);
    auto c_dVwapLong_nulls = std::make_shared<clickhouse::ColumnUInt8>();
    c_dVwapLong_nulls->Reserve(n);

    // UInt64 nullables
    auto c_llVolumeBought_data = std::make_shared<clickhouse::ColumnUInt64>();
    c_llVolumeBought_data->Reserve(n);
    auto c_llVolumeBought_nulls = std::make_shared<clickhouse::ColumnUInt8>();
    c_llVolumeBought_nulls->Reserve(n);
    auto c_llVolumeSold_data = std::make_shared<clickhouse::ColumnUInt64>();
    c_llVolumeSold_data->Reserve(n);
    auto c_llVolumeSold_nulls = std::make_shared<clickhouse::ColumnUInt8>();
    c_llVolumeSold_nulls->Reserve(n);

    // String nullables
    auto c_sAggressorExchOrdId_data = std::make_shared<clickhouse::ColumnString>();
    c_sAggressorExchOrdId_data->Reserve(n);
    auto c_sAggressorExchOrdId_nulls = std::make_shared<clickhouse::ColumnUInt8>();
    c_sAggressorExchOrdId_nulls->Reserve(n);

    auto c_sAggressorSide_data = std::make_shared<clickhouse::ColumnString>();
    c_sAggressorSide_data->Reserve(n);
    auto c_sAggressorSide_nulls = std::make_shared<clickhouse::ColumnUInt8>();
    c_sAggressorSide_nulls->Reserve(n);

    auto c_sCondition_data = std::make_shared<clickhouse::ColumnString>();
    c_sCondition_data->Reserve(n);
    auto c_sCondition_nulls = std::make_shared<clickhouse::ColumnUInt8>();
    c_sCondition_nulls->Reserve(n);

    auto c_sExchOrdId_data = std::make_shared<clickhouse::ColumnString>();
    c_sExchOrdId_data->Reserve(n);
    auto c_sExchOrdId_nulls = std::make_shared<clickhouse::ColumnUInt8>();
    c_sExchOrdId_nulls->Reserve(n);

    // ---------- Fill vectors/columns ----------
    for (const auto& t : batch) {
        v_sExchange.emplace_back(Utils::toString(t.sExchange));
        v_sTicker.emplace_back(Utils::toString(t.sTicker));
        v_dPrice.emplace_back(t.dPrice);

        v_bNetChangeFlag.emplace_back(static_cast<uint8_t>(t.bNetChangeFlag));
        v_bPercentChangeFlag.emplace_back(static_cast<uint8_t>(t.bPercentChangeFlag));
        v_bPriceFlag.emplace_back(static_cast<uint8_t>(t.bPriceFlag));
        v_bVolumeBoughtFlag.emplace_back(static_cast<uint8_t>(t.bVolumeBoughtFlag));
        v_bVolumeSoldFlag.emplace_back(static_cast<uint8_t>(t.bVolumeSoldFlag));
        v_bVwapFlag.emplace_back(static_cast<uint8_t>(t.bVwapFlag));
        v_bVwapLongFlag.emplace_back(static_cast<uint8_t>(t.bVwapLongFlag));

        v_iJopSsboe.emplace_back(t.iJopSsboe);
        v_iJopNsecs.emplace_back(t.iJopNsecs);
        v_iSourceSsboe.emplace_back(t.iSourceSsboe);
        v_iSourceUsecs.emplace_back(t.iSourceUsecs);
        v_iSourceNsecs.emplace_back(t.iSourceNsecs);
        v_iSsboe.emplace_back(t.iSsboe);
        v_iUsecs.emplace_back(t.iUsecs);

        v_llSize.emplace_back(static_cast<uint64_t>(t.llSize));

        // Nullable Float64s (use flags)
        if (t.bNetChangeFlag) {
            c_dNetChange_data->Append(t.dNetChange);
            c_dNetChange_nulls->Append(0);
        }
        else {
            c_dNetChange_data->Append(0.0);
            c_dNetChange_nulls->Append(1);
        }

        if (t.bPercentChangeFlag) {
            c_dPercentChange_data->Append(t.dPercentChange);
            c_dPercentChange_nulls->Append(0);
        }
        else {
            c_dPercentChange_data->Append(0.0);
            c_dPercentChange_nulls->Append(1);
        }

        if (t.bVwapFlag) {
            c_dVwap_data->Append(t.dVwap);
            c_dVwap_nulls->Append(0);
        }
        else {
            c_dVwap_data->Append(0.0);
            c_dVwap_nulls->Append(1);
        }

        if (t.bVwapLongFlag) {
            c_dVwapLong_data->Append(t.dVwapLong);
            c_dVwapLong_nulls->Append(0);
        }
        else {
            c_dVwapLong_data->Append(0.0);
            c_dVwapLong_nulls->Append(1);
        }

        // Nullable UInt64s (use flags)
        if (t.bVolumeBoughtFlag) {
            c_llVolumeBought_data->Append(static_cast<uint64_t>(t.llVolumeBought));
            c_llVolumeBought_nulls->Append(0);
        }
        else {
            c_llVolumeBought_data->Append(0ULL);
            c_llVolumeBought_nulls->Append(1);
        }

        if (t.bVolumeSoldFlag) {
            c_llVolumeSold_data->Append(static_cast<uint64_t>(t.llVolumeSold));
            c_llVolumeSold_nulls->Append(0);
        }
        else {
            c_llVolumeSold_data->Append(0ULL);
            c_llVolumeSold_nulls->Append(1);
        }

        // Nullable Strings (empty => NULL)
        const auto sAggEx = Utils::toString(t.sAggressorExchOrdId);
        const auto sAggSide = Utils::toString(t.sAggressorSide);
        const auto sCond = Utils::toString(t.sCondition);
        const auto sExOrdId = Utils::toString(t.sExchOrdId);

        if (!sAggEx.empty()) {
            c_sAggressorExchOrdId_data->Append(sAggEx);
            c_sAggressorExchOrdId_nulls->Append(0);
        }
        else {
            c_sAggressorExchOrdId_data->Append(std::string());
            c_sAggressorExchOrdId_nulls->Append(1);
        }

        if (!sAggSide.empty()) {
            c_sAggressorSide_data->Append(sAggSide);
            c_sAggressorSide_nulls->Append(0);
        }
        else {
            c_sAggressorSide_data->Append(std::string());
            c_sAggressorSide_nulls->Append(1);
        }

        if (!sCond.empty()) {
            c_sCondition_data->Append(sCond);
            c_sCondition_nulls->Append(0);
        }
        else {
            c_sCondition_data->Append(std::string());
            c_sCondition_nulls->Append(1);
        }

        if (!sExOrdId.empty()) {
            c_sExchOrdId_data->Append(sExOrdId);
            c_sExchOrdId_nulls->Append(0);
        }
        else {
            c_sExchOrdId_data->Append(std::string());
            c_sExchOrdId_nulls->Append(1);
        }
    }

    // ---------- Append to Block using your helper for non-nullable ----------
    ClickhouseUtils::appendColumnToBlock(block, "sExchange", v_sExchange);
    ClickhouseUtils::appendColumnToBlock(block, "sTicker", v_sTicker);
    ClickhouseUtils::appendColumnToBlock(block, "dPrice", v_dPrice);

    ClickhouseUtils::appendColumnToBlock(block, "bNetChangeFlag", v_bNetChangeFlag);
    ClickhouseUtils::appendColumnToBlock(block, "bPercentChangeFlag", v_bPercentChangeFlag);
    ClickhouseUtils::appendColumnToBlock(block, "bPriceFlag", v_bPriceFlag);
    ClickhouseUtils::appendColumnToBlock(block, "bVolumeBoughtFlag", v_bVolumeBoughtFlag);
    ClickhouseUtils::appendColumnToBlock(block, "bVolumeSoldFlag", v_bVolumeSoldFlag);
    ClickhouseUtils::appendColumnToBlock(block, "bVwapFlag", v_bVwapFlag);
    ClickhouseUtils::appendColumnToBlock(block, "bVwapLongFlag", v_bVwapLongFlag);

    ClickhouseUtils::appendColumnToBlock(block, "iJopSsboe", v_iJopSsboe);
    ClickhouseUtils::appendColumnToBlock(block, "iJopNsecs", v_iJopNsecs);
    ClickhouseUtils::appendColumnToBlock(block, "iSourceSsboe", v_iSourceSsboe);
    ClickhouseUtils::appendColumnToBlock(block, "iSourceUsecs", v_iSourceUsecs);
    ClickhouseUtils::appendColumnToBlock(block, "iSourceNsecs", v_iSourceNsecs);
    ClickhouseUtils::appendColumnToBlock(block, "iSsboe", v_iSsboe);
    ClickhouseUtils::appendColumnToBlock(block, "iUsecs", v_iUsecs);

    ClickhouseUtils::appendColumnToBlock(block, "llSize", v_llSize);

    // ---------- Append Nullable columns as pairs (data,nullmap) ----------
    ClickhouseUtils::appendColumnToBlock(block, "dNetChange", std::make_pair(c_dNetChange_data, c_dNetChange_nulls));
    ClickhouseUtils::appendColumnToBlock(block, "dPercentChange", std::make_pair(c_dPercentChange_data, c_dPercentChange_nulls));
    ClickhouseUtils::appendColumnToBlock(block, "dVwap", std::make_pair(c_dVwap_data, c_dVwap_nulls));
    ClickhouseUtils::appendColumnToBlock(block, "dVwapLong", std::make_pair(c_dVwapLong_data, c_dVwapLong_nulls));

    ClickhouseUtils::appendColumnToBlock(block, "llVolumeBought", std::make_pair(c_llVolumeBought_data, c_llVolumeBought_nulls));
    ClickhouseUtils::appendColumnToBlock(block, "llVolumeSold", std::make_pair(c_llVolumeSold_data, c_llVolumeSold_nulls));

    ClickhouseUtils::appendColumnToBlock(block, "sAggressorExchOrdId", std::make_pair(c_sAggressorExchOrdId_data, c_sAggressorExchOrdId_nulls));
    ClickhouseUtils::appendColumnToBlock(block, "sAggressorSide", std::make_pair(c_sAggressorSide_data, c_sAggressorSide_nulls));
    ClickhouseUtils::appendColumnToBlock(block, "sCondition", std::make_pair(c_sCondition_data, c_sCondition_nulls));
    ClickhouseUtils::appendColumnToBlock(block, "sExchOrdId", std::make_pair(c_sExchOrdId_data, c_sExchOrdId_nulls));

    return block;
}
