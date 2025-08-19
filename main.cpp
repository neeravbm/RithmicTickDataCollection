#include <iostream>

#include <args.hxx>

#include "Constants.hpp"
#include "Rithmic.hpp"
#include "TradeUploader.hpp"

int main(const int argc, char **argv) {
    args::ArgumentParser parser("Rithmic Tick Data Collection parser");
    args::ValueFlag<std::string> environment(parser, "env", "Clickhouse environment", {"env"}, args::Options::Required);
    args::ValueFlag<std::string> rithmicEnv(parser, "rithmic", "Rithmic environment", {'r', "rithmic"}, args::Options::Required);

    try {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Completion& e) {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Help&) {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    const auto env = args::get(environment);
    if (env != "Prod" && env != "Dev") {
        std::cout << "Clickhouse environment (env) has to be either Dev or Prod." << std::endl;
        return 2;
    }

    ClickhouseUtils::DB_NAME = "RealTimeData" + env;

    Rithmic r(args::get(rithmicEnv));
    r.login();

    r.subscribe(Constants::EXCHANGE_CBOT, "ZMU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMV5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMF6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMH6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMK6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMN6");

    r.subscribe(Constants::EXCHANGE_NYMEX, "PLQ5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PLU5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PLV5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PLX5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PLF6");

    r.subscribe(Constants::EXCHANGE_CME, "NQU5");
    r.subscribe(Constants::EXCHANGE_CME, "NQZ5");
    r.subscribe(Constants::EXCHANGE_CME, "NQH6");

    r.subscribe(Constants::EXCHANGE_CME, "ESU5");
    r.subscribe(Constants::EXCHANGE_CME, "ESZ5");
    r.subscribe(Constants::EXCHANGE_CME, "ESH6");

    r.subscribe(Constants::EXCHANGE_CME, "HEV5");
    r.subscribe(Constants::EXCHANGE_CME, "HEZ5");
    r.subscribe(Constants::EXCHANGE_CME, "HEG6");
    r.subscribe(Constants::EXCHANGE_CME, "HEJ6");
    r.subscribe(Constants::EXCHANGE_CME, "HEK6");
    r.subscribe(Constants::EXCHANGE_CME, "HEM6");
    r.subscribe(Constants::EXCHANGE_CME, "HEN6");

    r.subscribe(Constants::EXCHANGE_COMEX, "GCQ5");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCU5");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCV5");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCX5");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCZ5");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCF6");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCG6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZSU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSX5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSF6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSH6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSK6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSM6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSQ6");

    r.subscribe(Constants::EXCHANGE_CME, "6ZU5");
    r.subscribe(Constants::EXCHANGE_CME, "6ZV5");
    r.subscribe(Constants::EXCHANGE_CME, "6ZX5");
    r.subscribe(Constants::EXCHANGE_CME, "6ZZ5");
    r.subscribe(Constants::EXCHANGE_CME, "6ZF6");
    r.subscribe(Constants::EXCHANGE_CME, "6ZG6");

    r.subscribe(Constants::EXCHANGE_CME, "6LU5");
    r.subscribe(Constants::EXCHANGE_CME, "6LV5");
    r.subscribe(Constants::EXCHANGE_CME, "6LX5");
    r.subscribe(Constants::EXCHANGE_CME, "6LZ5");
    r.subscribe(Constants::EXCHANGE_CME, "6LF6");
    r.subscribe(Constants::EXCHANGE_CME, "6LG6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZTU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZTZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZTH6");

    r.subscribe(Constants::EXCHANGE_CME, "BTCQ5");
    r.subscribe(Constants::EXCHANGE_CME, "BTCU5");
    r.subscribe(Constants::EXCHANGE_CME, "BTCV5");

    r.subscribeToHistoricalTrades();
    r.subscribeToRealTimeTradesAndQuotes();

    TradeUploader uploader(&r, /*threshold*/10000, std::chrono::minutes(1));
    uploader.start();


    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    uploader.stop();
}
