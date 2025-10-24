#include <bits/stdc++.h>
using namespace std;

static const long long INF = LLONG_MAX / 4;

vector<long long> dijkstra(int n, const vector<vector<pair<int,long long>>>& adj, int start) {
    if (n <= 0) throw invalid_argument("Number of nodes must be positive.");
    if ((int)adj.size() != n) throw invalid_argument("Adjacency list size must be n.");
    if (start < 0 || start >= n) throw invalid_argument("Start node out of range.");

    for (int u = 0; u < n; ++u) {
        for (auto &e : adj[u]) {
            int v = e.first;
            long long w = e.second;
            if (v < 0 || v >= n) throw invalid_argument("Edge endpoint out of range.");
            if (w < 0) throw invalid_argument("Negative weights not allowed for Dijkstra.");
        }
    }

    vector<long long> dist(n, INF);
    vector<char> visited(n, 0);
    dist[start] = 0;

    using State = pair<long long,int>; // (dist, node)
    priority_queue<State, vector<State>, greater<State>> pq;
    pq.emplace(0LL, start);

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (visited[u]) continue;
        visited[u] = 1;
        for (auto [v, w] : adj[u]) {
            if (visited[v]) continue;
            if (d > INF - w) continue; // prevent overflow
            long long nd = d + w;
            if (nd < dist[v]) {
                dist[v] = nd;
                pq.emplace(nd, v);
            }
        }
    }
    return dist;
}

static void print_result(const vector<long long>& dist) {
    for (size_t i = 0; i < dist.size(); ++i) {
        if (i) cout << ' ';
        if (dist[i] >= INF) cout << "INF";
        else cout << dist[i];
    }
    cout << '\n';
}

int main() {
    // Test case 1
    {
        int n = 5, s = 0;
        vector<vector<pair<int,long long>>> adj(n);
        adj[0].push_back({1,2});
        adj[0].push_back({2,5});
        adj[1].push_back({2,1});
        adj[1].push_back({3,2});
        adj[2].push_back({3,1});
        adj[3].push_back({4,3});
        auto dist = dijkstra(n, adj, s);
        print_result(dist); // 0 2 3 4 7
    }
    // Test case 2
    {
        int n = 5, s = 0;
        vector<vector<pair<int,long long>>> adj(n);
        adj[0].push_back({1,1});
        adj[1].push_back({2,1});
        adj[2].push_back({3,1});
        adj[3].push_back({4,1});
        auto dist = dijkstra(n, adj, s);
        print_result(dist); // 0 1 2 3 4
    }
    // Test case 3
    {
        int n = 5, s = 0;
        vector<vector<pair<int,long long>>> adj(n);
        adj[0].push_back({1,10});
        adj[0].push_back({2,3});
        adj[2].push_back({1,1});
        adj[2].push_back({3,2});
        adj[1].push_back({3,4});
        adj[3].push_back({4,2});
        adj[4].push_back({1,1});
        auto dist = dijkstra(n, adj, s);
        print_result(dist); // 0 4 3 5 7
    }
    // Test case 4 (disconnected)
    {
        int n = 5, s = 0;
        vector<vector<pair<int,long long>>> adj(n);
        adj[1].push_back({2,2});
        adj[2].push_back({3,2});
        auto dist = dijkstra(n, adj, s);
        print_result(dist); // 0 INF INF INF INF
    }
    // Test case 5 (classic)
    {
        int n = 6, s = 0;
        vector<vector<pair<int,long long>>> adj(n);
        adj[0].push_back({1,7});
        adj[0].push_back({2,9});
        adj[0].push_back({5,14});
        adj[1].push_back({2,10});
        adj[1].push_back({3,15});
        adj[2].push_back({3,11});
        adj[2].push_back({5,2});
        adj[3].push_back({4,6});
        adj[4].push_back({5,9});
        auto dist = dijkstra(n, adj, s);
        print_result(dist); // 0 7 9 20 26 11
    }
    return 0;
}