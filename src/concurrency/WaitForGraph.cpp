#include "../../include/concurrency/WaitForGraph.h"

WaitForGraph::WaitForGraph() {
}

WaitForGraph::~WaitForGraph() {
}

void WaitForGraph::addEdge(uint32_t from_txn, uint32_t to_txn) {
    graph_[from_txn].insert(to_txn);
}

void WaitForGraph::removeEdge(uint32_t from_txn, uint32_t to_txn) {
    if (graph_.find(from_txn) != graph_.end()) {
        graph_[from_txn].erase(to_txn);
    }
}

bool WaitForGraph::hasCycle() const {
    std::map<uint32_t, int> state;  // 0: unvisited, 1: visiting, 2: visited
    
    for (const auto& [node, _] : graph_) {
        if (state[node] == 0) {
            if (detectCycleDFS(node, state)) {
                return true;
            }
        }
    }
    return false;
}

bool WaitForGraph::detectCycleDFS(uint32_t node, std::map<uint32_t, int>& state) const {
    state[node] = 1;  // Mark as visiting
    
    if (graph_.find(node) != graph_.end()) {
        for (uint32_t neighbor : graph_.at(node)) {
            if (state[neighbor] == 1) {
                return true;  // Back edge found - cycle exists
            }
            if (state[neighbor] == 0) {
                if (detectCycleDFS(neighbor, state)) {
                    return true;
                }
            }
        }
    }
    
    state[node] = 2;  // Mark as visited
    return false;
}
