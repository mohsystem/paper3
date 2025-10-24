#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <utility>

std::vector<int> bfs(int n, const std::vector<std::pair<int,int>>& edges, int start, bool undirected) {
    std::vector<int> result;
    if (n <= 0 || start < 0 || start >= n) return result;

    std::vector<std::vector<int>> adj(n);
    for (const auto& e : edges) {
        int u = e.first, v = e.second;
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        adj[u].push_back(v);
        if (undirected && u != v) adj[v].push_back(u);
    }
    // Sort and deduplicate
    for (int i = 0; i < n; ++i) {
        auto& a = adj[i];
        std::sort(a.begin(), a.end());
        a.erase(std::unique(a.begin(), a.end()), a.end());
    }

    std::vector<char> visited(n, 0);
    std::queue<int> q;
    visited[start] = 1;
    q.push(start);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        result.push_back(u);
        for (int v : adj[u]) {
            if (!visited[v]) {
                visited[v] = 1;
                q.push(v);
            }
        }
    }
    return result;
}

static void print_vec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]";
}

int main() {
    // Test 1: Simple undirected chain
    {
        std::vector<std::pair<int,int>> edges = {{0,1},{1,2},{2,3}};
        auto res = bfs(4, edges, 0, true);
        std::cout << "Test 1: "; print_vec(res); std::cout << "\n";
    }
    // Test 2: Undirected graph with cycles
    {
        std::vector<std::pair<int,int>> edges = {{0,1},{0,2},{1,2},{2,3}};
        auto res = bfs(4, edges, 1, true);
        std::cout << "Test 2: "; print_vec(res); std::cout << "\n";
    }
    // Test 3: Disconnected graph
    {
        std::vector<std::pair<int,int>> edges = {{3,4}};
        auto res = bfs(5, edges, 3, true);
        std::cout << "Test 3: "; print_vec(res); std::cout << "\n";
    }
    // Test 4: Directed cycle
    {
        std::vector<std::pair<int,int>> edges = {{0,1},{1,2},{2,0}};
        auto res = bfs(3, edges, 1, false);
        std::cout << "Test 4: "; print_vec(res); std::cout << "\n";
    }
    // Test 5: Invalid edges and self-loop
    {
        std::vector<std::pair<int,int>> edges = {{-1,2},{1,4},{0,0}};
        auto res = bfs(4, edges, 0, true);
        std::cout << "Test 5: "; print_vec(res); std::cout << "\n";
    }
    return 0;
}