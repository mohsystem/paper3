
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <limits>
#include <set>

using namespace std;

struct Edge {
    int destination;
    int weight;
    
    Edge(int dest, int w) : destination(dest), weight(w) {}
};

struct Node {
    int vertex;
    int distance;
    
    Node(int v, int d) : vertex(v), distance(d) {}
    
    bool operator>(const Node& other) const {
        return distance > other.distance;
    }
};

map<int, int> dijkstra(const map<int, vector<Edge>>& graph, int start) {
    map<int, int> distances;
    set<int> visited;
    priority_queue<Node, vector<Node>, greater<Node>> pq;
    
    if (graph.find(start) == graph.end()) {
        return distances;
    }
    
    // Initialize all distances to infinity
    for (const auto& pair : graph) {
        distances[pair.first] = numeric_limits<int>::max();
    }
    distances[start] = 0;
    pq.push(Node(start, 0));
    
    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();
        int currentVertex = current.vertex;
        
        if (visited.find(currentVertex) != visited.end()) {
            continue;
        }
        visited.insert(currentVertex);
        
        auto it = graph.find(currentVertex);
        if (it == graph.end()) {
            continue;
        }
        
        for (const Edge& edge : it->second) {
            if (edge.weight < 0) {
                continue; // Skip negative weights
            }
            
            int neighbor = edge.destination;
            int currentDist = distances[currentVertex];
            
            // Check for overflow
            if (currentDist != numeric_limits<int>::max() &&
                currentDist + edge.weight < numeric_limits<int>::max()) {
                int newDistance = currentDist + edge.weight;
                
                if (newDistance < distances[neighbor]) {
                    distances[neighbor] = newDistance;
                    pq.push(Node(neighbor, newDistance));
                }
            }
        }
    }
    
    return distances;
}

int main() {
    // Test Case 1: Simple graph
    map<int, vector<Edge>> graph1;
    graph1[0] = {Edge(1, 4), Edge(2, 1)};
    graph1[1] = {Edge(3, 1)};
    graph1[2] = {Edge(1, 2), Edge(3, 5)};
    graph1[3] = {};
    
    cout << "Test Case 1:" << endl;
    map<int, int> result1 = dijkstra(graph1, 0);
    for (const auto& pair : result1) {
        cout << pair.first << ": " << pair.second << endl;
    }
    
    // Test Case 2: Disconnected graph
    map<int, vector<Edge>> graph2;
    graph2[0] = {Edge(1, 2)};
    graph2[1] = {};
    graph2[2] = {Edge(3, 1)};
    graph2[3] = {};
    
    cout << "\\nTest Case 2:" << endl;
    map<int, int> result2 = dijkstra(graph2, 0);
    for (const auto& pair : result2) {
        cout << pair.first << ": " << pair.second << endl;
    }
    
    // Test Case 3: Single node
    map<int, vector<Edge>> graph3;
    graph3[0] = {};
    
    cout << "\\nTest Case 3:" << endl;
    map<int, int> result3 = dijkstra(graph3, 0);
    for (const auto& pair : result3) {
        cout << pair.first << ": " << pair.second << endl;
    }
    
    // Test Case 4: Multiple paths
    map<int, vector<Edge>> graph4;
    graph4[0] = {Edge(1, 10), Edge(2, 5)};
    graph4[1] = {Edge(3, 1)};
    graph4[2] = {Edge(1, 3), Edge(3, 9)};
    graph4[3] = {};
    
    cout << "\\nTest Case 4:" << endl;
    map<int, int> result4 = dijkstra(graph4, 0);
    for (const auto& pair : result4) {
        cout << pair.first << ": " << pair.second << endl;
    }
    
    // Test Case 5: Larger graph
    map<int, vector<Edge>> graph5;
    graph5[0] = {Edge(1, 7), Edge(2, 9), Edge(5, 14)};
    graph5[1] = {Edge(0, 7), Edge(2, 10), Edge(3, 15)};
    graph5[2] = {Edge(0, 9), Edge(1, 10), Edge(3, 11), Edge(5, 2)};
    graph5[3] = {Edge(1, 15), Edge(2, 11), Edge(4, 6)};
    graph5[4] = {Edge(3, 6), Edge(5, 9)};
    graph5[5] = {Edge(0, 14), Edge(2, 2), Edge(4, 9)};
    
    cout << "\\nTest Case 5:" << endl;
    map<int, int> result5 = dijkstra(graph5, 0);
    for (const auto& pair : result5) {
        cout << pair.first << ": " << pair.second << endl;
    }
    
    return 0;
}
