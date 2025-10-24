
#include <iostream>
#include <vector>
#include <list>
using namespace std;

class Graph {
private:
    int vertices;
    vector<list<int>> adjacencyList;
    
    void dfsUtil(int node, vector<bool>& visited, vector<int>& result) {
        visited[node] = true;
        result.push_back(node);
        
        for (int neighbor : adjacencyList[node]) {
            if (!visited[neighbor]) {
                dfsUtil(neighbor, visited, result);
            }
        }
    }
    
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
    
    vector<int> depthFirstSearch(int startNode) {
        vector<int> result;
        if (startNode < 0 || startNode >= vertices) {
            return result;
        }
        
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
    cout << "Test Case 1: Linear Graph" << endl;
    Graph graph1(5);
    graph1.addEdge(0, 1);
    graph1.addEdge(1, 2);
    graph1.addEdge(2, 3);
    graph1.addEdge(3, 4);
    cout << "DFS from node 0: ";
    printVector(graph1.depthFirstSearch(0));
    
    // Test Case 2: Graph with branches
    cout << "\\nTest Case 2: Branched Graph" << endl;
    Graph graph2(7);
    graph2.addEdge(0, 1);
    graph2.addEdge(0, 2);
    graph2.addEdge(1, 3);
    graph2.addEdge(1, 4);
    graph2.addEdge(2, 5);
    graph2.addEdge(2, 6);
    cout << "DFS from node 0: ";
    printVector(graph2.depthFirstSearch(0));
    
    // Test Case 3: Graph with cycle
    cout << "\\nTest Case 3: Graph with Cycle" << endl;
    Graph graph3(4);
    graph3.addEdge(0, 1);
    graph3.addEdge(1, 2);
    graph3.addEdge(2, 3);
    graph3.addEdge(3, 1);
    cout << "DFS from node 0: ";
    printVector(graph3.depthFirstSearch(0));
    
    // Test Case 4: Disconnected graph
    cout << "\\nTest Case 4: Disconnected Graph" << endl;
    Graph graph4(6);
    graph4.addEdge(0, 1);
    graph4.addEdge(1, 2);
    graph4.addEdge(3, 4);
    graph4.addEdge(4, 5);
    cout << "DFS from node 0: ";
    printVector(graph4.depthFirstSearch(0));
    cout << "DFS from node 3: ";
    printVector(graph4.depthFirstSearch(3));
    
    // Test Case 5: Single node
    cout << "\\nTest Case 5: Single Node" << endl;
    Graph graph5(1);
    cout << "DFS from node 0: ";
    printVector(graph5.depthFirstSearch(0));
    
    return 0;
}
