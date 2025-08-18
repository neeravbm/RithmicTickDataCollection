//
// Created by neera on 11/10/2023.
//

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "Constants.hpp"
#include "Rithmic.hpp"
#include "Series.hpp"
#include "Utils.hpp"

#ifndef WinOS
#include <unistd.h>
#else
#include <time.h>
#endif

Rithmic::Rithmic(const std::string& instance1) : instance(instance1) {
    const std::filesystem::path filename = std::filesystem::path("..") / (".env." + instance1 + ".yaml");

    std::string line;
    std::ifstream myFile(filename, std::ifstream::in);

    int index = 1;
    while (std::getline(myFile, line)) {
        auto words = Utils::split_string(line, '=');
        if (words.size() != 2) {
            const auto error = "Line " + std::to_string(index) + " in " + filename.string() + " is not in the form key=value";
            std::cout << error << std::endl;
            throw std::exception();
        }

        config.insert(std::make_pair(words[0], words[1]));
        index++;
    }

    if (!config.contains("EXECUTION_INSTANCE")) {
        std::cerr << "Provide a value of EXECUTION_INSTANCE in " << filename.string() << std::endl;
        throw std::exception();
    }

    Utils::setString(Constants::EXECUTION_INSTANCE, config.at("EXECUTION_INSTANCE"));
}

