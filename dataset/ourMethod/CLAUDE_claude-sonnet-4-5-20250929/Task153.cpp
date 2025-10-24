
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <sstream>
#include <limits>
#include <cstdint>

// Maximum limits to prevent excessive memory usage
const size_t MAX_NODES = 100000;
const size_t MAX_EDGES = 1000000;

class Graph {
private:
    // Using adjacency list representation for efficient BFS
    std::unordered_map<int, std::vector<int>> adjacencyList;
    size_t edgeCount;

public:
    Graph() : edgeCount(0) {}

    // Add edge with bounds checking to prevent resource exhaustion
    bool addEdge(int from, int to) {
        // Validate node IDs are non-negative
        if (from < 0 || to < 0) {
            return false;
        }
        
        // Check maximum edges limit to prevent memory exhaustion
        if (edgeCount >= MAX_EDGES) {
            return false;
        }
        
        // Check maximum nodes limit
        if (adjacencyList.size() >= MAX_NODES && 
            adjacencyList.find(from) == adjacencyList.end()) {
            return false;
        }
        
        adjacencyList[from].push_back(to);
        edgeCount++;
        
        // Ensure 'to' node exists in the graph even if it has no outgoing edges
        if (adjacencyList.find(to) == adjacencyList.end()) {
            if (adjacencyList.size() >= MAX_NODES) {
                return false;
            }
            adjacencyList[to] = std::vector<int>();
        }
        
        return true;
    }

    // BFS implementation with memory safety
    std::vector<int> breadthFirstSearch(int startNode) {
        std::vector<int> result;
        
        // Validate start node exists in graph
        if (adjacencyList.find(startNode) == adjacencyList.end()) {
            return result; // Return empty result for invalid start node
        }
        
        // Reserve space to prevent repeated reallocations
        result.reserve(std::min(adjacencyList.size(), MAX_NODES));
        
        std::unordered_set<int> visited;
        std::queue<int> queue;
        
        // Initialize BFS with start node
        queue.push(startNode);
        visited.insert(startNode);
        
        // Perform BFS traversal
        while (!queue.empty()) {
            int currentNode = queue.front();
            queue.pop();
            
            result.push_back(currentNode);
            
            // Get neighbors safely
            auto it = adjacencyList.find(currentNode);
            if (it != adjacencyList.end()) {
                const std::vector<int>& neighbors = it->second;
                
                // Process each neighbor with bounds checking
                for (size_t i = 0; i < neighbors.size(); ++i) {
                    int neighbor = neighbors[i];
                    
                    // Only visit unvisited nodes
                    if (visited.find(neighbor) == visited.end()) {
                        visited.insert(neighbor);
                        queue.push(neighbor);
                    }
                }
            }
        }
        
        return result;
    }
};

// Safe string to integer conversion with validation
bool safeStringToInt(const std::string& str, int& result) {
    // Validate input is not empty
    if (str.empty()) {
        return false;
    }
    
    // Check for valid integer format
    size_t pos = 0;
    if (str[0] == '-' || str[0] == '+') {
        pos = 1;
    }
    
    if (pos >= str.length()) {
        return false;
    }
    
    // Validate all characters are digits
    for (; pos < str.length(); ++pos) {
        if (!std::isdigit(static_cast<unsigned char>(str[pos]))) {
            return false;
        }
    }
    
    try {
        // Use stringstream for safe conversion
        std::istringstream iss(str);
        long long temp;
        iss >> temp;
        
        // Check for overflow
        if (temp > std::numeric_limits<int>::max() || 
            temp < std::numeric_limits<int>::min()) {
            return false;
        }
        
        result = static_cast<int>(temp);
        return true;
    } catch (...) {
        return false;
    }
}

int main() {
    // Test case 1: Simple linear graph
    {
        Graph g;
        g.addEdge(1, 2);
        g.addEdge(2, 3);
        g.addEdge(3, 4);
        
        std::vector<int> result = g.breadthFirstSearch(1);
        std::cout << "Test 1 - Linear graph BFS from node 1: ";
        for (size_t i = 0; i < result.size(); ++i) {
            std::cout << result[i];
            if (i < result.size() - 1) std::cout << " ";
        }
        std::cout << std::endl;
    }
    
    // Test case 2: Binary tree structure
    {
        Graph g;
        g.addEdge(1, 2);
        g.addEdge(1, 3);
        g.addEdge(2, 4);
        g.addEdge(2, 5);
        g.addEdge(3, 6);
        g.addEdge(3, 7);
        
        std::vector<int> result = g.breadthFirstSearch(1);
        std::cout << "Test 2 - Tree BFS from node 1: ";
        for (size_t i = 0; i < result.size(); ++i) {
            std::cout << result[i];
            if (i < result.size() - 1) std::cout << " ";
        }
        std::cout << std::endl;
    }
    
    // Test case 3: Graph with cycle
    {
        Graph g;
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(2, 0);
        g.addEdge(2, 3);
        
        std::vector<int> result = g.breadthFirstSearch(0);
        std::cout << "Test 3 - Cyclic graph BFS from node 0: ";
        for (size_t i = 0; i < result.size(); ++i) {
            std::cout << result[i];
            if (i < result.size() - 1) std::cout << " ";
        }
        std::cout << std::endl;
    }
    
    // Test case 4: Disconnected components
    {
        Graph g;
        g.addEdge(0, 1);
        g.addEdge(1, 2);
        g.addEdge(3, 4);
        g.addEdge(4, 5);
        
        std::vector<int> result = g.breadthFirstSearch(0);
        std::cout << "Test 4 - Disconnected graph BFS from node 0: ";
        for (size_t i = 0; i < result.size(); ++i) {
            std::cout << result[i];
            if (i < result.size() - 1) std::cout << " ";
        }
        std::cout << std::endl;
    }
    
    // Test case 5: Invalid start node
    {
        Graph g;
        g.addEdge(1, 2);
        g.addEdge(2, 3);
        
        std::vector<int> result = g.breadthFirstSearch(99);
        std::cout << "Test 5 - Invalid start node (99): ";
        if (result.empty()) {
            std::cout << "Empty result (expected)";
        }
        std::cout << std::endl;
    }
    
    return 0;
}
