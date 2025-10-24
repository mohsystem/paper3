#include <iostream>
#include <vector>
#include <queue>
#include <limits>

using namespace std;

// Using a type alias for graph edges for clarity
using Edge = pair<int, int>; // {destination, weight}

/**
 * Implements Dijkstra's algorithm to find the shortest path from a source
 * node to all other nodes in a weighted graph.
 *
 * @param V The number of vertices in the graph.
 * @param adj The adjacency list representation of the graph.
 * @param src The source vertex.
 * @return A vector containing the shortest distances from the source vertex.
 */
vector<int> dijkstra(int V, const vector<vector<Edge>>& adj, int src) {
    const int INF = numeric_limits<int>::max();
    vector<int> dist(V, INF);
    dist[src] = 0;

    // Min-priority queue storing {distance, vertex}
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.push({0, src});

    while (!pq.empty()) {
        int u = pq.top().second;
        int d = pq.top().first;
        pq.pop();

        // If we've found a shorter path already, skip
        if (d > dist[u]) {
            continue;
        }

        for (const auto& edge : adj[u]) {
            int v = edge.first;
            int weight = edge.second;

            // Relaxation step
            if (dist[u] != INF && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

void print_distances(int src, const vector<int>& dist) {
    cout << "Distances from source " << src << ": [";
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
    // --- Test Case 1: Simple Graph ---
    cout << "--- Test Case 1 ---" << endl;
    int V1 = 5, src1 = 0;
    vector<vector<Edge>> adj1(V1);
    adj1[0].push_back({1, 10}); adj1[0].push_back({3, 5});
    adj1[1].push_back({2, 1}); adj1[1].push_back({3, 2});
    adj1[2].push_back({4, 4});
    adj1[3].push_back({1, 3}); adj1[3].push_back({2, 9}); adj1[3].push_back({4, 2});
    adj1[4].push_back({0, 7}); adj1[4].push_back({2, 6});
    vector<int> dist1 = dijkstra(V1, adj1, src1);
    print_distances(src1, dist1);

    // --- Test Case 2: Disconnected Graph ---
    cout << "\n--- Test Case 2 ---" << endl;
    int V2 = 6, src2 = 0;
    vector<vector<Edge>> adj2(V2);
    adj2[0].push_back({1, 2});
    adj2[1].push_back({2, 3});
    adj2[3].push_back({4, 5});
    adj2[4].push_back({5, 6});
    vector<int> dist2 = dijkstra(V2, adj2, src2);
    print_distances(src2, dist2);

    // --- Test Case 3: Line Graph ---
    cout << "\n--- Test Case 3 ---" << endl;
    int V3 = 4, src3 = 0;
    vector<vector<Edge>> adj3(V3);
    adj3[0].push_back({1, 1});
    adj3[1].push_back({2, 2});
    adj3[2].push_back({3, 3});
    vector<int> dist3 = dijkstra(V3, adj3, src3);
    print_distances(src3, dist3);

    // --- Test Case 4: Graph with alternative paths ---
    cout << "\n--- Test Case 4 ---" << endl;
    int V4 = 6, src4 = 0;
    vector<vector<Edge>> adj4(V4);
    adj4[0].push_back({1, 4}); adj4[0].push_back({2, 1});
    adj4[1].push_back({3, 1});
    adj4[2].push_back({1, 2}); adj4[2].push_back({3, 5});
    adj4[3].push_back({4, 3});
    adj4[4].push_back({5, 2});
    adj4[2].push_back({5, 10});
    vector<int> dist4 = dijkstra(V4, adj4, src4);
    print_distances(src4, dist4);

    // --- Test Case 5: Single Node Graph ---
    cout << "\n--- Test Case 5 ---" << endl;
    int V5 = 1, src5 = 0;
    vector<vector<Edge>> adj5(V5);
    vector<int> dist5 = dijkstra(V5, adj5, src5);
    print_distances(src5, dist5);

    return 0;
}