RApi::REngine* Rithmic::login() {
    RApi::REngineParams oReParams;
    RApi::LoginParams oLoginParams;
    int iCode;

    myAdminCallbacks = new AdminCallbacks(this);
    oReParams.pAdmCallbacks = myAdminCallbacks;

    Utils::setString(oReParams.sAppName, Constants::APP_NAME);
    Utils::setString(oReParams.sAppVersion, Constants::APP_VERSION);
    Utils::setString(oReParams.sLogFilePath, Constants::LOG_FILE_PATH);

    try {
        if (config.contains("RAPI_MD_ENCODING") && config.contains("RAPI_IH_ENCODING")) {
            char* fake_envp[11];
            fake_envp[0] = strdup(("MML_DMN_SRVR_ADDR=" + config.at("MML_DMN_SRVR_ADDR")).c_str());
            fake_envp[1] = strdup(("MML_DOMAIN_NAME=" + config.at("MML_DOMAIN_NAME")).c_str());
            fake_envp[2] = strdup(("MML_LIC_SRVR_ADDR=" + config.at("MML_LIC_SRVR_ADDR")).c_str());
            fake_envp[3] = strdup(("MML_LOC_BROK_ADDR=" + config.at("MML_LOC_BROK_ADDR")).c_str());
            fake_envp[4] = strdup(("MML_LOGGER_ADDR=" + config.at("MML_LOGGER_ADDR")).c_str());
            fake_envp[5] = strdup(("MML_LOG_TYPE=" + config.at("MML_LOG_TYPE")).c_str());
            fake_envp[6] = strdup("MML_SSL_CLNT_AUTH_FILE=rithmic_ssl_cert_auth_params");
            fake_envp[7] = strdup(("USER=" + config.at("USER")).c_str());
            fake_envp[8] = strdup(("RAPI_MD_ENCODING=" + config.at("RAPI_MD_ENCODING")).c_str());
            fake_envp[9] = strdup(("RAPI_IH_ENCODING=" + config.at("RAPI_IH_ENCODING")).c_str());
            fake_envp[10] = nullptr;
            oReParams.envp = fake_envp;
            myEngine = new RApi::REngine(&oReParams);
        }
        else {
            char* fake_envp[9];
            fake_envp[0] = strdup(("MML_DMN_SRVR_ADDR=" + config.at("MML_DMN_SRVR_ADDR")).c_str());
            fake_envp[1] = strdup(("MML_DOMAIN_NAME=" + config.at("MML_DOMAIN_NAME")).c_str());
            fake_envp[2] = strdup(("MML_LIC_SRVR_ADDR=" + config.at("MML_LIC_SRVR_ADDR")).c_str());
            fake_envp[3] = strdup(("MML_LOC_BROK_ADDR=" + config.at("MML_LOC_BROK_ADDR")).c_str());
            fake_envp[4] = strdup(("MML_LOGGER_ADDR=" + config.at("MML_LOGGER_ADDR")).c_str());
            fake_envp[5] = strdup(("MML_LOG_TYPE=" + config.at("MML_LOG_TYPE")).c_str());
            fake_envp[6] = strdup("MML_SSL_CLNT_AUTH_FILE=rithmic_ssl_cert_auth_params");
            fake_envp[7] = strdup(("USER=" + config.at("USER")).c_str());
            fake_envp[8] = nullptr;
            oReParams.envp = fake_envp;
            myEngine = new RApi::REngine(&oReParams);
        }
    }
    catch (OmneException& oEx) {
        iCode = oEx.getErrorCode();
        std::cout << "REngine::REngine() error : " << iCode << std::endl;
        std::cout << "REngine::REngine() error : " << oEx.getErrorString() << std::endl;

        throw oEx;
    }

    myCallbacks = new Callbacks(this);

    /*   ----------------------------------------------------------------   */
    /*   Set up parameters for logging in.  The MdCnnctPt and IhCnnctPt     */
    /*   have values for Rithmic Test.   Both are necessary to retrieve     */
    /*   bars.  Sometimes, users have different credentials for market      */
    /*   data and history login points, but this is a more unusual case.  */
    /*   The code below uses the same credentials for both.                 */

    oLoginParams.pCallbacks = myCallbacks;

    oLoginParams.sMdCnnctPt.pData = strdup(config.at("MD_CONNCT_PT").c_str());
    oLoginParams.sMdCnnctPt.iDataLen = (int)strlen(oLoginParams.sMdCnnctPt.pData);

    oLoginParams.sMdUser.pData = const_cast<char*>(config.at("USER").c_str());
    oLoginParams.sMdUser.iDataLen = (int)strlen(oLoginParams.sMdUser.pData);

    oLoginParams.sMdPassword.pData = const_cast<char*>(config.at("PASSWORD").c_str());
    oLoginParams.sMdPassword.iDataLen = (int)strlen(oLoginParams.sMdPassword.pData);

    oLoginParams.sIhCnnctPt.pData = strdup(config.at("IH_CONNCT_PT").c_str());
    oLoginParams.sIhCnnctPt.iDataLen = (int)strlen(oLoginParams.sIhCnnctPt.pData);

    oLoginParams.sIhUser = oLoginParams.sMdUser;
    oLoginParams.sIhPassword = oLoginParams.sMdPassword;

    oLoginParams.sTsUser.pData = const_cast<char*>(config.at("USER").c_str());
    oLoginParams.sTsUser.iDataLen = (int)strlen(oLoginParams.sTsUser.pData);

    oLoginParams.sTsPassword.pData = const_cast<char*>(config.at("PASSWORD").c_str());
    oLoginParams.sTsPassword.iDataLen = (int)strlen(oLoginParams.sTsPassword.pData);

    oLoginParams.sTsCnnctPt.pData = strdup(config.at("TS_CONNCT_PT").c_str());
    oLoginParams.sTsCnnctPt.iDataLen = (int)strlen(oLoginParams.sTsCnnctPt.pData);

    /*   ----------------------------------------------------------------   */

    iMdLoginStatus = NOT_LOGGED_IN;
    iIhLoginStatus = NOT_LOGGED_IN;
    iTsLoginStatus = NOT_LOGGED_IN;

    if (!myEngine->login(&oLoginParams, &iCode)) {
        std::cout << "REngine::login() error : " << iCode << std::endl;

        delete myEngine;

        throw std::exception();
    }

    /*   ----------------------------------------------------------------   */
    /*   After calling REngine::login, RCallbacks::Alert may be called a    */
    /*   number of times.  Wait for when the login to the MdCnnctPt is      */
    /*   complete.  (See MyCallbacks::Alert() for details).                 */

    while (iMdLoginStatus != COMPLETE && iIhLoginStatus != COMPLETE && iTsLoginStatus != COMPLETE) {
        if (iMdLoginStatus == FAILED || iIhLoginStatus == FAILED || iTsLoginStatus == FAILED) {
            std::cout << "Login failed" << std::endl;
            throw std::exception();
        }

        Utils::sleepFor(1);
    }

    while (!bRcvdAccount) {
        Utils::sleepFor(1);
    }

    return myEngine;
}

