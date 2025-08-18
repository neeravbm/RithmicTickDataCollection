//
// Created by neera on 11/10/2023.
//

#include <algorithm>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iostream>

#include "Utils.hpp"

#ifdef WinOS
#include <Windows.h>
#else
#include <libgen.h>
#include <unistd.h> // Required for sleep() on Linux
#endif

std::vector<std::string> Utils::split_string(const std::string &str, char delimiter) {
    std::vector<std::string> substrs;
    std::vector<std::string> words;
    std::stringstream ss(str);
    while (ss.good()) {
        std::string substr;
        std::getline(ss, substr, delimiter);
        substrs.push_back(substr);
    }
    return substrs;
}

void Utils::convertUnixTimestampToDateTime(int unixTimestamp, const std::string &tz) {
    std::chrono::sys_time<std::chrono::seconds> tp{std::chrono::seconds{unixTimestamp}};
    auto zt = std::chrono::zoned_time{tz, tp};
    auto lt = zt.get_local_time();
    auto h = floor<std::chrono::hours>(lt);
    auto m = floor<std::chrono::minutes>(lt);
    auto midnight = floor<std::chrono::days>(lt);
    /*auto h = std::chrono::duration_cast<std::chrono::hours>(lt);
    auto m = std::chrono::duration_cast<std::chrono::minutes>(lt);*/
    auto durationSinceMidnight = m - midnight;

    using namespace std::chrono_literals;
    std::cout << (durationSinceMidnight >= 570min && durationSinceMidnight <= 974min) << std::endl;
}

bool Utils::isWithinRegularSession(int unixTimestamp) {
    std::chrono::sys_time<std::chrono::seconds> tp{std::chrono::seconds{unixTimestamp}};
    auto zt = std::chrono::zoned_time{"America/New_York", tp};
    auto lt = zt.get_local_time();
    auto h = floor<std::chrono::hours>(lt);
    auto m = floor<std::chrono::minutes>(lt);
    auto midnight = floor<std::chrono::days>(lt);
    auto durationSinceMidnight = m - midnight;

    using namespace std::chrono_literals;
    return (durationSinceMidnight >= 570min && durationSinceMidnight <= 974min);
}

void Utils::setString(tsNCharcb &str, std::string val) {
    // TODO: Delete the allocated char arrays in destructor.
    auto c = new char[val.length()];
    strcpy(c, val.data());

    str.pData = c;
    str.iDataLen = val.length();
}

void Utils::setString(tsNCharcb &des, const tsNCharcb &src) {
    des = {strdup(src.pData), src.iDataLen};
}

std::string Utils::toString(const tsNCharcb &str) {
    std::string str1(str.pData, str.iDataLen);
    return str1;
}

tsNCharcb Utils::toString(char* str) {
    const tsNCharcb s = {str, static_cast<int>(strlen(str))};
    return s;
}

tsNCharcb Utils::toString(const char* str) {
    const tsNCharcb s = {const_cast<char*>(str), static_cast<int>(strlen(str))};
    return s;
}

tsNCharcb* Utils::cloneString(const tsNCharcb &src) {
    const auto c = new char[src.iDataLen];
    strncpy(c, src.pData, src.iDataLen);
    const auto dest = new tsNCharcb(c, src.iDataLen);
    return dest;
}

tsNCharcb Utils::getString(const std::vector<std::string> &strVec, const std::string &delimiter) {
    tsNCharcb c;
    if (strVec.empty()) {
        return c;
    }

    std::string concatenatedString;
    for (unsigned int i = 0; i < strVec.size(); i++) {
        if (i > 0) concatenatedString.append(delimiter);
        concatenatedString.append(strVec[i]);
    }

    Utils::setString(c, concatenatedString);

    return c;
}

