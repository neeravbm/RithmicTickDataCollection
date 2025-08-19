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

    r.subscribe(Constants::EXCHANGE_CME, "6AU5");
    r.subscribe(Constants::EXCHANGE_CME, "6AV5");
    r.subscribe(Constants::EXCHANGE_CME, "6AX5");

    r.subscribe(Constants::EXCHANGE_CME, "6BU5");
    r.subscribe(Constants::EXCHANGE_CME, "6BV5");
    r.subscribe(Constants::EXCHANGE_CME, "6BX5");

    r.subscribe(Constants::EXCHANGE_CME, "6CU5");
    r.subscribe(Constants::EXCHANGE_CME, "6CV5");
    r.subscribe(Constants::EXCHANGE_CME, "6CX5");

    r.subscribe(Constants::EXCHANGE_CME, "6EU5");
    r.subscribe(Constants::EXCHANGE_CME, "6EV5");
    r.subscribe(Constants::EXCHANGE_CME, "6EX5");

    r.subscribe(Constants::EXCHANGE_CME, "6JU5");
    r.subscribe(Constants::EXCHANGE_CME, "6JV5");
    r.subscribe(Constants::EXCHANGE_CME, "6JX5");

    r.subscribe(Constants::EXCHANGE_CME, "6LU5");
    r.subscribe(Constants::EXCHANGE_CME, "6LV5");
    r.subscribe(Constants::EXCHANGE_CME, "6LX5");
    r.subscribe(Constants::EXCHANGE_CME, "6LZ5");
    r.subscribe(Constants::EXCHANGE_CME, "6LF6");
    r.subscribe(Constants::EXCHANGE_CME, "6LG6");

    r.subscribe(Constants::EXCHANGE_CME, "6MU5");
    r.subscribe(Constants::EXCHANGE_CME, "6MV5");
    r.subscribe(Constants::EXCHANGE_CME, "6MX5");

    r.subscribe(Constants::EXCHANGE_CME, "6NU5");
    r.subscribe(Constants::EXCHANGE_CME, "6NZ5");
    r.subscribe(Constants::EXCHANGE_CME, "6NH6");

    r.subscribe(Constants::EXCHANGE_CME, "6SU5");
    r.subscribe(Constants::EXCHANGE_CME, "6SZ5");
    r.subscribe(Constants::EXCHANGE_CME, "6SH6");

    r.subscribe(Constants::EXCHANGE_CME, "6ZU5");
    r.subscribe(Constants::EXCHANGE_CME, "6ZV5");
    r.subscribe(Constants::EXCHANGE_CME, "6ZX5");
    r.subscribe(Constants::EXCHANGE_CME, "6ZZ5");
    r.subscribe(Constants::EXCHANGE_CME, "6ZF6");
    r.subscribe(Constants::EXCHANGE_CME, "6ZG6");

    r.subscribe(Constants::EXCHANGE_CME, "BTCQ5");
    r.subscribe(Constants::EXCHANGE_CME, "BTCU5");
    r.subscribe(Constants::EXCHANGE_CME, "BTCV5");

    r.subscribe(Constants::EXCHANGE_NYMEX, "CLU5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "CLV5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "CLX5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "CLZ5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "CLF6");
    r.subscribe(Constants::EXCHANGE_NYMEX, "CLG6");

    r.subscribe(Constants::EXCHANGE_CME, "EMDU5");
    r.subscribe(Constants::EXCHANGE_CME, "EMDZ5");
    r.subscribe(Constants::EXCHANGE_CME, "EMDH6");

    r.subscribe(Constants::EXCHANGE_CME, "ESU5");
    r.subscribe(Constants::EXCHANGE_CME, "ESZ5");
    r.subscribe(Constants::EXCHANGE_CME, "ESH6");

    r.subscribe(Constants::EXCHANGE_COMEX, "GCQ5");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCU5");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCV5");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCX5");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCZ5");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCF6");
    r.subscribe(Constants::EXCHANGE_COMEX, "GCG6");

    r.subscribe(Constants::EXCHANGE_CME, "GFU5");
    r.subscribe(Constants::EXCHANGE_CME, "GFV5");
    r.subscribe(Constants::EXCHANGE_CME, "GFX5");
    r.subscribe(Constants::EXCHANGE_CME, "GFF6");
    r.subscribe(Constants::EXCHANGE_CME, "GFH6");
    r.subscribe(Constants::EXCHANGE_CME, "GFJ6");

    r.subscribe(Constants::EXCHANGE_CME, "HEV5");
    r.subscribe(Constants::EXCHANGE_CME, "HEZ5");
    r.subscribe(Constants::EXCHANGE_CME, "HEG6");
    r.subscribe(Constants::EXCHANGE_CME, "HEJ6");
    r.subscribe(Constants::EXCHANGE_CME, "HEK6");
    r.subscribe(Constants::EXCHANGE_CME, "HEM6");
    r.subscribe(Constants::EXCHANGE_CME, "HEN6");

    r.subscribe(Constants::EXCHANGE_COMEX, "HGQ5");
    r.subscribe(Constants::EXCHANGE_COMEX, "HGU5");
    r.subscribe(Constants::EXCHANGE_COMEX, "HGV5");
    r.subscribe(Constants::EXCHANGE_COMEX, "HGX5");

    r.subscribe(Constants::EXCHANGE_NYMEX, "HOU5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "HOV5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "HOX5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "HOZ5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "HOF6");
    r.subscribe(Constants::EXCHANGE_NYMEX, "HOG6");

    r.subscribe(Constants::EXCHANGE_CME, "LEQ5");
    r.subscribe(Constants::EXCHANGE_CME, "LEV5");
    r.subscribe(Constants::EXCHANGE_CME, "LEZ5");
    r.subscribe(Constants::EXCHANGE_CME, "LEG6");
    r.subscribe(Constants::EXCHANGE_CME, "LEJ6");
    r.subscribe(Constants::EXCHANGE_CME, "LEM6");
    r.subscribe(Constants::EXCHANGE_CME, "LEQ6");

    r.subscribe(Constants::EXCHANGE_NYMEX, "NGU25");
    r.subscribe(Constants::EXCHANGE_NYMEX, "NGV25");
    r.subscribe(Constants::EXCHANGE_NYMEX, "NGX25");
    r.subscribe(Constants::EXCHANGE_NYMEX, "NGZ25");
    r.subscribe(Constants::EXCHANGE_NYMEX, "NGF26");
    r.subscribe(Constants::EXCHANGE_NYMEX, "NGG26");

    r.subscribe(Constants::EXCHANGE_CME, "NQU5");
    r.subscribe(Constants::EXCHANGE_CME, "NQZ5");
    r.subscribe(Constants::EXCHANGE_CME, "NQH6");

    r.subscribe(Constants::EXCHANGE_NYMEX, "PAQ5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PAU5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PAV5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PAX5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PAZ5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PAH6");

    r.subscribe(Constants::EXCHANGE_NYMEX, "PLQ5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PLU5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PLV5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PLX5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "PLF6");

    r.subscribe(Constants::EXCHANGE_NYMEX, "RBU5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "RBV5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "RBX5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "RBZ5");
    r.subscribe(Constants::EXCHANGE_NYMEX, "RBF6");

    r.subscribe(Constants::EXCHANGE_COMEX, "SIQ5");
    r.subscribe(Constants::EXCHANGE_COMEX, "SIU5");
    r.subscribe(Constants::EXCHANGE_COMEX, "SIV5");
    r.subscribe(Constants::EXCHANGE_COMEX, "SIX5");

    r.subscribe(Constants::EXCHANGE_CBOT, "YMU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "YMZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "YMH6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZBU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZBZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZBH6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZCU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZCZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZCH6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZCK6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZCN6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZFU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZFZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZFH6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZLU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZLV5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZLZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZLF6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZLH6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZLK6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZLN6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZMU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMV5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMF6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMH6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMK6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZMN6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZNU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZNZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZNH6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZSU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSX5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSF6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSH6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSK6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSM6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZSQ6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZTU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZTZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZTH6");

    r.subscribe(Constants::EXCHANGE_CBOT, "ZWU5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZWZ5");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZWH6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZWK6");
    r.subscribe(Constants::EXCHANGE_CBOT, "ZWN6");

    r.subscribeToHistoricalTrades();
    r.subscribeToRealTimeTradesAndQuotes();

    TradeUploader uploader(&r, /*threshold*/10000, std::chrono::minutes(1));
    uploader.start();


    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    uploader.stop();
}
