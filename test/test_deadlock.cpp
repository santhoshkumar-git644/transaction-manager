#include <iostream>
#include "../include/concurrency/WaitForGraph.h"

int main() {
    std::cout << "Testing Deadlock Detection\n";
    
    WaitForGraph graph;
    
    // Test 1: No cycle
    graph.addEdge(1, 2);
    graph.addEdge(2, 3);
    std::cout << "Graph with no cycle - hasCycle: " << (graph.hasCycle() ? "true" : "false") << std::endl;
    
    // Test 2: With cycle
    graph.addEdge(3, 1);
    std::cout << "Graph with cycle - hasCycle: " << (graph.hasCycle() ? "true" : "false") << std::endl;
    
    // TODO: Add more comprehensive deadlock detection tests
    
    return 0;
}