tsNCharcb Utils::getString(const std::string& str1, const std::string& str2, const std::string& str3, const std::string& str4, const std::string& str5, const std::string& str6, const std::string& delimiter) {
    const std::string concatenatedString = str1 + delimiter + str2 + delimiter + str3 + delimiter + str4 + delimiter + str5 + delimiter + str6;
    tsNCharcb c;
    Utils::setString(c, concatenatedString);
    return c;
}

bool Utils::areEqual(const tsNCharcb &val1, const tsNCharcb &val2) {
    return val1.iDataLen == val2.iDataLen && memcmp(val1.pData, val2.pData, val1.iDataLen) == 0;
}

bool Utils::areEqual(const tsNCharcb &val1, const tsNCharcb *val2) {
    return val1.iDataLen == val2->iDataLen && memcmp(val1.pData, val2->pData, val1.iDataLen) == 0;
}

bool Utils::areEqual(const std::string &str, const tsNCharcb &val) {
    return str.length() == val.iDataLen && memcmp(str.data(), val.pData, val.iDataLen) == 0;
}

void Utils::setNull(tsNCharcb &str) {
    if (str.iDataLen > 0) {
        delete[] str.pData;
        str.pData = nullptr;
        str.iDataLen = 0;
    }
}

void Utils::sleepFor(int seconds) {
#ifndef WinOS
    sleep(seconds);
#else
    Sleep(1000 * seconds);
#endif
}

std::string Utils::getBarIdentifier(const tsNCharcb &exchange, const tsNCharcb &ticker, const int barType, const int barPeriod) {
    std::string t(exchange.pData, exchange.iDataLen);
    std::string t1(ticker.pData, ticker.iDataLen);
    t = t + "_" + t1 + "_" + std::to_string(barType) + "_" + std::to_string(barPeriod);
    return t;
}

std::string Utils::getAssetIdentifier(const tsNCharcb &exchange, const tsNCharcb &ticker) {
    std::string t(exchange.pData, exchange.iDataLen);
    std::string t1(ticker.pData, ticker.iDataLen);
    t = t + "_" + t1;
    return t;
}

std::pair<int, int> Utils::splitTime(const std::string& t) {
    auto hour_minute = Utils::split_string(t, ':');
    if (hour_minute.size() < 2) {
        throw std::invalid_argument("Time should be in HH:MM format");
    }

    auto hour = std::stoi(hour_minute[0]);
    auto minute = std::stoi(hour_minute[1]);

    return std::make_pair(hour, minute);
}

std::vector<std::pair<int, int> > Utils::convertToMinutesInDay(const std::vector<std::pair<std::string, std::string> > &t) {
    std::vector<std::pair<int, int> > tm(t.size());
    uint32_t i = 0;
    for (const auto& s : t) {
        auto startTime = Utils::splitTime(s.first);
        auto endTime = Utils::splitTime(s.second);
        tm[i] = std::make_pair(startTime.first * 60 + startTime.second, endTime.first * 60 + endTime.second);
        i++;
    }
    return tm;
}

bool Utils::isBetween(const int currentMinuteInDay, const std::vector<std::pair<int, int> > &minutesInDay) {
    return std::ranges::any_of(minutesInDay, [&currentMinuteInDay](const auto &tm) {
        return (tm.first <= tm.second && currentMinuteInDay >= tm.first && currentMinuteInDay <= tm.second) ||
               (tm.first > tm.second && (currentMinuteInDay >= tm.first || currentMinuteInDay <= tm.second));
    });
}

tsNCharcb Utils::getFileNameWithoutPathAndExtension(const std::string& filepath) {
    std::string filename = filepath;

    // Remove directory path
#ifdef _WIN32
    const char* lastBackslash = strrchr(filename.c_str(), '\\');
#else
    const char* lastBackslash = strrchr(filename.c_str(), '/');
#endif

    if (lastBackslash) {
        filename = lastBackslash + 1;
    }

    // Remove extension
    size_t lastDot = filename.rfind('.');
    if (lastDot != std::string::npos) {
        filename = filename.substr(0, lastDot);
    }

    tsNCharcb c;
    setString(c, filename);

    return c;
}
