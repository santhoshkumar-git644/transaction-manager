#ifndef WAIT_FOR_GRAPH_H
#define WAIT_FOR_GRAPH_H

#include <map>
#include <set>
#include <cstdint>

class WaitForGraph {
public:
    WaitForGraph();
    ~WaitForGraph();
    
    void addEdge(uint32_t from_txn, uint32_t to_txn);
    void removeEdge(uint32_t from_txn, uint32_t to_txn);
    bool hasCycle() const;
    
    // TODO: Deadlock detection methods
    
private:
    std::map<uint32_t, std::set<uint32_t>> graph_;
    bool detectCycleDFS(uint32_t node, std::map<uint32_t, int>& state) const;
};

#endif // WAIT_FOR_GRAPH_H
