
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>

class Task154 {
private:
    std::unordered_map<int, std::vector<int>> adjacencyList;
    
    void dfsHelper(int node, std::unordered_set<int>& visited, std::vector<int>& result) {
        if (visited.find(node) != visited.end()) {
            return;
        }
        
        visited.insert(node);
        result.push_back(node);
        
        if (adjacencyList.find(node) != adjacencyList.end()) {
            for (int neighbor : adjacencyList[node]) {
                if (visited.find(neighbor) == visited.end()) {
                    dfsHelper(neighbor, visited, result);
                }
            }
        }
    }
    
public:
    void addEdge(int source, int destination) {
        if (source < 0 || destination < 0) {
            throw std::invalid_argument("Node values must be non-negative");
        }
        
        adjacencyList[source];
        adjacencyList[destination];
        adjacencyList[source].push_back(destination);
    }
    
    std::vector<int> depthFirstSearch(int startNode) {
        if (startNode < 0) {
            throw std::invalid_argument("Start node must be non-negative");
        }
        
        std::vector<int> result;
        std::unordered_set<int> visited;
        
        if (adjacencyList.find(startNode) == adjacencyList.end()) {
            return result;
        }
        
        dfsHelper(startNode, visited, result);
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
    // Test Case 1: Simple linear graph
    std::cout << "Test Case 1: Linear Graph" << std::endl;
    Task154 graph1;
    graph1.addEdge(1, 2);
    graph1.addEdge(2, 3);
    graph1.addEdge(3, 4);
    std::cout << "DFS from node 1: ";
    printVector(graph1.depthFirstSearch(1));
    
    // Test Case 2: Graph with branches
    std::cout << "\\nTest Case 2: Branched Graph" << std::endl;
    Task154 graph2;
    graph2.addEdge(1, 2);
    graph2.addEdge(1, 3);
    graph2.addEdge(2, 4);
    graph2.addEdge(2, 5);
    graph2.addEdge(3, 6);
    std::cout << "DFS from node 1: ";
    printVector(graph2.depthFirstSearch(1));
    
    // Test Case 3: Graph with cycle
    std::cout << "\\nTest Case 3: Graph with Cycle" << std::endl;
    Task154 graph3;
    graph3.addEdge(1, 2);
    graph3.addEdge(2, 3);
    graph3.addEdge(3, 1);
    graph3.addEdge(3, 4);
    std::cout << "DFS from node 1: ";
    printVector(graph3.depthFirstSearch(1));
    
    // Test Case 4: Disconnected graph
    std::cout << "\\nTest Case 4: Disconnected Graph" << std::endl;
    Task154 graph4;
    graph4.addEdge(1, 2);
    graph4.addEdge(3, 4);
    graph4.addEdge(5, 6);
    std::cout << "DFS from node 1: ";
    printVector(graph4.depthFirstSearch(1));
    std::cout << "DFS from node 3: ";
    printVector(graph4.depthFirstSearch(3));
    
    // Test Case 5: Single node
    std::cout << "\\nTest Case 5: Single Node" << std::endl;
    Task154 graph5;
    graph5.addEdge(1, 1);
    std::cout << "DFS from node 1: ";
    printVector(graph5.depthFirstSearch(1));
    
    return 0;
}
