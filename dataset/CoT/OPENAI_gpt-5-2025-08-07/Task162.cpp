#include <iostream>
#include <vector>
#include <queue>
#include <tuple>
#include <limits>
#include <algorithm>

/*
Chain-of-Through process:
1) Problem understanding: Shortest paths from a start node using Dijkstra on non-negative weighted graph.
2) Security: Validate inputs, avoid overflow using safe INF, skip invalid edges.
3) Secure coding: Use adjacency list, min-heap, stale entry check, bounds checks.
4) Review: Ensure safe arithmetic and proper handling of unreachable nodes.
5) Output: Final robust implementation with test cases.
*/

struct Result {
    std::vector<long long> dist;
    std::vector<int> parent;
};

Result dijkstra(int n, const std::vector<std::tuple<int,int,long long>>& edges, int start) {
    if (n <= 0) throw std::invalid_argument("Number of nodes must be positive");
    if (start < 0 || start >= n) throw std::invalid_argument("Start node out of range");

    std::vector<std::vector<std::pair<int,long long>>> g(n);
    for (const auto& e : edges) {
        int u, v; long long w;
        std::tie(u, v, w) = e;
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        if (w < 0) continue;
        g[u].push_back({v, w});
    }

    const long long INF = std::numeric_limits<long long>::max() / 4;
    std::vector<long long> dist(n, INF);
    std::vector<int> parent(n, -1);
    dist[start] = 0;

    using Node = std::pair<long long,int>; // {dist, node}
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d != dist[u]) continue;
        for (const auto& [v, w] : g[u]) {
            if (dist[u] <= INF - w) {
                long long nd = dist[u] + w;
                if (nd < dist[v]) {
                    dist[v] = nd;
                    parent[v] = u;
                    pq.push({nd, v});
                }
            }
        }
    }

    return {dist, parent};
}

static void printVec(const std::vector<long long>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        if (v[i] >= (std::numeric_limits<long long>::max()/8)) std::cout << "INF";
        else std::cout << v[i];
    }
    std::cout << "]\n";
}

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    // Test 1
    int n1 = 5;
    std::vector<std::tuple<int,int,long long>> edges1 = {
        {0,1,10},{0,2,3},{1,2,1},{1,3,2},{2,1,4},{2,3,8},{2,4,2},{3,4,7},{4,3,9}
    };
    auto r1 = dijkstra(n1, edges1, 0);
    std::cout << "Test 1 distances:\n"; printVec(r1.dist);
    std::cout << "Test 1 parents:\n"; printVec(r1.parent);

    // Test 2 (zero-weight)
    int n2 = 4;
    std::vector<std::tuple<int,int,long long>> edges2 = {
        {0,1,0},{0,2,5},{1,2,1},{1,3,4},{2,3,0}
    };
    auto r2 = dijkstra(n2, edges2, 0);
    std::cout << "Test 2 distances:\n"; printVec(r2.dist);
    std::cout << "Test 2 parents:\n"; printVec(r2.parent);

    // Test 3 (disconnected)
    int n3 = 5;
    std::vector<std::tuple<int,int,long long>> edges3 = {
        {0,1,2},{1,2,2}
    };
    auto r3 = dijkstra(n3, edges3, 0);
    std::cout << "Test 3 distances:\n"; printVec(r3.dist);
    std::cout << "Test 3 parents:\n"; printVec(r3.parent);

    // Test 4 (undirected classic, both directions)
    int n4 = 6;
    std::vector<std::tuple<int,int,long long>> edges4 = {
        {0,1,7},{1,0,7},{0,2,9},{2,0,9},{0,5,14},{5,0,14},
        {1,2,10},{2,1,10},{1,3,15},{3,1,15},{2,3,11},{3,2,11},
        {2,5,2},{5,2,2},{3,4,6},{4,3,6},{4,5,9},{5,4,9}
    };
    auto r4 = dijkstra(n4, edges4, 0);
    std::cout << "Test 4 distances:\n"; printVec(r4.dist);
    std::cout << "Test 4 parents:\n"; printVec(r4.parent);

    // Test 5 (directed asymmetric)
    int n5 = 5;
    std::vector<std::tuple<int,int,long long>> edges5 = {
        {0,1,2},{1,2,3},{0,2,10},{2,3,1},{3,4,1},{1,4,100}
    };
    auto r5 = dijkstra(n5, edges5, 0);
    std::cout << "Test 5 distances:\n"; printVec(r5.dist);
    std::cout << "Test 5 parents:\n"; printVec(r5.parent);

    return 0;
}