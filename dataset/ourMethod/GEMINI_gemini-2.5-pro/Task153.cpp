#include <iostream>
#include <vector>
#include <queue>
#include <list>

class Graph {
private:
    int numVertices;
    std::vector<std::list<int>> adj;

public:
    explicit Graph(int vertices) {
        if (vertices < 0) {
            std::cerr << "Error: Number of vertices cannot be negative." << std::endl;
            numVertices = 0;
        } else {
            numVertices = vertices;
            adj.resize(numVertices);
        }
    }

    void addEdge(int src, int dest) {
        if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) {
            std::cerr << "Error: Invalid vertex for edge (" << src << ", " << dest << ")" << std::endl;
            return;
        }
        adj[src].push_back(dest);
    }

    std::vector<int> bfs(int startNode) const {
        std::vector<int> result;
        if (startNode < 0 || startNode >= numVertices) {
            std::cerr << "Error: Start node " << startNode << " is out of bounds." << std::endl;
            return result;
        }
        if (numVertices == 0) {
            return result;
        }

        std::vector<bool> visited(numVertices, false);
        std::queue<int> queue;

        visited[startNode] = true;
        queue.push(startNode);

        while (!queue.empty()) {
            int currentNode = queue.front();
            queue.pop();
            result.push_back(currentNode);

            for (int neighbor : adj[currentNode]) {
                if (neighbor >= 0 && neighbor < numVertices && !visited[neighbor]) {
                    visited[neighbor] = true;
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
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1: Simple connected graph
    std::cout << "Test Case 1: Simple connected graph" << std::endl;
    Graph g1(6);
    g1.addEdge(0, 1);
    g1.addEdge(0, 2);
    g1.addEdge(1, 3);
    g1.addEdge(2, 4);
    g1.addEdge(3, 5);
    std::cout << "BFS starting from node 0: ";
    printVector(g1.bfs(0));

    // Test Case 2: Start from a different node
    std::cout << "\nTest Case 2: Start from a different node" << std::endl;
    Graph g2(4);
    g2.addEdge(0, 1);
    g2.addEdge(0, 2);
    g2.addEdge(1, 2);
    g2.addEdge(2, 0);
    g2.addEdge(2, 3);
    g2.addEdge(3, 3);
    std::cout << "BFS starting from node 2: ";
    printVector(g2.bfs(2));

    // Test Case 3: Graph with a cycle
    std::cout << "\nTest Case 3: Graph with a cycle" << std::endl;
    Graph g3(3);
    g3.addEdge(0, 1);
    g3.addEdge(1, 2);
    g3.addEdge(2, 0);
    std::cout << "BFS starting from node 0: ";
    printVector(g3.bfs(0));

    // Test Case 4: Disconnected graph
    std::cout << "\nTest Case 4: Disconnected graph" << std::endl;
    Graph g4(5);
    g4.addEdge(0, 1);
    g4.addEdge(1, 2);
    g4.addEdge(3, 4);
    std::cout << "BFS starting from node 0: ";
    printVector(g4.bfs(0));
    std::cout << "BFS starting from node 3: ";
    printVector(g4.bfs(3));

    // Test Case 5: Invalid start node
    std::cout << "\nTest Case 5: Invalid start node" << std::endl;
    Graph g5(3);
    g5.addEdge(0, 1);
    std::cout << "BFS starting from node 5: ";
    printVector(g5.bfs(5));

    return 0;
}