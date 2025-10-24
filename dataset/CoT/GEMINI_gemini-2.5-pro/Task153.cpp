#include <iostream>
#include <vector>
#include <queue>
#include <list>

class Graph {
private:
    int V; // Number of vertices
    std::vector<std::list<int>> adj; // Adjacency list

public:
    Graph(int vertices) {
        if (vertices < 0) {
            // Handle negative vertex count if necessary, e.g., throw an exception
            V = 0;
        } else {
            V = vertices;
            adj.resize(V);
        }
    }

    void addEdge(int v, int w) {
        if (v >= 0 && v < V && w >= 0 && w < V) {
            adj[v].push_back(w);
            adj[w].push_back(v); // For an undirected graph
        }
    }

    std::vector<int> bfs(int startNode) {
        std::vector<int> result;
        if (startNode < 0 || startNode >= V) {
            std::cerr << "Error: Start node " << startNode << " is out of bounds." << std::endl;
            return result; // Return empty vector for invalid start node
        }

        std::vector<bool> visited(V, false);
        std::queue<int> q;

        visited[startNode] = true;
        q.push(startNode);

        while (!q.empty()) {
            int currentNode = q.front();
            q.pop();
            result.push_back(currentNode);

            for (int neighbor : adj[currentNode]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
        return result;
    }
};

int main() {
    std::cout << "--- Breadth-First Search (BFS) Traversal ---" << std::endl;

    // Graph structure for all test cases
    //        0
    //       / \
    //      1---2
    //     / \ |
    //    3---4 5
    //         |
    //         6
    int numVertices = 7;
    Graph g(numVertices);
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 3);
    g.addEdge(1, 4);
    g.addEdge(2, 5);
    g.addEdge(3, 4);
    g.addEdge(5, 6);

    int startNodes[] = {0, 3, 6, 4, 1};
    int numTestCases = sizeof(startNodes) / sizeof(startNodes[0]);

    for (int i = 0; i < numTestCases; ++i) {
        std::cout << "\nTest Case " << (i + 1) << ": Starting from node " << startNodes[i] << std::endl;
        std::vector<int> bfsResult = g.bfs(startNodes[i]);
        std::cout << "BFS Traversal: ";
        for (int node : bfsResult) {
            std::cout << node << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}