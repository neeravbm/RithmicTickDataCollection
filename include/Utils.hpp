//
// Created by neera on 11/10/2023.
//

#ifndef RITHMICHFT_UTILS_HPP
#define RITHMICHFT_UTILS_HPP


#include <chrono>
#include <string>
#include <vector>
#include "RApiPlus.h"

/**
 * Pair of exchange and ticker.
 */
typedef std::pair<const tsNCharcb &, const tsNCharcb &> AssetIdentifier;

/**
 * Tuple of exchange, ticker, bar type and bar period.
 */
typedef std::tuple<const tsNCharcb &, const tsNCharcb &, const int, const int> BarType;

class Utils {
public:
    static std::vector<std::string> split_string(const std::string &str, char delimiter);
    static void convertUnixTimestampToDateTime(int unixTimestamp, std::string const& tz);
    static bool isWithinRegularSession(int unixTimestamp);
    static void setString(tsNCharcb &str, std::string val);
    static void setString(tsNCharcb &des, const tsNCharcb &src);
    static std::string toString(const tsNCharcb &str);
    static tsNCharcb toString(char* str);
    static tsNCharcb toString(const char* str);
    static tsNCharcb* cloneString(const tsNCharcb &src);
    static tsNCharcb getString(const std::vector<std::string> &strVec, const std::string &delimiter);
    static tsNCharcb getString(const std::string& str1, const std::string& str2, const std::string& str3, const std::string& str4, const std::string& str5, const std::string& str6, const std::string& delimiter);
    static void setNull(tsNCharcb &str);
    static bool areEqual(const tsNCharcb &val1, const tsNCharcb &val2);
    static bool areEqual(const tsNCharcb &val1, const tsNCharcb *val2);
    static bool areEqual(const std::string& str, const tsNCharcb &val);
    static void sleepFor(int seconds);

    static std::string getBarIdentifier(const tsNCharcb &exchange, const tsNCharcb &ticker, int barType, int barPeriod);
    static std::string getAssetIdentifier(const tsNCharcb &exchange, const tsNCharcb &ticker);

    static std::pair<int, int> splitTime(const std::string &t);
    static std::vector<std::pair<int, int> > convertToMinutesInDay(const std::vector<std::pair<std::string, std::string> > &t);

    static bool isBetween(int currentMinuteInDay, const std::vector<std::pair<int, int> > &minutesInDay);

    static tsNCharcb getFileNameWithoutPathAndExtension(const std::string& filepath);
};

namespace std {
    template <>
    class hash<AssetIdentifier> {
    public:
        size_t operator()(const AssetIdentifier &assetIdentifier) const {
            std::string exchange(assetIdentifier.first.pData, assetIdentifier.first.iDataLen);
            std::string ticker(assetIdentifier.second.pData, assetIdentifier.second.iDataLen);
            return std::hash<std::string>()(exchange) ^ std::hash<std::string>()(ticker);
        }
    };

    template <>
    class equal_to<AssetIdentifier> {
    public:
        bool operator()(const AssetIdentifier &lhs, const AssetIdentifier &rhs) const {
            return Utils::areEqual(lhs.first, rhs.first) && Utils::areEqual(lhs.second, rhs.second);
        }
    };

    template <>
    class hash<BarType> {
    public:
        size_t operator()(const BarType &barType) const {
            std::string exchange(std::get<0>(barType).pData, std::get<0>(barType).iDataLen);
            std::string ticker(std::get<1>(barType).pData, std::get<1>(barType).iDataLen);
            return std::hash<std::string>()(exchange) ^ std::hash<std::string>()(ticker) ^ std::hash<int>()(std::get<2>(barType)) ^ std::hash<int>()(std::get<3>(barType));
        }
    };

    template <>
    class equal_to<BarType> {
    public:
        bool operator()(const BarType &lhs, const BarType &rhs) const {
            return std::get<2>(lhs) == std::get<2>(rhs) && std::get<3>(lhs) == std::get<3>(rhs) && Utils::areEqual(std::get<0>(lhs), std::get<0>(rhs)) && Utils::areEqual(std::get<1>(lhs), std::get<1>(rhs));
        }
    };

    template <>
    class hash<const std::vector<int *>> {
    public:
        size_t operator()(const std::vector<int *>& vec) const {
            std::hash<int*> hasher;
            size_t result = 0;

            for (const auto& ptr : vec) {
                result ^= hasher(ptr) + 0x9e3779b9 + (result << 6) + (result >> 2); // Combine hashes
            }

            return result;
        }
    };

    template <>
    class equal_to<const std::vector<int *>> {
        bool operator()(const std::vector<int *> &lhs, const std::vector<int *> &rhs) const {
            if (lhs.size() != rhs.size()) {
                return false;
            }

            for (unsigned int i = 0; i < lhs.size(); i++) {
                if (*lhs[i] != *rhs[i]) {
                    return false;
                }
            }

            return true;
        }
    };
}



#endif //RITHMICHFT_UTILS_HPP
