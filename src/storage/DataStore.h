#pragma once

#include "Record.h"

#include <optional>
#include <string>
#include <unordered_map>

// In-memory key-value datastore.
// This layer has no knowledge of transactions; concurrency control is
// enforced externally by the LockManager.
class DataStore {
public:
    // Overwrite (or insert) the value for key
    void write(const std::string& key, const std::string& value);

    // Return the value for key, or std::nullopt if the key does not exist
    std::optional<std::string> read(const std::string& key) const;

    // True if the key is present in the store
    bool exists(const std::string& key) const;

    // Remove a key from the store
    void remove(const std::string& key);

    // Print all key-value pairs to stdout (for debugging / demo)
    void print() const;

private:
    std::unordered_map<std::string, Record> store_;
};
