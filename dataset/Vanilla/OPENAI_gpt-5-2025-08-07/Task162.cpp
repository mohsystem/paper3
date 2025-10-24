#include <bits/stdc++.h>
using namespace std;

vector<long long> dijkstra(int n, const vector<array<int,3>>& edges, int start) {
    vector<vector<pair<int,int>>> adj(n);
    for (auto &e : edges) {
        int u = e[0], v = e[1], w = e[2];
        adj[u].push_back({v, w});
    }
    const long long INF = LLONG_MAX / 4;
    vector<long long> dist(n, INF);
    vector<char> vis(n, 0);
    dist[start] = 0;
    priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<pair<long long,int>>> pq;
    pq.push({0LL, start});
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (vis[u]) continue;
        vis[u] = 1;
        for (auto [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

void printDistances(const vector<long long>& dist) {
    for (size_t i = 0; i < dist.size(); ++i) {
        if (i) cout << ' ';
        if (dist[i] >= LLONG_MAX / 8) cout << "INF";
        else cout << dist[i];
    }
    cout << '\n';
}

int main() {
    // Test 1: Directed graph
    int n1 = 5;
    vector<array<int,3>> edges1 = {
        {0,1,10},{0,2,3},{1,2,1},{2,1,4},{1,3,2},{2,3,2},{3,4,7},{2,4,8},{4,3,9}
    };
    printDistances(dijkstra(n1, edges1, 0)); // Expected: 0 7 3 5 11

    // Test 2: Undirected graph
    int n2 = 4;
    vector<array<int,3>> edges2 = {
        {0,1,5},{1,0,5},{1,2,6},{2,1,6},{0,3,10},{3,0,10},{2,3,2},{3,2,2}
    };
    printDistances(dijkstra(n2, edges2, 0)); // Expected: 0 5 11 10

    // Test 3: Unreachable nodes
    int n3 = 4;
    vector<array<int,3>> edges3 = {
        {0,1,2}
    };
    printDistances(dijkstra(n3, edges3, 0)); // Expected: 0 2 INF INF

    // Test 4: Zero-weight edges
    int n4 = 5;
    vector<array<int,3>> edges4 = {
        {0,1,0},{1,2,0},{2,3,0},{3,4,0}
    };
    printDistances(dijkstra(n4, edges4, 0)); // Expected: 0 0 0 0 0

    // Test 5: Undirected classic
    int n5 = 6;
    vector<array<int,3>> edges5 = {
        {0,1,7},{1,0,7},{0,2,9},{2,0,9},{0,5,14},{5,0,14},
        {1,2,10},{2,1,10},{1,3,15},{3,1,15},{2,3,11},{3,2,11},
        {2,5,2},{5,2,2},{3,4,6},{4,3,6},{4,5,9},{5,4,9}
    };
    printDistances(dijkstra(n5, edges5, 0)); // Expected: 0 7 9 20 20 11

    return 0;
}