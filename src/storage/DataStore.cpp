#include "../../include/storage/DataStore.h"
#include <algorithm>

DataStore::DataStore() {
}

DataStore::~DataStore() {
    clear();
}

bool DataStore::insertRecord(std::shared_ptr<Record> record) {
    if (!record) {
        return false;
    }
    
    // Check if record with this ID already exists
    if (records_.find(record->getRecordId()) != records_.end()) {
        return false;  // Record ID already exists
    }
    
    records_[record->getRecordId()] = record;
    return true;
}

std::shared_ptr<Record> DataStore::getRecord(uint32_t record_id) const {
    auto it = records_.find(record_id);
    if (it != records_.end()) {
        return it->second;
    }
    return nullptr;
}

bool DataStore::recordExists(uint32_t record_id) const {
    return records_.find(record_id) != records_.end();
}

bool DataStore::deleteRecord(uint32_t record_id) {
    auto it = records_.find(record_id);
    if (it != records_.end()) {
        records_.erase(it);
        return true;
    }
    return false;
}

bool DataStore::updateRecord(uint32_t record_id, const std::vector<uint8_t>& data) {
    auto it = records_.find(record_id);
    if (it != records_.end()) {
        it->second->setData(data);  // May throw if data too large
        return true;
    }
    return false;
}

size_t DataStore::getRecordCount() const {
    return records_.size();
}

size_t DataStore::getTotalSize() const {
    size_t total = 0;
    for (const auto& [id, record] : records_) {
        total += record->getSize();
    }
    return total;
}

std::vector<uint32_t> DataStore::getAllRecordIds() const {
    std::vector<uint32_t> ids;
    for (const auto& [id, record] : records_) {
        ids.push_back(id);
    }
    return ids;
}

void DataStore::clear() {
    records_.clear();
}
