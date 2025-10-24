
#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <utility>

using namespace std;

class Task162 {
public:
    static vector<int> dijkstra(vector<vector<pair<int, int>>>& graph, int start, int n) {
        vector<int> distances(n, INT_MAX);
        distances[start] = 0;
        
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
        pq.push({0, start});
        
        vector<bool> visited(n, false);
        
        while (!pq.empty()) {
            int u = pq.top().second;
            pq.pop();
            
            if (visited[u]) continue;
            visited[u] = true;
            
            for (auto& edge : graph[u]) {
                int v = edge.first;
                int weight = edge.second;
                
                if (!visited[v] && distances[u] != INT_MAX && 
                    distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    pq.push({distances[v], v});
                }
            }
        }
        
        return distances;
    }
};

int main() {
    // Test Case 1: Simple graph
    int n1 = 5;
    vector<vector<pair<int, int>>> graph1(n1);
    graph1[0].push_back({1, 4});
    graph1[0].push_back({2, 1});
    graph1[2].push_back({1, 2});
    graph1[1].push_back({3, 1});
    graph1[2].push_back({3, 5});
    graph1[3].push_back({4, 3});
    
    cout << "Test Case 1:" << endl;
    cout << "Start node: 0" << endl;
    cout << "Shortest distances: ";
    vector<int> result1 = Task162::dijkstra(graph1, 0, n1);
    for (int d : result1) {
        if (d == INT_MAX) cout << "INF ";
        else cout << d << " ";
    }
    cout << endl;
    
    // Test Case 2: Disconnected node
    int n2 = 4;
    vector<vector<pair<int, int>>> graph2(n2);
    graph2[0].push_back({1, 5});
    graph2[1].push_back({2, 3});
    
    cout << "\\nTest Case 2:" << endl;
    cout << "Start node: 0" << endl;
    cout << "Shortest distances: ";
    vector<int> result2 = Task162::dijkstra(graph2, 0, n2);
    for (int d : result2) {
        if (d == INT_MAX) cout << "INF ";
        else cout << d << " ";
    }
    cout << endl;
    
    // Test Case 3: Complete graph
    int n3 = 3;
    vector<vector<pair<int, int>>> graph3(n3);
    graph3[0].push_back({1, 2});
    graph3[0].push_back({2, 5});
    graph3[1].push_back({2, 1});
    
    cout << "\\nTest Case 3:" << endl;
    cout << "Start node: 0" << endl;
    cout << "Shortest distances: ";
    vector<int> result3 = Task162::dijkstra(graph3, 0, n3);
    for (int d : result3) {
        if (d == INT_MAX) cout << "INF ";
        else cout << d << " ";
    }
    cout << endl;
    
    // Test Case 4: Single node
    int n4 = 1;
    vector<vector<pair<int, int>>> graph4(n4);
    
    cout << "\\nTest Case 4:" << endl;
    cout << "Start node: 0" << endl;
    cout << "Shortest distances: ";
    vector<int> result4 = Task162::dijkstra(graph4, 0, n4);
    for (int d : result4) {
        if (d == INT_MAX) cout << "INF ";
        else cout << d << " ";
    }
    cout << endl;
    
    // Test Case 5: Multiple paths
    int n5 = 6;
    vector<vector<pair<int, int>>> graph5(n5);
    graph5[0].push_back({1, 7});
    graph5[0].push_back({2, 9});
    graph5[0].push_back({5, 14});
    graph5[1].push_back({2, 10});
    graph5[1].push_back({3, 15});
    graph5[2].push_back({3, 11});
    graph5[2].push_back({5, 2});
    graph5[3].push_back({4, 6});
    graph5[4].push_back({5, 9});
    
    cout << "\\nTest Case 5:" << endl;
    cout << "Start node: 0" << endl;
    cout << "Shortest distances: ";
    vector<int> result5 = Task162::dijkstra(graph5, 0, n5);
    for (int d : result5) {
        if (d == INT_MAX) cout << "INF ";
        else cout << d << " ";
    }
    cout << endl;
    
    return 0;
}
