#ifndef RECORD_H
#define RECORD_H

#include <cstdint>
#include <vector>
#include <chrono>

/**
 * @class Record
 * @brief Represents a data record in the database.
 *
 * A Record encapsulates:
 * - Unique record ID
 * - Binary data payload
 * - Creation and modification timestamps
 * - Size tracking
 *
 * Records are immutable once created but can have their data updated.
 * All data is stored as binary to support heterogeneous data types.
 */
class Record {
public:
    /**
     * @brief Construct a new Record with the given ID.
     * @param record_id Unique identifier for this record
     */
    Record(uint32_t record_id);
    ~Record();
    
    /**
     * @brief Get the unique record ID.
     * @return Record ID (immutable)
     */
    uint32_t getRecordId() const;
    
    /**
     * @brief Get the binary data payload.
     * @return Const reference to the data vector
     */
    const std::vector<uint8_t>& getData() const;
    
    /**
     * @brief Set/update the record's data payload.
     * @param data Binary data to store
     * @throws std::invalid_argument if data exceeds MAX_RECORD_SIZE
     */
    void setData(const std::vector<uint8_t>& data);
    
    /**
     * @brief Get the size of the data payload in bytes.
     * @return Size in bytes
     */
    size_t getSize() const;
    
    /**
     * @brief Get the record creation timestamp.
     * @return System clock time point
     */
    std::chrono::system_clock::time_point getCreatedTime() const;
    
    /**
     * @brief Get the record last modification timestamp.
     * @return System clock time point
     */
    std::chrono::system_clock::time_point getModifiedTime() const;
    
private:
    uint32_t record_id_;                                    ///< Unique record identifier
    std::vector<uint8_t> data_;                             ///< Binary data payload
    std::chrono::system_clock::time_point created_time_;   ///< Creation timestamp
    std::chrono::system_clock::time_point modified_time_;  ///< Last modification timestamp
};

#endif // RECORD_H
