#include <iostream>
#include <vector>
#include <queue>
#include <limits>

using std::vector;
using std::pair;
using std::priority_queue;
using std::cout;
using std::endl;

static const long long INF = std::numeric_limits<long long>::max() / 4;

vector<long long> dijkstra(int n, const vector<std::array<int, 3>>& edges, int start) {
    if (n <= 0 || start < 0 || start >= n) {
        return {};
    }

    vector<vector<pair<int, long long>>> graph(n);
    for (const auto& e : edges) {
        int u = e[0];
        int v = e[1];
        long long w = static_cast<long long>(e[2]);
        if (u < 0 || u >= n || v < 0 || v >= n) {
            return {};
        }
        if (w < 0 || w > INF / 2) {
            return {};
        }
        graph[u].push_back({v, w});
    }

    vector<long long> dist(n, INF);
    vector<char> visited(n, 0);
    dist[start] = 0;

    using Node = pair<long long, int>;
    priority_queue<Node, vector<Node>, std::greater<Node>> pq;
    pq.push({0LL, start});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (visited[u]) continue;
        visited[u] = 1;

        for (const auto& [v, w] : graph[u]) {
            if (d <= INF - w) {
                long long alt = d + w;
                if (alt < dist[v]) {
                    dist[v] = alt;
                    pq.push({alt, v});
                }
            }
        }
    }

    return dist;
}

void printResult(const char* title, const vector<long long>& dist) {
    cout << title << "\n";
    if (dist.empty()) {
        cout << "Invalid input\n";
        return;
    }
    for (size_t i = 0; i < dist.size(); ++i) {
        if (dist[i] >= INF) cout << "INF";
        else cout << dist[i];
        if (i + 1 != dist.size()) cout << " ";
    }
    cout << "\n";
}

int main() {
    // Test 1: Simple directed graph
    {
        int n = 3;
        vector<std::array<int, 3>> edges = {
            {0, 1, 4},
            {0, 2, 1},
            {2, 1, 2}
        };
        auto dist = dijkstra(n, edges, 0);
        printResult("Test 1", dist);
    }

    // Test 2: Disconnected nodes
    {
        int n = 4;
        vector<std::array<int, 3>> edges = {
            {0, 1, 5}
        };
        auto dist = dijkstra(n, edges, 0);
        printResult("Test 2", dist);
    }

    // Test 3: Zero-weight edges
    {
        int n = 4;
        vector<std::array<int, 3>> edges = {
            {0, 1, 0},
            {1, 2, 0},
            {2, 3, 1}
        };
        auto dist = dijkstra(n, edges, 0);
        printResult("Test 3", dist);
    }

    // Test 4: Larger weights with alternative shorter path
    {
        int n = 5;
        vector<std::array<int, 3>> edges = {
            {0, 1, 1000000000},
            {1, 2, 1000000000},
            {0, 3, 1},
            {3, 4, 1},
            {4, 2, 1}
        };
        auto dist = dijkstra(n, edges, 0);
        printResult("Test 4", dist);
    }

    // Test 5: Invalid input (negative weight)
    {
        int n = 2;
        vector<std::array<int, 3>> edges = {
            {0, 1, -1}
        };
        auto dist = dijkstra(n, edges, 0);
        printResult("Test 5", dist);
    }

    return 0;
}