#include "../../include/storage/Record.h"
#include "../../include/utils/constants.h"
#include <stdexcept>
#include <string>

Record::Record(uint32_t record_id)
    : record_id_(record_id),
      created_time_(std::chrono::system_clock::now()),
      modified_time_(std::chrono::system_clock::now()) {
}

Record::~Record() {
}

uint32_t Record::getRecordId() const {
    return record_id_;
}

const std::vector<uint8_t>& Record::getData() const {
    return data_;
}

void Record::setData(const std::vector<uint8_t>& data) {
    if (data.size() > MAX_RECORD_SIZE) {
        throw std::invalid_argument(
            "Data size (" + std::to_string(data.size()) + " bytes) exceeds MAX_RECORD_SIZE (" +
            std::to_string(MAX_RECORD_SIZE) + " bytes)"
        );
    }
    data_ = data;
    modified_time_ = std::chrono::system_clock::now();
}

size_t Record::getSize() const {
    return data_.size();
}

std::chrono::system_clock::time_point Record::getCreatedTime() const {
    return created_time_;
}

std::chrono::system_clock::time_point Record::getModifiedTime() const {
    return modified_time_;
}
