#include "DataStore.h"

#include <iostream>

void DataStore::write(const std::string& key, const std::string& value) {
    store_[key] = Record(key, value);
}

std::optional<std::string> DataStore::read(const std::string& key) const {
    auto it = store_.find(key);
    if (it != store_.end()) {
        return it->second.value;
    }
    return std::nullopt;
}

bool DataStore::exists(const std::string& key) const {
    return store_.count(key) > 0;
}

void DataStore::remove(const std::string& key) {
    store_.erase(key);
}

void DataStore::print() const {
    if (store_.empty()) {
        std::cout << "  (empty)\n";
        return;
    }
    for (const auto& [key, record] : store_) {
        std::cout << "  " << key << " = " << record.value << "\n";
    }
}
