
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <algorithm>

class Task153 {
private:
    std::unordered_map<int, std::vector<int>> adjacencyList;

public:
    void addEdge(int source, int destination) {
        if (source < 0 || destination < 0) {
            throw std::invalid_argument("Node values must be non-negative");
        }
        adjacencyList[source].push_back(destination);
        if (adjacencyList.find(destination) == adjacencyList.end()) {
            adjacencyList[destination] = std::vector<int>();
        }
    }
    
    std::vector<int> breadthFirstSearch(int startNode) {
        if (startNode < 0) {
            throw std::invalid_argument("Start node must be non-negative");
        }
        
        std::vector<int> result;
        if (adjacencyList.find(startNode) == adjacencyList.end()) {
            return result;
        }
        
        std::unordered_set<int> visited;
        std::queue<int> queue;
        
        queue.push(startNode);
        visited.insert(startNode);
        
        while (!queue.empty()) {
            int currentNode = queue.front();
            queue.pop();
            result.push_back(currentNode);
            
            for (int neighbor : adjacencyList[currentNode]) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    queue.push(neighbor);
                }
            }
        }
        
        return result;
    }
};

void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test case 1: Simple linear graph
    std::cout << "Test Case 1: Linear Graph" << std::endl;
    Task153 graph1;
    graph1.addEdge(1, 2);
    graph1.addEdge(2, 3);
    graph1.addEdge(3, 4);
    std::cout << "BFS from node 1: ";
    printVector(graph1.breadthFirstSearch(1));
    
    // Test case 2: Graph with multiple branches
    std::cout << "\\nTest Case 2: Graph with Multiple Branches" << std::endl;
    Task153 graph2;
    graph2.addEdge(0, 1);
    graph2.addEdge(0, 2);
    graph2.addEdge(1, 3);
    graph2.addEdge(1, 4);
    graph2.addEdge(2, 5);
    std::cout << "BFS from node 0: ";
    printVector(graph2.breadthFirstSearch(0));
    
    // Test case 3: Disconnected graph
    std::cout << "\\nTest Case 3: Disconnected Graph" << std::endl;
    Task153 graph3;
    graph3.addEdge(0, 1);
    graph3.addEdge(2, 3);
    std::cout << "BFS from node 0: ";
    printVector(graph3.breadthFirstSearch(0));
    
    // Test case 4: Graph with cycle
    std::cout << "\\nTest Case 4: Graph with Cycle" << std::endl;
    Task153 graph4;
    graph4.addEdge(0, 1);
    graph4.addEdge(1, 2);
    graph4.addEdge(2, 0);
    graph4.addEdge(2, 3);
    std::cout << "BFS from node 0: ";
    printVector(graph4.breadthFirstSearch(0));
    
    // Test case 5: Single node
    std::cout << "\\nTest Case 5: Single Node" << std::endl;
    Task153 graph5;
    graph5.addEdge(5, 5);
    std::cout << "BFS from node 5: ";
    printVector(graph5.breadthFirstSearch(5));
    
    return 0;
}
