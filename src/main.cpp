#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <vector>

#include "transaction/TransactionManager.h"
#include "concurrency/LockManager.h"
#include "recovery/LogManager.h"
#include "recovery/RecoveryManager.h"
#include "storage/DataStore.h"

namespace {

struct Command {
    std::string op;
    std::string txn_label;
    std::string resource_name;
    int value = 0;
};

std::vector<uint8_t> intToBytes(int value) {
    const std::string text = std::to_string(value);
    return std::vector<uint8_t>(text.begin(), text.end());
}

int bytesToInt(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return 0;
    }
    const std::string text(data.begin(), data.end());
    return std::stoi(text);
}

}  // namespace

int main() {
    TransactionManager tm;
    LockManager lock_mgr;
    auto log_mgr = std::make_shared<LogManager>("transaction.log");
    DataStore data_store;

    std::map<std::string, uint32_t> txn_ids;
    std::map<std::string, uint32_t> resource_ids;
    std::map<uint32_t, std::set<uint32_t>> txn_held_resources;
    std::vector<Command> pending;

    uint32_t next_resource_id = 1;

    auto printHelp = [&]() {
        std::cout << "Commands: BEGIN <TXN>, READ <TXN> <KEY>, WRITE <TXN> <KEY> <VAL>, COMMIT <TXN>, ABORT <TXN>, SHOW <KEY>, STATUS, CHECKPOINT, RECOVER, HELP, EXIT" << '\n';
    };

    auto ensureResource = [&](const std::string& resource_name) {
        std::stringstream parts(resource_name);
        std::string segment;
        std::string path;
        uint32_t last_id = 0;

        while (std::getline(parts, segment, '.')) {
            if (segment.empty()) {
                continue;
            }

            path = path.empty() ? segment : path + "." + segment;
            auto it = resource_ids.find(path);
            if (it != resource_ids.end()) {
                last_id = it->second;
                continue;
            }

            const uint32_t resource_id = next_resource_id++;
            resource_ids[path] = resource_id;

            if (last_id != 0) {
                lock_mgr.setResourceParent(resource_id, last_id);
            }

            auto record = std::make_shared<Record>(resource_id);
            const int initial_value = (path == "A") ? 10 : 0;
            record->setData(intToBytes(initial_value));
            data_store.insertRecord(record);

            last_id = resource_id;
        }

        if (last_id == 0) {
            const uint32_t resource_id = next_resource_id++;
            resource_ids[resource_name] = resource_id;
            auto record = std::make_shared<Record>(resource_id);
            record->setData(intToBytes(0));
            data_store.insertRecord(record);
            return resource_id;
        }

        return last_id;
    };

    auto executeCommand = [&](const Command& cmd, bool from_pending) {
        auto txn_it = txn_ids.find(cmd.txn_label);
        if (txn_it == txn_ids.end()) {
            return false;
        }

        const uint32_t txn_id = txn_it->second;
        if (!tm.isTransactionActive(txn_id)) {
            return false;
        }

        if (cmd.op == "READ") {
            const uint32_t resource_id = ensureResource(cmd.resource_name);
            const bool granted = lock_mgr.requestLock(txn_id, resource_id, LockType::SHARED);
            if (!granted) {
                if (!from_pending) {
                    std::cout << cmd.txn_label << " waiting for lock on " << cmd.resource_name << '\n';
                }
                return false;
            }

            txn_held_resources[txn_id].insert(resource_id);
            auto record = data_store.getRecord(resource_id);
            const int value = record ? bytesToInt(record->getData()) : 0;
            std::cout << cmd.txn_label << " reads " << cmd.resource_name << " = " << value << '\n';
            return true;
        }

        if (cmd.op == "WRITE") {
            const uint32_t resource_id = ensureResource(cmd.resource_name);
            const bool granted = lock_mgr.requestLock(txn_id, resource_id, LockType::EXCLUSIVE);
            if (!granted) {
                if (!from_pending) {
                    std::cout << cmd.txn_label << " waiting for lock on " << cmd.resource_name << '\n';
                }
                return false;
            }

            txn_held_resources[txn_id].insert(resource_id);
            data_store.updateRecord(resource_id, intToBytes(cmd.value));
            log_mgr->writeLog(std::make_shared<LogRecord>(txn_id, LogRecord::LogType::UPDATE));
            std::cout << cmd.txn_label << " writes " << cmd.resource_name << " = " << cmd.value << '\n';
            return true;
        }

        return false;
    };

    auto hasPendingForTxn = [&](const std::string& txn_label) {
        for (const auto& command : pending) {
            if (command.txn_label == txn_label) {
                return true;
            }
        }
        return false;
    };

    auto processPending = [&]() {
        bool made_progress = true;
        std::set<std::string> resumed;

        while (made_progress) {
            made_progress = false;
            for (auto it = pending.begin(); it != pending.end();) {
                const auto txn_it = txn_ids.find(it->txn_label);
                if (txn_it == txn_ids.end() || !tm.isTransactionActive(txn_it->second)) {
                    it = pending.erase(it);
                    continue;
                }

                if (resumed.find(it->txn_label) == resumed.end() && !it->resource_name.empty()) {
                    const uint32_t resource_id = ensureResource(it->resource_name);
                    if (lock_mgr.hasLock(txn_it->second, resource_id)) {
                        std::cout << it->txn_label << " resumes" << '\n';
                        resumed.insert(it->txn_label);
                    }
                }

                const bool executed = executeCommand(*it, true);
                if (executed) {
                    it = pending.erase(it);
                    made_progress = true;
                } else {
                    ++it;
                }
            }
        }
    };

    std::ifstream file_input("input.txt");
    std::istream* input_stream = &std::cin;
    if (file_input.is_open()) {
        input_stream = &file_input;
    }

    std::string line;
    printHelp();
    while (std::getline(*input_stream, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream parser(line);
        Command cmd;
        parser >> cmd.op;

        if (cmd.op == "HELP") {
            printHelp();
            continue;
        }

        if (cmd.op == "EXIT") {
            break;
        }

        if (cmd.op == "BEGIN") {
            parser >> cmd.txn_label;
            if (cmd.txn_label.empty()) {
                std::cout << "Invalid BEGIN command" << '\n';
                continue;
            }
            if (txn_ids.find(cmd.txn_label) != txn_ids.end()) {
                std::cout << cmd.txn_label << " already exists" << '\n';
                continue;
            }

            auto txn = tm.beginTransaction();
            txn_ids[cmd.txn_label] = txn->getTransactionId();
            log_mgr->writeLog(std::make_shared<LogRecord>(txn->getTransactionId(), LogRecord::LogType::BEGIN));
            std::cout << cmd.txn_label << " started" << '\n';
            continue;
        }

        if (cmd.op == "READ") {
            parser >> cmd.txn_label >> cmd.resource_name;
            if (cmd.txn_label.empty() || cmd.resource_name.empty()) {
                std::cout << "Invalid READ command" << '\n';
                continue;
            }
            if (txn_ids.find(cmd.txn_label) == txn_ids.end()) {
                std::cout << cmd.txn_label << " not found" << '\n';
                continue;
            }

            if (hasPendingForTxn(cmd.txn_label)) {
                pending.push_back(cmd);
                continue;
            }

            if (!executeCommand(cmd, false)) {
                pending.push_back(cmd);
            }
            continue;
        }

        if (cmd.op == "WRITE") {
            parser >> cmd.txn_label >> cmd.resource_name >> cmd.value;
            if (cmd.txn_label.empty() || cmd.resource_name.empty()) {
                std::cout << "Invalid WRITE command" << '\n';
                continue;
            }
            if (txn_ids.find(cmd.txn_label) == txn_ids.end()) {
                std::cout << cmd.txn_label << " not found" << '\n';
                continue;
            }

            if (hasPendingForTxn(cmd.txn_label)) {
                pending.push_back(cmd);
                continue;
            }

            if (!executeCommand(cmd, false)) {
                pending.push_back(cmd);
            }
            continue;
        }

        if (cmd.op == "COMMIT") {
            parser >> cmd.txn_label;
            auto txn_it = txn_ids.find(cmd.txn_label);
            if (txn_it == txn_ids.end()) {
                std::cout << cmd.txn_label << " not found" << '\n';
                continue;
            }

            const uint32_t txn_id = txn_it->second;
            if (!tm.commitTransaction(txn_id)) {
                std::cout << cmd.txn_label << " is not active" << '\n';
                continue;
            }

            lock_mgr.completeTransaction(txn_id);
            txn_held_resources.erase(txn_id);

            log_mgr->writeLog(std::make_shared<LogRecord>(txn_id, LogRecord::LogType::COMMIT));
            std::cout << cmd.txn_label << " committed" << '\n';
            processPending();
            continue;
        }

        if (cmd.op == "ABORT") {
            parser >> cmd.txn_label;
            auto txn_it = txn_ids.find(cmd.txn_label);
            if (txn_it == txn_ids.end()) {
                std::cout << cmd.txn_label << " not found" << '\n';
                continue;
            }

            const uint32_t txn_id = txn_it->second;
            if (!tm.abortTransaction(txn_id)) {
                std::cout << cmd.txn_label << " is not active" << '\n';
                continue;
            }

            lock_mgr.completeTransaction(txn_id);
            txn_held_resources.erase(txn_id);

            log_mgr->writeLog(std::make_shared<LogRecord>(txn_id, LogRecord::LogType::ABORT));
            std::cout << cmd.txn_label << " aborted" << '\n';
            processPending();
            continue;
        }

        if (cmd.op == "SHOW") {
            parser >> cmd.resource_name;
            if (cmd.resource_name.empty()) {
                std::cout << "Invalid SHOW command" << '\n';
                continue;
            }

            const uint32_t resource_id = ensureResource(cmd.resource_name);
            auto record = data_store.getRecord(resource_id);
            const int value = record ? bytesToInt(record->getData()) : 0;
            std::cout << cmd.resource_name << " = " << value << '\n';
            continue;
        }

        if (cmd.op == "STATUS") {
            std::cout << "active_transactions=" << tm.getActiveTransactionCount()
                      << " pending_commands=" << pending.size()
                      << " committed=" << tm.getCommittedCount()
                      << " aborted=" << tm.getAbortedCount() << '\n';
            continue;
        }

        if (cmd.op == "CHECKPOINT") {
            log_mgr->writeLog(std::make_shared<LogRecord>(0, LogRecord::LogType::CHECKPOINT));
            log_mgr->flushLogs();
            std::cout << "checkpoint written" << '\n';
            continue;
        }

        if (cmd.op == "RECOVER") {
            auto active_ids = tm.getActiveTransactionIds();
            tm.abortAllTransactions();

            for (uint32_t txn_id : active_ids) {
                lock_mgr.completeTransaction(txn_id);
                txn_held_resources.erase(txn_id);
            }
            pending.clear();

            log_mgr->flushLogs();
            RecoveryManager recovery_mgr(log_mgr);
            recovery_mgr.performRecovery();

            const auto redo_records = recovery_mgr.getRedoRecords();
            const auto undo_records = recovery_mgr.getUndoRecords();
            std::cout << "recovery complete: redo=" << redo_records.size()
                      << " undo=" << undo_records.size() << '\n';
            continue;
        }

        std::cout << "Unknown command: " << cmd.op << '\n';
    }

    log_mgr->flushLogs();
    return 0;
}
