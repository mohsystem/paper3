
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <limits>
#include <algorithm>

// Security: Using safe C++ containers and bounds checking throughout
// All inputs validated before processing

class Graph {
private:
    std::unordered_map<int, std::vector<int>> adjacencyList;
    static constexpr size_t MAX_NODES = 100000;  // Prevent excessive memory
    static constexpr size_t MAX_EDGES_PER_NODE = 10000;

public:
    // Add edge with validation
    bool addEdge(int from, int to) {
        // Security: Validate node IDs to prevent overflow/underflow
        if (from < 0 || to < 0) {
            std::cerr << "Error: Node IDs must be non-negative\\n";
            return false;
        }
        
        // Security: Limit graph size to prevent resource exhaustion
        if (adjacencyList.size() >= MAX_NODES) {
            std::cerr << "Error: Maximum node count exceeded\\n";
            return false;
        }
        
        // Security: Limit edges per node
        if (adjacencyList[from].size() >= MAX_EDGES_PER_NODE) {
            std::cerr << "Error: Maximum edges per node exceeded\\n";
            return false;
        }
        
        adjacencyList[from].push_back(to);
        return true;
    }
    
    // Perform DFS with stack overflow prevention
    std::vector<int> dfs(int startNode) {
        std::vector<int> result;
        
        // Security: Validate start node
        if (startNode < 0) {
            std::cerr << "Error: Invalid start node\\n";
            return result;
        }
        
        std::unordered_set<int> visited;
        // Security: Use iterative DFS to prevent stack overflow
        dfsIterative(startNode, visited, result);
        
        return result;
    }
    
private:
    // Iterative DFS to prevent stack overflow from deep recursion
    void dfsIterative(int start, std::unordered_set<int>& visited, 
                      std::vector<int>& result) {
        std::vector<int> stack;
        stack.push_back(start);
        
        // Security: Limit iterations to prevent infinite loops
        size_t iterations = 0;
        const size_t MAX_ITERATIONS = MAX_NODES * MAX_EDGES_PER_NODE;
        
        while (!stack.empty() && iterations++ < MAX_ITERATIONS) {
            int node = stack.back();
            stack.pop_back();
            
            if (visited.find(node) != visited.end()) {
                continue;
            }
            
            visited.insert(node);
            result.push_back(node);
            
            // Add neighbors in reverse order to maintain left-to-right traversal
            if (adjacencyList.find(node) != adjacencyList.end()) {
                const std::vector<int>& neighbors = adjacencyList[node];
                for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
                    if (visited.find(*it) == visited.end()) {
                        stack.push_back(*it);
                    }
                }
            }
        }
        
        if (iterations >= MAX_ITERATIONS) {
            std::cerr << "Warning: DFS iteration limit reached\\n";
        }
    }
};

// Security: Safe input parsing with validation
bool parseEdge(const std::string& line, int& from, int& to) {
    std::istringstream iss(line);
    
    // Security: Clear any error flags before parsing
    iss.clear();
    
    if (!(iss >> from >> to)) {
        return false;
    }
    
    // Security: Check for extra data on line
    std::string extra;
    if (iss >> extra) {
        return false;
    }
    
    return true;
}

int main() {
    // Test case 1: Simple linear graph
    {
        Graph g;
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(2, 3);
        std::vector<int> result = g.dfs(0);
        std::cout << "Test 1 - Linear graph: ";
        for (int node : result) std::cout << node << " ";
        std::cout << "\\n";
    }
    
    // Test case 2: Graph with branches
    {
        Graph g;
        g.addEdge(0, 1);
        g.addEdge(0, 2);
        g.addEdge(1, 3);
        g.addEdge(2, 4);
        std::vector<int> result = g.dfs(0);
        std::cout << "Test 2 - Branching graph: ";
        for (int node : result) std::cout << node << " ";
        std::cout << "\\n";
    }
    
    // Test case 3: Graph with cycle
    {
        Graph g;
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(2, 0);
        g.addEdge(2, 3);
        std::vector<int> result = g.dfs(0);
        std::cout << "Test 3 - Graph with cycle: ";
        for (int node : result) std::cout << node << " ";
        std::cout << "\\n";
    }
    
    // Test case 4: Disconnected node
    {
        Graph g;
        g.addEdge(0, 1);
        g.addEdge(2, 3);
        std::vector<int> result = g.dfs(0);
        std::cout << "Test 4 - Disconnected graph: ";
        for (int node : result) std::cout << node << " ";
        std::cout << "\\n";
    }
    
    // Test case 5: Single node
    {
        Graph g;
        std::vector<int> result = g.dfs(0);
        std::cout << "Test 5 - Single node: ";
        for (int node : result) std::cout << node << " ";
        std::cout << "\\n";
    }
    
    return 0;
}
