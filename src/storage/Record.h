#pragma once

#include <string>

// A key-value record stored in the DataStore
struct Record {
    std::string key;
    std::string value;

    Record() = default;
    Record(const std::string& k, const std::string& v) : key(k), value(v) {}
};
