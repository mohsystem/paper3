#include <iostream>
#include <vector>
#include <stdexcept>

class Graph {
private:
    int V;
    std::vector<std::vector<int>> adj;

    void dfsUtil(int v, std::vector<bool>& visited, std::vector<int>& result) {
        visited[v] = true;
        result.push_back(v);

        for (int neighbor : adj[v]) {
            if (!visited[neighbor]) {
                dfsUtil(neighbor, visited, result);
            }
        }
    }

public:
    explicit Graph(int vertices) {
        if (vertices < 0) {
            throw std::invalid_argument("Number of vertices must be non-negative");
        }
        V = vertices;
        adj.resize(V);
    }

    void addEdge(int u, int v) {
        if (u < 0 || u >= V || v < 0 || v >= V) {
            std::cerr << "Error: Vertex out of bounds. Cannot add edge (" << u << ", " << v << ")\n";
            return;
        }
        adj[u].push_back(v);
    }

    std::vector<int> dfs(int startNode) {
        std::vector<int> result;
        if (startNode < 0 || startNode >= V) {
            std::cerr << "Error: Start node " << startNode << " is invalid.\n";
            return result;
        }

        std::vector<bool> visited(V, false);
        dfsUtil(startNode, visited, result);
        return result;
    }
};

void print_vector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1: Standard DFS
    std::cout << "Test Case 1:" << std::endl;
    Graph g1(7);
    g1.addEdge(0, 1);
    g1.addEdge(0, 2);
    g1.addEdge(1, 3);
    g1.addEdge(1, 4);
    g1.addEdge(2, 5);
    g1.addEdge(2, 6);
    std::cout << "DFS starting from vertex 0: ";
    print_vector(g1.dfs(0));

    // Test Case 2: Graph with a cycle
    std::cout << "\nTest Case 2:" << std::endl;
    Graph g2(4);
    g2.addEdge(0, 1);
    g2.addEdge(0, 2);
    g2.addEdge(1, 2);
    g2.addEdge(2, 0);
    g2.addEdge(2, 3);
    g2.addEdge(3, 3);
    std::cout << "DFS starting from vertex 2: ";
    print_vector(g2.dfs(2));

    // Test Case 3: Disconnected graph component
    std::cout << "\nTest Case 3:" << std::endl;
    Graph g3(5);
    g3.addEdge(0, 1);
    g3.addEdge(1, 2);
    g3.addEdge(3, 4);
    std::cout << "DFS starting from vertex 0: ";
    print_vector(g3.dfs(0));
    std::cout << "DFS starting from vertex 3: ";
    print_vector(g3.dfs(3));

    // Test Case 4: Start node has no outgoing edges
    std::cout << "\nTest Case 4:" << std::endl;
    Graph g4(3);
    g4.addEdge(0, 1);
    g4.addEdge(0, 2);
    std::cout << "DFS starting from vertex 1: ";
    print_vector(g4.dfs(1));

    // Test Case 5: Invalid start node
    std::cout << "\nTest Case 5:" << std::endl;
    Graph g5(3);
    g5.addEdge(0, 1);
    g5.addEdge(1, 2);
    std::cout << "DFS starting from vertex 5: ";
    print_vector(g5.dfs(5));
    std::cout << "DFS starting from vertex -1: ";
    print_vector(g5.dfs(-1));

    return 0;
}