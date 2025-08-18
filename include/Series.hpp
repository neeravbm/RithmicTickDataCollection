//
// Created by neera on 10/10/2024.
//

#pragma once

#include <stdexcept>

#include "Constants.hpp"

// Series class template to hold a series of data points of type T
template <typename T>
class Series {
private:
    T data[Constants::TRADES_BUFFER_LEN];  // Fixed-size array to store data points
    unsigned long currentCount;  // Current number of data points in the series. This is an increasing number and stores the number of bars for which data has been stored. 0 indicates that there is no valid data, 1 indicates that there is only 1 data stored and so on.
    std::size_t head;  // Current head for circular buffer. This is the index where a new value will be added. So the latest value is at head - 1 adjusted for the length of the circular buffer.

public:
    // Constructor initializes count and index to zero
    Series() : currentCount(0), head(0) {}

    // Adds a new value to the series (circular buffer behavior)
    void add(const T& value) {
        data[head] = value;
        head = (head + 1) % Constants::TRADES_BUFFER_LEN;
        currentCount++;
    }

    // Retrieves the value at the specified lag (0 is the most recent, 1 is the previous value and so on)
    T get(const std::size_t lag) const {
        if (lag >= Constants::TRADES_BUFFER_LEN || (lag >= head && currentCount < Constants::TRADES_BUFFER_LEN)) [[unlikely]] {
            return static_cast<T>(0);
            //throw std::out_of_range("Index out of range");
        }
        std::size_t actualIndex = (head + Constants::TRADES_BUFFER_LEN - lag - 1) % Constants::TRADES_BUFFER_LEN;
        return data[actualIndex];
    }

    // Returns the number of data points or bars added.
    std::size_t count() const {
        return currentCount;
    }
};