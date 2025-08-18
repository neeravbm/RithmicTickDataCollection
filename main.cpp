#include <Constants.hpp>
#include <iostream>
#include <Series.hpp>

#include "Rithmic.hpp"
#include "TradeUploader.hpp"

int main(const int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Please provide the environment name as a command-line argument" << std::endl;
        return -1;
    }

    const std::string env(argv[1]);

    TradeUploader uploader(ch, /*threshold*/10000, std::chrono::minutes(1));
    uploader.start();

    Rithmic r(env);
    r.login();

    r.subscribe(Constants::EXCHANGE_CME, "NQU5");
    r.subscribeToHistoricalTrades();
    r.subscribeToRealTimeTradesAndQuotes();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    uploader.stop();
}
