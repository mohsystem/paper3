
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Graph {
private:
    int vertices;
    vector<vector<int>> adjList;
    
    void dfsUtil(int node, vector<bool>& visited, vector<int>& result) {
        visited[node] = true;
        result.push_back(node);
        
        for (int neighbor : adjList[node]) {
            if (!visited[neighbor]) {
                dfsUtil(neighbor, visited, result);
            }
        }
    }
    
public:
    Graph(int vertices) {
        this->vertices = vertices;
        adjList.resize(vertices);
    }
    
    void addEdge(int src, int dest) {
        adjList[src].push_back(dest);
    }
    
    vector<int> dfs(int startNode) {
        vector<int> result;
        vector<bool> visited(vertices, false);
        dfsUtil(startNode, visited, result);
        return result;
    }
};

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
    cout << "Test Case 1: Linear Graph (0->1->2->3)" << endl;
    Graph g1(4);
    g1.addEdge(0, 1);
    g1.addEdge(1, 2);
    g1.addEdge(2, 3);
    cout << "DFS from node 0: ";
    printVector(g1.dfs(0));
    
    // Test Case 2: Graph with branching
    cout << "\\nTest Case 2: Branching Graph" << endl;
    Graph g2(7);
    g2.addEdge(0, 1);
    g2.addEdge(0, 2);
    g2.addEdge(1, 3);
    g2.addEdge(1, 4);
    g2.addEdge(2, 5);
    g2.addEdge(2, 6);
    cout << "DFS from node 0: ";
    printVector(g2.dfs(0));
    
    // Test Case 3: Graph with cycle
    cout << "\\nTest Case 3: Graph with Cycle" << endl;
    Graph g3(4);
    g3.addEdge(0, 1);
    g3.addEdge(1, 2);
    g3.addEdge(2, 0);
    g3.addEdge(2, 3);
    cout << "DFS from node 0: ";
    printVector(g3.dfs(0));
    
    // Test Case 4: Disconnected graph
    cout << "\\nTest Case 4: Disconnected Graph" << endl;
    Graph g4(5);
    g4.addEdge(0, 1);
    g4.addEdge(1, 2);
    g4.addEdge(3, 4);
    cout << "DFS from node 0: ";
    printVector(g4.dfs(0));
    cout << "DFS from node 3: ";
    printVector(g4.dfs(3));
    
    // Test Case 5: Single node
    cout << "\\nTest Case 5: Single Node" << endl;
    Graph g5(1);
    cout << "DFS from node 0: ";
    printVector(g5.dfs(0));
    
    return 0;
}
