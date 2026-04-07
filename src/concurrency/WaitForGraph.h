#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

// Directed Wait-For Graph used for deadlock detection.
//
// An edge  A -> B  means "transaction A is waiting for transaction B to
// release at least one lock".  A cycle in this graph indicates a deadlock.
class WaitForGraph {
public:
    // Add an edge: 'waiter' waits for 'holder'
    void addEdge(int waiter, int holder);

    // Remove all edges that involve txnId (both outgoing and incoming)
    void removeEdgesForTxn(int txnId);

    // Remove only outgoing edges from txnId (txn is no longer waiting)
    void removeOutgoingEdges(int txnId);

    // True if the edge (from -> to) exists
    bool hasEdge(int from, int to) const;

    // Detect a cycle.  Returns the txnId of the chosen victim (node in the
    // cycle with the highest ID → "youngest transaction first" policy),
    // or -1 when no cycle is found.
    int detectCycle() const;

    // Print the graph to stdout (for debugging / demo)
    void print() const;

private:
    // Adjacency list: node -> set of nodes it points to
    std::unordered_map<int, std::unordered_set<int>> adjList_;

    // Recursive DFS helper.  Returns true if a cycle is detected.
    // 'victim' is set to the chosen victim txnId.
    bool dfs(int node,
             std::unordered_map<int, int>& color,
             int& victim) const;
};
