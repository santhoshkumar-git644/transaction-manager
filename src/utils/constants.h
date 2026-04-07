#pragma once

namespace Constants {
    constexpr int INVALID_TXN_ID  = -1;
    constexpr int INITIAL_LSN     = 0;
    constexpr int MAX_TRANSACTIONS = 100;

    // Sentinel value used when a log record has no predecessor for its transaction
    constexpr int NO_PREV_LSN = -1;
}
