//
// Created by neera on 11/17/2023.
//

#ifndef RITHMICHFT_CONSTANTS_HPP
#define RITHMICHFT_CONSTANTS_HPP

#include "RApiPlus.h"

namespace Constants {
    constexpr tsNCharcb APP_NAME = {(char *)"neme:RithmicHFT", 15};
    constexpr tsNCharcb APP_VERSION = {(char *)"1.0.0.0", 7};
    constexpr tsNCharcb LOG_FILE_PATH = {(char *)"sb.log", 6};

    constexpr int MAX_LEN = 256; // Max length of the account id.

    constexpr unsigned int TRADES_BUFFER_LEN = 262180864;

    // Number of hours by which UTC is ahead of Eastern time currently.
    constexpr int OFFSET_HOURS = 5;

    constexpr tsNCharcb EXCHANGE_CME = {(char *)"CME", 3};
    constexpr tsNCharcb EXCHANGE_NYMEX = {(char *)"NYMEX", 5};
    constexpr tsNCharcb EXCHANGE_CBOT = {(char *)"CBOT", 4};
    constexpr tsNCharcb EXCHANGE_COMEX = {(char *)"COMEX", 5};

    inline tsNCharcb TRADE_ROUTE_CME;
    inline tsNCharcb TRADE_ROUTE_NYMEX;
    inline tsNCharcb TRADE_ROUTE_CBOT;
    inline tsNCharcb TRADE_ROUTE_COMEX;

    inline RApi::AccountInfo ACCOUNT_INFO;

    inline tsNCharcb EXECUTION_INSTANCE;

    constexpr tsNCharcb EMPTY_STRING = {(char*)"", 0};
};

#endif //RITHMICHFT_CONSTANTS_HPP
