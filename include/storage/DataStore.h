#ifndef DATA_STORE_H
#define DATA_STORE_H

#include <map>
#include <memory>
#include <cstdint>
#include <vector>
#include "Record.h"

/**
 * @class DataStore
 * @brief In-memory data storage system for managing records.
 *
 * Provides CRUD operations (Create, Read, Update, Delete) for data records.
 * Records are stored efficiently in a hash map for O(1) lookup by ID.
 *
 * Features:
 * - Insert new records
 * - Retrieve records by ID
 * - Update record data
 * - Delete records
 * - Query statistics (count, total size)
 * - Existence checks
 *
 * Thread-safety: Currently NOT thread-safe. Requires external synchronization
 * via LockManager for concurrent access.
 */
class DataStore {
public:
    /**
     * @brief Construct a new DataStore (initially empty).
     */
    DataStore();
    ~DataStore();
    
    /**
     * @brief Insert a new record into the store.
     * @param record Shared pointer to the record to insert
     * @return true if insertion successful, false if record ID already exists
     * @note If a record with the same ID exists, insertion is rejected
     */
    bool insertRecord(std::shared_ptr<Record> record);
    
    /**
     * @brief Retrieve a record by ID.
     * @param record_id ID of the record to retrieve
     * @return Shared pointer to the record, or nullptr if not found
     */
    std::shared_ptr<Record> getRecord(uint32_t record_id) const;
    
    /**
     * @brief Check if a record exists.
     * @param record_id ID of the record to check
     * @return true if record exists, false otherwise
     */
    bool recordExists(uint32_t record_id) const;
    
    /**
     * @brief Delete a record from the store.
     * @param record_id ID of the record to delete
     * @return true if deletion successful, false if record not found
     */
    bool deleteRecord(uint32_t record_id);
    
    /**
     * @brief Update a record's data.
     * @param record_id ID of the record to update
     * @param data New data payload
     * @return true if update successful, false if record not found
     * @throws std::invalid_argument if data exceeds MAX_RECORD_SIZE
     */
    bool updateRecord(uint32_t record_id, const std::vector<uint8_t>& data);
    
    /**
     * @brief Get the number of records in the store.
     * @return Number of records
     */
    size_t getRecordCount() const;
    
    /**
     * @brief Get the total size of all records in bytes.
     * @return Total size in bytes
     */
    size_t getTotalSize() const;
    
    /**
     * @brief Get all record IDs in the store.
     * @return Vector of all record IDs
     */
    std::vector<uint32_t> getAllRecordIds() const;
    
    /**
     * @brief Clear all records from the store.
     */
    void clear();
    
private:
    std::map<uint32_t, std::shared_ptr<Record>> records_;  ///< Map of record ID -> Record
};

#endif // DATA_STORE_H
