
#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <algorithm>

using namespace std;

class Graph {
private:
    int vertices;
    vector<list<int>> adjacencyList;
    
public:
    Graph(int vertices) {
        this->vertices = vertices;
        adjacencyList.resize(vertices);
    }
    
    void addEdge(int source, int destination) {
        if (source >= 0 && source < vertices && destination >= 0 && destination < vertices) {
            adjacencyList[source].push_back(destination);
        }
    }
    
    vector<int> breadthFirstSearch(int startNode) {
        vector<int> result;
        if (startNode < 0 || startNode >= vertices) {
            return result;
        }
        
        vector<bool> visited(vertices, false);
        queue<int> q;
        
        visited[startNode] = true;
        q.push(startNode);
        
        while (!q.empty()) {
            int node = q.front();
            q.pop();
            result.push_back(node);
            
            for (int neighbor : adjacencyList[node]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
        
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
    cout << "Test Case 1: Linear Graph" << endl;
    Graph graph1(4);
    graph1.addEdge(0, 1);
    graph1.addEdge(1, 2);
    graph1.addEdge(2, 3);
    cout << "BFS from node 0: ";
    printVector(graph1.breadthFirstSearch(0));
    
    // Test Case 2: Graph with multiple branches
    cout << "\\nTest Case 2: Branching Graph" << endl;
    Graph graph2(6);
    graph2.addEdge(0, 1);
    graph2.addEdge(0, 2);
    graph2.addEdge(1, 3);
    graph2.addEdge(1, 4);
    graph2.addEdge(2, 5);
    cout << "BFS from node 0: ";
    printVector(graph2.breadthFirstSearch(0));
    
    // Test Case 3: Graph with cycle
    cout << "\\nTest Case 3: Graph with Cycle" << endl;
    Graph graph3(5);
    graph3.addEdge(0, 1);
    graph3.addEdge(1, 2);
    graph3.addEdge(2, 3);
    graph3.addEdge(3, 1);
    graph3.addEdge(0, 4);
    cout << "BFS from node 0: ";
    printVector(graph3.breadthFirstSearch(0));
    
    // Test Case 4: Disconnected graph
    cout << "\\nTest Case 4: Disconnected Graph" << endl;
    Graph graph4(5);
    graph4.addEdge(0, 1);
    graph4.addEdge(2, 3);
    graph4.addEdge(3, 4);
    cout << "BFS from node 0: ";
    printVector(graph4.breadthFirstSearch(0));
    
    // Test Case 5: Single node
    cout << "\\nTest Case 5: Single Node" << endl;
    Graph graph5(1);
    cout << "BFS from node 0: ";
    printVector(graph5.breadthFirstSearch(0));
    
    return 0;
}