void Rithmic::getHistoricalTrades(const AssetIdentifier& assetIdentifier, const int historicalBarsToFetchInSeconds) const {
    /*RApi::ReplayBarParams rbParams;

    rbParams.sExchange = std::get<0>(assetIdentifier);
    rbParams.sTicker = std::get<1>(assetIdentifier);

    rbParams.iType = RApi::BAR_TYPE_TICK;
    rbParams.iSpecifiedTicks = 1;

    rbParams.iEndSsboe = (int)time(NULL);
    rbParams.iEndUsecs = 0;

    rbParams.iStartSsboe = rbParams.iEndSsboe - historicalBarsToFetchInSeconds;
    rbParams.iStartUsecs = 0;

    const std::chrono::zoned_time ztStart{"America/New_York", std::chrono::sys_seconds(std::chrono::seconds(rbParams.iStartSsboe))};
    const std::chrono::zoned_time ztEnd{"America/New_York", std::chrono::sys_seconds(std::chrono::seconds(rbParams.iEndSsboe))};

    std::cout << "Requesting " << historicalBarsToFetchInSeconds << " seconds of bars of " << Utils::toString(rbParams.sTicker) << " between " << ztStart << " and " << ztEnd << std::endl;

    int code;
    if (!myEngine->replayBars(&rbParams, &code)) {
        std::cout << "REngine::replayBars() error : " << code << std::endl;
        throw std::exception();
    }*/
}

void Rithmic::getHistoricalTrades(const int historicalBarsToFetchInSeconds) const {
    /*int code;

    for (const auto& price : prices) {
        getHistoricalTrades(price.first, historicalBarsToFetchInSeconds);
    }*/
}

void Rithmic::subscribeToRealTimeTradesAndQuotes() const {
    int code;

    for (const auto [exchange, ticker] : assetIdentifiers) {
        auto exchange1 = exchange;
        auto ticker1 = ticker;
        if (!myEngine->subscribe(&exchange1, &ticker1, RApi::MD_QUOTES | RApi::MD_PRINTS, &code)) {
            std::cout << "REngine::subscribe() error : " << code << std::endl;
            throw std::exception();
        }
    }
}

void Rithmic::unsubscribeToRealTimeTradesAndQuotes() const {
    int code;

    for (const auto [exchange, ticker] : assetIdentifiers) {
        auto exchange1 = exchange;
        auto ticker1 = ticker;
        if (!myEngine->unsubscribe(&exchange1, &ticker1, &code)) {
            std::cout << "REngine::unsubscribe() error : " << code << std::endl;
            throw std::exception();
        }
    }
}

void Rithmic::subscribeToHistoricalTrades() const {
    int code;

    for (const auto [exchange, ticker] : assetIdentifiers) {
        auto exchange1 = exchange;
        auto ticker1 = ticker;
        if (!myEngine->replayTrades(&exchange1, &ticker1, 0, 0, &code)) {
            std::cout << "REngine::replayTrades() error : " << code << std::endl;
            throw std::exception();
        }
    }
}

void Rithmic::unsubscribeToRealTimeTrades() const {
    int code;

    for (const auto [exchange, ticker] : assetIdentifiers) {
        auto exchange1 = exchange;
        auto ticker1 = ticker;
        if (!myEngine->unsubscribe(&exchange1, &ticker1, &code)) {
            std::cout << "REngine::unsubscribe() error : " << code << std::endl;
            throw std::exception();
        }
    }
}

