
#include <iostream>
#include <vector>
#include <queue>
#include <cstring>

using namespace std;

class Graph {
private:
    int vertices;
    vector<vector<int>> adjacencyList;
    
public:
    Graph(int vertices) {
        this->vertices = vertices;
        adjacencyList.resize(vertices);
    }
    
    void addEdge(int source, int destination) {
        adjacencyList[source].push_back(destination);
    }
    
    vector<int> bfs(int startNode) {
        vector<int> result;
        vector<bool> visited(vertices, false);
        queue<int> q;
        
        visited[startNode] = true;
        q.push(startNode);
        
        while (!q.empty()) {
            int current = q.front();
            q.pop();
            result.push_back(current);
            
            for (int neighbor : adjacencyList[current]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
        
        return result;
    }
};

vector<int> breadthFirstSearch(int vertices, vector<vector<int>>& edges, int startNode) {
    Graph graph(vertices);
    for (const auto& edge : edges) {
        graph.addEdge(edge[0], edge[1]);
    }
    return graph.bfs(startNode);
}

void printVector(const vector<int>& vec) {
    cout << "[";
    for (size_t i = 0; i < vec.size(); i++) {
        cout << vec[i];
        if (i < vec.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

int main() {
    // Test Case 1: Simple linear graph
    cout << "Test Case 1:" << endl;
    vector<vector<int>> edges1 = {{0, 1}, {1, 2}, {2, 3}};
    printVector(breadthFirstSearch(4, edges1, 0));
    
    // Test Case 2: Graph with multiple branches
    cout << "\\nTest Case 2:" << endl;
    vector<vector<int>> edges2 = {{0, 1}, {0, 2}, {1, 3}, {1, 4}, {2, 5}};
    printVector(breadthFirstSearch(6, edges2, 0));
    
    // Test Case 3: Disconnected graph
    cout << "\\nTest Case 3:" << endl;
    vector<vector<int>> edges3 = {{0, 1}, {2, 3}};
    printVector(breadthFirstSearch(4, edges3, 0));
    
    // Test Case 4: Graph with cycles
    cout << "\\nTest Case 4:" << endl;
    vector<vector<int>> edges4 = {{0, 1}, {0, 2}, {1, 2}, {2, 0}, {2, 3}};
    printVector(breadthFirstSearch(4, edges4, 0));
    
    // Test Case 5: Single node
    cout << "\\nTest Case 5:" << endl;
    vector<vector<int>> edges5 = {};
    printVector(breadthFirstSearch(1, edges5, 0));
    
    return 0;
}
