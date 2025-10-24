#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <utility>

using namespace std;

// Using a type alias for a pair of integers for cleaner code
using iPair = pair<int, int>;

/**
 * Finds the shortest paths from a starting node to all other nodes in a weighted graph.
 *
 * @param V         The number of vertices in the graph.
 * @param adj       The adjacency list representation of the graph.
 * @param startNode The starting node.
 * @return A vector containing the shortest distances from the start node.
 */
vector<int> dijkstra(int V, const vector<vector<iPair>>& adj, int startNode) {
    // Priority queue to store {distance, vertex}
    priority_queue<iPair, vector<iPair>, greater<iPair>> pq;

    // Vector to store distances, initialized to infinity
    vector<int> dist(V, numeric_limits<int>::max());

    // Start node setup
    pq.push({0, startNode});
    dist[startNode] = 0;

    while (!pq.empty()) {
        int u = pq.top().second;
        int d = pq.top().first;
        pq.pop();

        // Optimization: if we've found a shorter path already, skip
        if (d > dist[u]) {
            continue;
        }

        // Iterate through all adjacent vertices
        for (const auto& edge : adj[u]) {
            int v = edge.first;
            int weight = edge.second;

            // If there is a shorter path to v through u
            if (dist[u] != numeric_limits<int>::max() && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                pq.push({dist[v], v});
            }
        }
    }

    return dist;
}

void printDistances(int startNode, const vector<int>& dist) {
    cout << "Shortest distances from node " << startNode << ": [";
    for (size_t i = 0; i < dist.size(); ++i) {
        if (dist[i] == numeric_limits<int>::max()) {
            cout << "INF";
        } else {
            cout << dist[i];
        }
        if (i < dist.size() - 1) {
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

int main() {
    // Test Case 1: Simple connected graph
    cout << "--- Test Case 1 ---" << endl;
    int V1 = 5;
    vector<vector<iPair>> adj1(V1);
    adj1[0].push_back({1, 9}); adj1[0].push_back({2, 6}); adj1[0].push_back({3, 5}); adj1[0].push_back({4, 3});
    adj1[2].push_back({1, 2}); adj1[2].push_back({3, 4});
    int startNode1 = 0;
    vector<int> dist1 = dijkstra(V1, adj1, startNode1);
    printDistances(startNode1, dist1);

    // Test Case 2: Graph with unreachable node
    cout << "\n--- Test Case 2 ---" << endl;
    int V2 = 4;
    vector<vector<iPair>> adj2(V2);
    adj2[0].push_back({1, 10});
    adj2[1].push_back({2, 20});
    int startNode2 = 0;
    vector<int> dist2 = dijkstra(V2, adj2, startNode2);
    printDistances(startNode2, dist2);

    // Test Case 3: Linear graph
    cout << "\n--- Test Case 3 ---" << endl;
    int V3 = 4;
    vector<vector<iPair>> adj3(V3);
    adj3[0].push_back({1, 5}); adj3[1].push_back({2, 5}); adj3[2].push_back({3, 5});
    int startNode3 = 0;
    vector<int> dist3 = dijkstra(V3, adj3, startNode3);
    printDistances(startNode3, dist3);

    // Test Case 4: Graph with a cycle
    cout << "\n--- Test Case 4 ---" << endl;
    int V4 = 4;
    vector<vector<iPair>> adj4(V4);
    adj4[0].push_back({1, 1}); adj4[0].push_back({3, 10});
    adj4[1].push_back({2, 2});
    adj4[2].push_back({0, 3});
    int startNode4 = 0;
    vector<int> dist4 = dijkstra(V4, adj4, startNode4);
    printDistances(startNode4, dist4);

    // Test Case 5: More complex graph with multiple paths
    cout << "\n--- Test Case 5 ---" << endl;
    int V5 = 6;
    vector<vector<iPair>> adj5(V5);
    adj5[0].push_back({1, 7}); adj5[0].push_back({2, 9}); adj5[0].push_back({5, 14});
    adj5[1].push_back({2, 10}); adj5[1].push_back({3, 15});
    adj5[2].push_back({3, 11}); adj5[2].push_back({5, 2});
    adj5[3].push_back({4, 6});
    adj5[4].push_back({5, 9});
    int startNode5 = 0;
    vector<int> dist5 = dijkstra(V5, adj5, startNode5);
    printDistances(startNode5, dist5);

    return 0;
}