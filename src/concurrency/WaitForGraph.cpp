#include "WaitForGraph.h"

#include <algorithm>
#include <iostream>

// ---------------------------------------------------------------------------
// Mutators
// ---------------------------------------------------------------------------

void WaitForGraph::addEdge(int waiter, int holder) {
    adjList_[waiter].insert(holder);
    // Ensure the holder node exists in the graph even if it has no outgoing edges
    adjList_.emplace(holder, std::unordered_set<int>{});
}

void WaitForGraph::removeEdgesForTxn(int txnId) {
    // Remove outgoing edges
    adjList_.erase(txnId);
    // Remove incoming edges
    for (auto& [node, neighbors] : adjList_) {
        neighbors.erase(txnId);
    }
}

void WaitForGraph::removeOutgoingEdges(int txnId) {
    auto it = adjList_.find(txnId);
    if (it != adjList_.end()) {
        it->second.clear();
    }
}

bool WaitForGraph::hasEdge(int from, int to) const {
    auto it = adjList_.find(from);
    if (it != adjList_.end()) {
        return it->second.count(to) > 0;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Cycle detection (DFS with three-color marking)
// ---------------------------------------------------------------------------

int WaitForGraph::detectCycle() const {
    // 0 = white (unvisited), 1 = gray (in DFS stack), 2 = black (done)
    std::unordered_map<int, int> color;
    for (const auto& [node, _] : adjList_) {
        color[node] = 0;
    }

    int victim = -1;
    for (const auto& [node, _] : adjList_) {
        if (color[node] == 0) {
            if (dfs(node, color, victim)) {
                return victim;
            }
        }
    }
    return -1;
}

bool WaitForGraph::dfs(int node,
                       std::unordered_map<int, int>& color,
                       int& victim) const {
    color[node] = 1; // Mark gray (currently in stack)

    auto it = adjList_.find(node);
    if (it != adjList_.end()) {
        for (int neighbor : it->second) {
            if (color.count(neighbor) == 0 || color.at(neighbor) == 0) {
                if (dfs(neighbor, color, victim)) {
                    return true;
                }
            } else if (color.at(neighbor) == 1) {
                // Back edge → cycle found.
                // Choose the node with the higher ID as the victim
                // (youngest-transaction-first policy).
                victim = std::max(node, neighbor);
                return true;
            }
        }
    }

    color[node] = 2; // Mark black (done)
    return false;
}

// ---------------------------------------------------------------------------
// Diagnostics
// ---------------------------------------------------------------------------

void WaitForGraph::print() const {
    std::cout << "Wait-For Graph:\n";
    for (const auto& [node, neighbors] : adjList_) {
        std::cout << "  T" << node << " ->";
        for (int n : neighbors) {
            std::cout << " T" << n;
        }
        std::cout << "\n";
    }
}