void Rithmic::getTradeRoutes() const {
    int code;
    if (!myEngine->listTradeRoutes(nullptr, &code)) {
        std::cout << "REngine::listTradeRoutes() error :" << code << std::endl;
        throw std::exception();
    }

    while (!bRcvdTradeRoutes) {
        Utils::sleepFor(1);
    }
}

void Rithmic::setAccountInfo(const RApi::AccountInfo& accountInfo) {
    Utils::setString(Constants::ACCOUNT_INFO.sFcmId, accountInfo.sFcmId);
    Utils::setString(Constants::ACCOUNT_INFO.sAccountId, accountInfo.sAccountId);
    Utils::setString(Constants::ACCOUNT_INFO.sIbId, accountInfo.sIbId);
}

void
Rithmic::subscribe(const tsNCharcb& exchange, const std::string& ticker) {
    tsNCharcb tickerStr;
    Utils::setString(tickerStr, ticker);
    assetIdentifiers.emplace_back(exchange, tickerStr);
}

Rithmic::Callbacks::Callbacks(Rithmic* parent) : parent(parent) {}

int Rithmic::Callbacks::Alert(RApi::AlertInfo* pInfo, void* pContext, int* aiCode) {
    printf("Alert : %*.*s\n", pInfo->sMessage.iDataLen, pInfo->sMessage.iDataLen, pInfo->sMessage.pData);

    /*   ----------------------------------------------------------------   */
    /*   Signal when the login to the market data system (ticker plant)     */
    /*   is complete, and what the results are.                             */

    switch (pInfo->iConnectionId) {
    case RApi::MARKET_DATA_CONNECTION_ID:
        switch (pInfo->iAlertType) {
        case RApi::ALERT_LOGIN_COMPLETE:
            parent->iMdLoginStatus = COMPLETE;
            break;
        case RApi::ALERT_LOGIN_FAILED:
            parent->iMdLoginStatus = FAILED;
            break;
        default:
            break;
        }
        break;
    case RApi::INTRADAY_HISTORY_CONNECTION_ID:
        switch (pInfo->iAlertType) {
        case RApi::ALERT_LOGIN_COMPLETE:
            parent->iIhLoginStatus = COMPLETE;
            break;
        case RApi::ALERT_LOGIN_FAILED:
            parent->iIhLoginStatus = FAILED;
            break;
        default:
            break;
        }
        break;
    case RApi::TRADING_SYSTEM_CONNECTION_ID:
        switch (pInfo->iAlertType) {
        case RApi::ALERT_LOGIN_COMPLETE:
            parent->iTsLoginStatus = COMPLETE;
            break;
        case RApi::ALERT_LOGIN_FAILED:
            parent->iTsLoginStatus = FAILED;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    /*if (parent->iMdLoginStatus == COMPLETE && parent->iIhLoginStatus == COMPLETE) {
        parent->subscribeToRealTimeTicksAndQuotes("ESZ3");
        parent->getHistoricalTrades("ESZ3");
    }*/

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::AskQuote(RApi::AskInfo* pInfo, void* pContext, int* aiCode) {
    std::cout << "AskQuote called" << std::endl;

    int iIgnored;
    if (!pInfo -> dump(&iIgnored)) {
        std::cout << "error in pInfo -> dump : " << iIgnored << std::endl;
    }

    return (OK);
}

int Rithmic::Callbacks::Bar(RApi::BarInfo* pInfo, void* pContext, int* aiCode) {
    std::cout << "Bar called" << std::endl;

    /*   ----------------------------------------------------------------   */

    /*int iIgnored;
    if (parent->prices[BarType(pInfo->sExchange, pInfo->sTicker, pInfo->iType, pInfo->iType == RApi::BAR_TYPE_MINUTE ? pInfo->iSpecifiedMinutes : pInfo->iSpecifiedSeconds)]->barsReplayed && !pInfo -> dump(&iIgnored)) {
        std::cout << "error in pInfo -> dump : " << iIgnored << std::endl;
    }*/
    //std::cout << "Close price: " << pInfo->dClosePrice << std::endl;
    //std::cout << pInfo->sTicker.pData << " - " << std::this_thread::get_id() << ": " << pInfo->iStartSsboe << std::endl;

    /*const int period = pInfo->iType == RApi::BAR_TYPE_MINUTE ? pInfo->iSpecifiedMinutes : pInfo->iSpecifiedSeconds;
    const BarType bt(pInfo->sExchange, pInfo->sTicker, pInfo->iType, period);

    if (const auto &priceData = parent->prices[bt]; !priceData->barsReplayed) [[unlikely]] {
        if (!priceData->bars.contains(pInfo->iStartSsboe)) {
            OHLCPrice p = {
                true,
                pInfo->iStartSsboe,
                pInfo->dOpenPrice,
                pInfo->dHighPrice,
                pInfo->dLowPrice,
                pInfo->dClosePrice
            };

            // p will be copied but since this is done only during the initialization, loss of performance is fine.
            priceData->bars.insert(std::make_pair(pInfo->iStartSsboe, p));
        }
    }
    else [[likely]] {
        priceData->fillPriceArray(false, pInfo->iStartSsboe, pInfo->dOpenPrice, pInfo->dHighPrice, pInfo->dLowPrice, pInfo->dClosePrice);
        parent->executeDependents(bt);
    }

    parent->historicalBarsDownloaded++;
    if (parent->historicalBarsStartSecs < 0) parent->historicalBarsStartSecs = pInfo->iStartSsboe;
    parent->historicalBarsEndSecs = pInfo->iEndSsboe;

    *aiCode = API_OK;*/
    return (OK);
}

int Rithmic::Callbacks::BarReplay(RApi::BarReplayInfo* pInfo, void* pContext, int* aiCode) {
    //std::cout << std::endl;

    //std::cout << "BarReplay: " << pInfo->sTicker.pData << " - " << std::this_thread::get_id() << ": " << pInfo->iStartSsboe << std::endl;

    /*   ----------------------------------------------------------------   */

    /*int iIgnored;
    if (!pInfo -> dump(&iIgnored)) {
        std::cout << "error in pInfo -> dump : " << iIgnored << std::endl;
    }*/

    /*   ----------------------------------------------------------------   */

    /*const int period = pInfo->iType == RApi::BAR_TYPE_MINUTE ? pInfo->iSpecifiedMinutes : pInfo->iSpecifiedSeconds;
    const BarType bt(pInfo->sExchange, pInfo->sTicker, pInfo->iType, period);
    const auto priceData = parent->prices.at(bt);

    const std::chrono::zoned_time ztStart{"America/New_York", std::chrono::sys_seconds(std::chrono::seconds(parent->historicalBarsStartSecs))};
    const std::chrono::zoned_time ztEnd{"America/New_York", std::chrono::sys_seconds(std::chrono::seconds(parent->historicalBarsEndSecs))};

    std::cout << parent->historicalBarsDownloaded << " historical bars between " << ztStart << " and " << ztEnd << " fetched for " << Utils::toString(pInfo->sTicker) << std::endl;

    if (parent->historicalBarsDownloaded >= 10000) {
        // We may have more bars remaining.
        // Count seconds between endTime and current time.
        parent->historicalBarsDownloaded = 0;
        parent->historicalBarsStartSecs = -1;
        const auto numSeconds = static_cast<long long>(std::time(nullptr)) - static_cast<long long>(parent->historicalBarsEndSecs);
        parent->historicalBarsEndSecs = -1;
        parent->getHistoricalTicksAndQuotes(bt, numSeconds + 100); // Add 100 as overlap buffer just in case.
    }
    else {
        std::cout << "All " << priceData->bars.size() << " historical bars have been downloaded" << std::endl;

        for (const auto &[_, p] : priceData->bars) {
            priceData->fillPriceArray(true, p.startTime, p.open, p.high, p.low, p.close);
        }

        const std::chrono::zoned_time lastDownloaded{"America/New_York", std::chrono::sys_seconds(std::chrono::seconds(priceData->priceSeries.get(0).startTime))};
        std::cout << "Last downloaded historical bar for " << Utils::toString(pInfo->sTicker) << " is at " << lastDownloaded << std::endl;

        priceData->barsReplayed = true;
        priceData->bars.clear();

        parent->executeDependents(bt);
    }


    *aiCode = API_OK;*/
    return (OK);
}

int Rithmic::Callbacks::BidQuote(RApi::BidInfo* pInfo, void* pContext, int* aiCode) {
    std::cout << "BidQuote called" << std::endl;

    int iIgnored;
    if (!pInfo -> dump(&iIgnored)) {
        std::cout << "error in pInfo -> dump : " << iIgnored << std::endl;
    }

    return (OK);
}

int Rithmic::Callbacks::EndQuote(RApi::EndQuoteInfo* pInfo, void* pContext, int* aiCode) {
    std::cout << "EndQuote called" << std::endl;

    int iIgnored;
    if (!pInfo -> dump(&iIgnored)) {
        std::cout << "error in pInfo -> dump : " << iIgnored << std::endl;
    }

    return (OK);
}

int Rithmic::Callbacks::PriceIncrUpdate(RApi::PriceIncrInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\n");
    if (!pInfo->dump(&iIgnored)) {
        printf("error in pInfo -> dump : %d", iIgnored);
    }

    parent->bRcvdPriceIncr = true;

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::AccountList(RApi::AccountListInfo* pInfo, void* pContext, int* aiCode) {
    int iCode;
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\n");
    if (!pInfo->dump(&iIgnored)) {
        printf("error in pInfo -> dump : %d", iIgnored);
    }

    /*   ----------------------------------------------------------------   */

    if (pInfo->iArrayLen > 0) {
        /* copy the first account */
        if (const auto pAccount = &pInfo->asAccountInfoArray[0]; (pAccount->sAccountId.iDataLen > Constants::MAX_LEN) || (pAccount->sFcmId.iDataLen > Constants::MAX_LEN) || (pAccount->sIbId.iDataLen >
            Constants::MAX_LEN)) {
            printf("One or more of the char arrays is too small.\n");
        }
        else {
            setAccountInfo(pInfo->asAccountInfoArray[0]);

            if (!parent->myEngine->subscribeOrder(&Constants::ACCOUNT_INFO, &iCode)) {
                printf("REngine::subscribeOrder() error : %d\n", iCode);
            }

            parent->bRcvdAccount = true;
        }
    }

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::LineUpdate(RApi::LineInfo* pInfo, void* pContext, int* aiCode) {
    tsNCharcb sOrderSentToExch = {
        (char*)"order sent to exch",
        (int)strlen("order sent to exch")
    };
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    /*printf("\n\n");
    if (!pInfo -> dump(&iIgnored)) {
        printf("error in pInfo -> dump : %d", iIgnored);
    }*/

    /*   ----------------------------------------------------------------   */
    /*   record when the order was sent to the exchange... */

    /*if (pInfo -> sStatus.iDataLen == sOrderSentToExch.iDataLen &&
        memcmp(pInfo -> sStatus.pData,
               sOrderSentToExch.pData,
               sOrderSentToExch.iDataLen) == 0) {
        g_iToExchSsboe = pInfo -> iSsboe;
        g_iToExchUsecs = pInfo -> iUsecs;
    }*/

    /*   ----------------------------------------------------------------   */
    /*   if there's a completion reason, the order is complete... */

    /*if (pInfo -> sCompletionReason.pData) {
        g_bDone = true;
    }*/

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::TradeRouteList(RApi::TradeRouteListInfo* pInfo, void* pContext, int* aiCode) {
    std::cout << "We are looking for a trade route for : " << Constants::ACCOUNT_INFO.sFcmId.pData << "::"
        << Constants::ACCOUNT_INFO.sIbId.pData << "::" << Constants::EXCHANGE_CME.pData << std::endl;

    tsNCharcb sFcmId;
    tsNCharcb sIbId;
    tsNCharcb sExchange;
    tsNCharcb sTradeRoute;
    tsNCharcb sStatus;

    bool tradeRouteToCMEFound = false, tradeRouteToNYMEXFound = false;
    for (int i = 0; i < pInfo->iArrayLen; i++) {
        sFcmId = pInfo->asTradeRouteInfoArray[i].sFcmId;
        sIbId = pInfo->asTradeRouteInfoArray[i].sIbId;
        sExchange = pInfo->asTradeRouteInfoArray[i].sExchange;
        sTradeRoute = pInfo->asTradeRouteInfoArray[i].sTradeRoute;
        sStatus = pInfo->asTradeRouteInfoArray[i].sStatus;

        std::cout << sFcmId.pData << "::" << sIbId.pData << "::" << sExchange.pData << "::" << sTradeRoute.pData << "::" << sStatus.pData << std::endl;

        /* use first trade route where fcm/ib/exch matches, and status is "UP" */
        if (Utils::areEqual(Constants::ACCOUNT_INFO.sFcmId, sFcmId) && Utils::areEqual(Constants::ACCOUNT_INFO.sIbId, sIbId) && Utils::areEqual(RApi::sTRADE_ROUTE_STATUS_UP, sStatus)) {
            if (!tradeRouteToCMEFound && Utils::areEqual(Constants::EXCHANGE_CME, sExchange)) {
                std::cout << "Found the trade route to CME" << std::endl;
                Utils::setString(Constants::TRADE_ROUTE_CME, sTradeRoute);
                tradeRouteToCMEFound = true;
            }
            else if (!tradeRouteToNYMEXFound && Utils::areEqual(Constants::EXCHANGE_NYMEX, sExchange)) {
                std::cout << "Found the trade route to NYMEX" << std::endl;
                Utils::setString(Constants::TRADE_ROUTE_NYMEX, sTradeRoute);
                tradeRouteToNYMEXFound = true;
            }
        }
    }

    if (!tradeRouteToCMEFound) Utils::setNull(Constants::TRADE_ROUTE_CME);
    if (!tradeRouteToNYMEXFound) Utils::setNull(Constants::TRADE_ROUTE_NYMEX);

    parent->bRcvdTradeRoutes = true;

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::BustReport(RApi::OrderBustReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Bust Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::CancelReport(RApi::OrderCancelReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Cancel Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::FailureReport(RApi::OrderFailureReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Failure Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::FillReport(RApi::OrderFillReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Fill Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    // Exit orders don't have this sTag set but exit orders should have original order numbers so the individual
    // strategies should be able to discard orders that have different original order number.
    /*if (pReport->sTag.iDataLen == 0 || Utils::areEqual(pReport->sTag, Constants::EXECUTION_INSTANCE)) [[likely]] {
        parent->reports[AssetIdentifier(pReport->sExchange, pReport->sTicker)]->filledOrders.add(new OrderFillReport(pReport));
    }*/

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::ModifyReport(RApi::OrderModifyReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Modify Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::NotCancelledReport(RApi::OrderNotCancelledReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived NotCancelled Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::NotModifiedReport(RApi::OrderNotModifiedReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived NotModified Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::RejectReport(RApi::OrderRejectReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Reject Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */
    /*   record when the order returned from the exchange... */

    /*g_iFromExchSsboe = pReport -> iSsboe;
    g_iFromExchUsecs = pReport -> iUsecs;*/

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::StatusReport(RApi::OrderStatusReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Status Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */
    /*   record when the order returned from the exchange... */

    /*g_iFromExchSsboe = pReport -> iSsboe;
    g_iFromExchUsecs = pReport -> iUsecs;*/

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::TradeCorrectReport(RApi::OrderTradeCorrectReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Trade Correct Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::TradePrint(RApi::TradeInfo* pInfo, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Trade Report\n");
    pInfo->dump(&iIgnored);

    RApi::TradeInfo t;
    // Flags

    t.bNetChangeFlag    = pInfo->bNetChangeFlag;
    t.bPercentChangeFlag= pInfo->bPercentChangeFlag;
    t.bPriceFlag        = pInfo->bPriceFlag;
    t.bVolumeBoughtFlag = pInfo->bVolumeBoughtFlag;
    t.bVolumeSoldFlag   = pInfo->bVolumeSoldFlag;
    t.bVwapFlag         = pInfo->bVwapFlag;
    t.bVwapLongFlag     = pInfo->bVwapLongFlag;

    // Numerics
    t.dNetChange   = pInfo->dNetChange;
    t.dPercentChange = pInfo->dPercentChange;
    t.dPrice       = pInfo->dPrice;
    t.dVwap        = pInfo->dVwap;
    t.dVwapLong    = pInfo->dVwapLong;

    // Misc
    t.iConnId      = pInfo->iConnId;

    // Time components
    t.iJopNsecs    = pInfo->iJopNsecs;
    t.iJopSsboe    = pInfo->iJopSsboe;

    t.iSourceNsecs = pInfo->iSourceNsecs;
    t.iSourceSsboe = pInfo->iSourceSsboe;
    t.iSourceUsecs = pInfo->iSourceUsecs;

    t.iSsboe       = pInfo->iSsboe;
    t.iType        = pInfo->iType;
    t.iUsecs       = pInfo->iUsecs;

    // Sizes / volumes
    t.llSize         = pInfo->llSize;
    t.llVolumeBought = pInfo->llVolumeBought;
    t.llVolumeSold   = pInfo->llVolumeSold;

    // Strings / ids
    t.sAggressorExchOrdId = *Utils::cloneString(pInfo->sAggressorExchOrdId);
    t.sAggressorSide      = *Utils::cloneString(pInfo->sAggressorSide);
    t.sCondition          = *Utils::cloneString(pInfo->sCondition);
    t.sExchange           = *Utils::cloneString(pInfo->sExchange);
    t.sExchOrdId          = *Utils::cloneString(pInfo->sExchOrdId);
    t.sTicker             = *Utils::cloneString(pInfo->sTicker);

    // Handoff to uploader without locking
    if (!parent->g_tradeQueue.try_push(t)) {
        parent->g_dropped.fetch_add(1, std::memory_order_relaxed);
        // Optionally: log every N drops
    }

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::TriggerPulledReport(RApi::OrderTriggerPulledReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Trigger Pulled Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::TriggerReport(RApi::OrderTriggerReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Trigger Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

int Rithmic::Callbacks::OtherReport(RApi::OrderReport* pReport, void* pContext, int* aiCode) {
    int iIgnored;

    /*   ----------------------------------------------------------------   */

    printf("\n\nReceived Other Report\n");
    pReport->dump(&iIgnored);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}

Rithmic::AdminCallbacks::AdminCallbacks(Rithmic* parent) : parent(parent) {}

int Rithmic::AdminCallbacks::Alert(RApi::AlertInfo* pInfo, void* pContext, int* aiCode) {
    printf("AdmAlert : %*.*s\n", pInfo->sMessage.iDataLen, pInfo->sMessage.iDataLen, pInfo->sMessage.pData);

    /*   ----------------------------------------------------------------   */

    *aiCode = API_OK;
    return (OK);
}