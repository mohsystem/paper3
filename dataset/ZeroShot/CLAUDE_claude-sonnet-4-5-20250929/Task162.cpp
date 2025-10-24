
#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <utility>

using namespace std;

typedef pair<int, int> pii;

vector<int> dijkstra(vector<vector<pii>>& graph, int start, int n) {
    vector<int> distances(n, INT_MAX);
    distances[start] = 0;
    
    priority_queue<pii, vector<pii>, greater<pii>> pq;
    pq.push({0, start});
    
    vector<bool> visited(n, false);
    
    while (!pq.empty()) {
        int current_dist = pq.top().first;
        int u = pq.top().second;
        pq.pop();
        
        if (visited[u]) continue;
        visited[u] = true;
        
        for (auto& edge : graph[u]) {
            int v = edge.first;
            int weight = edge.second;
            
            if (!visited[v] && distances[u] != INT_MAX && distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                pq.push({distances[v], v});
            }
        }
    }
    
    return distances;
}

void printResult(const vector<int>& result) {
    cout << "[";
    for (size_t i = 0; i < result.size(); i++) {
        if (result[i] == INT_MAX)
            cout << "INF";
        else
            cout << result[i];
        if (i < result.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

int main() {
    // Test Case 1: Simple graph
    cout << "Test Case 1:" << endl;
    int n1 = 5;
    vector<vector<pii>> graph1(n1);
    graph1[0].push_back({1, 4});
    graph1[0].push_back({2, 1});
    graph1[1].push_back({3, 1});
    graph1[2].push_back({1, 2});
    graph1[2].push_back({3, 5});
    graph1[3].push_back({4, 3});
    vector<int> result1 = dijkstra(graph1, 0, n1);
    printResult(result1);
    
    // Test Case 2: Single node
    cout << "\\nTest Case 2:" << endl;
    int n2 = 1;
    vector<vector<pii>> graph2(n2);
    vector<int> result2 = dijkstra(graph2, 0, n2);
    printResult(result2);
    
    // Test Case 3: Disconnected graph
    cout << "\\nTest Case 3:" << endl;
    int n3 = 4;
    vector<vector<pii>> graph3(n3);
    graph3[0].push_back({1, 1});
    graph3[1].push_back({0, 1});
    vector<int> result3 = dijkstra(graph3, 0, n3);
    printResult(result3);
    
    // Test Case 4: Dense graph
    cout << "\\nTest Case 4:" << endl;
    int n4 = 4;
    vector<vector<pii>> graph4(n4);
    graph4[0].push_back({1, 5});
    graph4[0].push_back({2, 10});
    graph4[1].push_back({2, 3});
    graph4[1].push_back({3, 8});
    graph4[2].push_back({3, 2});
    vector<int> result4 = dijkstra(graph4, 0, n4);
    printResult(result4);
    
    // Test Case 5: Linear graph
    cout << "\\nTest Case 5:" << endl;
    int n5 = 6;
    vector<vector<pii>> graph5(n5);
    graph5[0].push_back({1, 2});
    graph5[1].push_back({2, 3});
    graph5[2].push_back({3, 1});
    graph5[3].push_back({4, 4});
    graph5[4].push_back({5, 2});
    vector<int> result5 = dijkstra(graph5, 0, n5);
    printResult(result5);
    
    return 0;
}
