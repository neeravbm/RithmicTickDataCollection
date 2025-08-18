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

    ClickhouseUtils::DB_NAME = "TradingOptimizations" + env;

    Rithmic r(args::get(rithmicEnv));
    r.login();

    r.subscribe(Constants::EXCHANGE_CME, "NQU5");
    r.subscribeToHistoricalTrades();
    r.subscribeToRealTimeTradesAndQuotes();

    TradeUploader uploader(&r, /*threshold*/10000, std::chrono::minutes(1));
    uploader.start();


    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    uploader.stop();
}
