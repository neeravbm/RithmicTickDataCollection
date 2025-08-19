//
// Created by neera on 11/10/2023.
//

#pragma once


#include <unordered_map>
#include <string>

#include "SpscRingBuffer.hpp"
#include "Utils.hpp"

enum LOGIN_STATUS {
    NOT_LOGGED_IN = 0,
    AWAITING_RESULTS,
    FAILED,
    COMPLETE
};

class PriceData;
class PriceFilter;
class PriceResampler;
class OrderReports;
class OHLCPrice;
class Strategy;

class Rithmic {
private:
    class Callbacks: public RApi::RCallbacks {
    private:
        Rithmic *parent;
    public:
        explicit Callbacks(Rithmic *parent);
        //~Callbacks();

        int AccountList(RApi::AccountListInfo * pInfo, void *pContext, int *aiCode) final;
        int Alert(RApi::AlertInfo *pInfo, void *pContext, int *aiCode) final;
        int AskQuote(RApi::AskInfo* pInfo, void* pContext, int* aiCode) override;
        int Bar(RApi::BarInfo *pInfo, void *pContext, int *aiCode) final;
        int BarReplay(RApi::BarReplayInfo * pInfo, void *pContext, int *aiCode) final;
        int BestAskQuote(RApi::AskInfo* pInfo, void* pContext, int* aiCode) override;
        int BestBidAskQuote(RApi::BidInfo* pBid, RApi::AskInfo* pAsk, void* pContext, int* aiCode) override;
        int BestBidQuote(RApi::BidInfo* pInfo, void* pContext, int* aiCode) override;
        int BidQuote(RApi::BidInfo* pInfo, void* pContext, int* aiCode) override;
        int BustReport(RApi::OrderBustReport *pReport, void *pContext, int *aiCode) final;
        int CancelReport(RApi::OrderCancelReport *pReport, void *pContext, int *aiCode) final;
        int EndQuote(RApi::EndQuoteInfo* pInfo, void* pContext, int* aiCode) override;
        int FailureReport(RApi::OrderFailureReport *pReport, void *pContext, int *aiCode) final;
        int FillReport(RApi::OrderFillReport *pReport, void *pContext, int *aiCode) final;
        int LineUpdate(RApi::LineInfo *pInfo, void *pContext, int *aiCode) final;
        int ModifyReport(RApi::OrderModifyReport *pReport, void *pContext, int *aiCode) final;
        int NotCancelledReport(RApi::OrderNotCancelledReport *pReport, void *pContext, int *aiCode) final;
        int NotModifiedReport(RApi::OrderNotModifiedReport *pReport, void *pContext, int *aiCode) final;
        int OtherReport(RApi::OrderReport *pReport, void *pContext, int *aiCode) final;
        int PriceIncrUpdate(RApi::PriceIncrInfo *pInfo, void *pContext, int *aiCode) final;
        int RejectReport(RApi::OrderRejectReport *pReport, void *pContext, int *aiCode) final;
        int StatusReport(RApi::OrderStatusReport *pReport, void *pContext, int *aiCode) final;
        int TradeCorrectReport(RApi::OrderTradeCorrectReport *pReport, void *pContext, int *aiCode) final;
        int TradePrint(RApi::TradeInfo* pInfo, void* pContext, int* aiCode) override;
        int TradeRouteList(RApi::TradeRouteListInfo *pInfo, void *pContext, int *aiCode) final;
        int TriggerPulledReport(RApi::OrderTriggerPulledReport *pReport, void *pContext, int *aiCode) final;
        int TriggerReport(RApi::OrderTriggerReport *pReport, void *pContext, int *aiCode) final;
    };

    class AdminCallbacks: public RApi::AdmCallbacks {
    private:
        Rithmic *parent;
    public:
        explicit AdminCallbacks(Rithmic *parent);
        int Alert(RApi::AlertInfo *pInfo, void *pContext, int *aiCode) final;
    };

    std::string instance;
    std::unordered_map<std::string, std::string> config;
    LOGIN_STATUS iMdLoginStatus, iIhLoginStatus, iTsLoginStatus;
    bool bRcvdPriceIncr = false;
    bool bRcvdAccount = false;
    bool bRcvdTradeRoutes = false;
    unsigned int historicalBarsDownloaded = 0;
    int historicalBarsStartSecs = -1, historicalBarsEndSecs = -1;

    RApi::REngine *myEngine;
    Callbacks *myCallbacks;
    AdminCallbacks *myAdminCallbacks;

    static void setAccountInfo(const RApi::AccountInfo &accountInfo);

    std::vector<AssetIdentifier> assetIdentifiers;

public:
    explicit Rithmic(const std::string& instance1);
    RApi::REngine* login();
    void getHistoricalTrades(const AssetIdentifier& assetIdentifier, int historicalBarsToFetchInSeconds) const;
    void getHistoricalTrades(int historicalBarsToFetchInSeconds) const;
    void subscribeToRealTimeTradesAndQuotes() const;
    void unsubscribeToRealTimeTradesAndQuotes() const;
    void subscribeToHistoricalTrades() const;
    void unsubscribeToRealTimeTrades() const;
    void getTradeRoutes() const;

    void subscribe(const tsNCharcb &exchange, const std::string& ticker);

    SpscRingBuffer<RApi::TradeInfo, 1<<20> g_tradeQueue; // ~1M slots.
    std::atomic<uint64_t> g_dropped{0}; // Monitorbackpressure drops
};
