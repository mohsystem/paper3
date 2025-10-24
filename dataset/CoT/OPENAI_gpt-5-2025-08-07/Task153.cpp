// Secure BFS implementation in C++ with test cases.
#include <bits/stdc++.h>
using namespace std;

// Performs BFS on a graph represented as adjacency list (0..n-1). Returns visit order.
// If start is out of range, returns vector with just 'start'.
vector<int> bfs(const vector<vector<int>>& graph, int start) {
    vector<int> order;
    int n = static_cast<int>(graph.size());
    if (start < 0 || start >= n) {
        order.push_back(start);
        return order;
    }

    vector<char> visited(n, 0);
    queue<int> q;
    visited[start] = 1;
    q.push(start);

    while (!q.empty()) {
        int cur = q.front(); q.pop();
        order.push_back(cur);
        const auto& neighbors = graph[cur];
        for (int nb : neighbors) {
            if (nb < 0 || nb >= n) continue; // defensive bound check
            if (!visited[nb]) {
                visited[nb] = 1;
                q.push(nb);
            }
        }
    }
    return order;
}

static void print_result(const string& label, const vector<int>& v) {
    cout << label << ": [";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ", ";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    // Test 1: Simple chain 0-1-2-3 (undirected)
    {
        vector<vector<int>> g(4);
        auto add_undirected = [&](int u, int v){
            if (u>=0 && u<4 && v>=0 && v<4) {
                if (find(g[u].begin(), g[u].end(), v) == g[u].end()) g[u].push_back(v);
                if (find(g[v].begin(), g[v].end(), u) == g[v].end()) g[v].push_back(u);
            }
        };
        add_undirected(0,1);
        add_undirected(1,2);
        add_undirected(2,3);
        print_result("Test1", bfs(g, 0)); // expected [0,1,2,3]
    }

    // Test 2: Cycle 0-1-2-0 with branch 1-3 (undirected)
    {
        vector<vector<int>> g(4);
        auto add_undirected = [&](int u, int v){
            if (find(g[u].begin(), g[u].end(), v) == g[u].end()) g[u].push_back(v);
            if (find(g[v].begin(), g[v].end(), u) == g[v].end()) g[v].push_back(u);
        };
        add_undirected(0,1);
        add_undirected(1,2);
        add_undirected(2,0);
        add_undirected(1,3);
        print_result("Test2", bfs(g, 0)); // expected [0,1,2,3] or similar
    }

    // Test 3: Disconnected with isolated node 5
    {
        vector<vector<int>> g(6); // 0..5, node 5 isolated
        print_result("Test3", bfs(g, 5)); // expected [5]
    }

    // Test 4: Ordered neighbors: 0 -> [3,1,2], deeper nodes
    {
        vector<vector<int>> g(6);
        g[0] = {3,1,2};
        g[1] = {4};
        g[2] = {};
        g[3] = {5};
        g[4] = {};
        g[5] = {};
        print_result("Test4", bfs(g, 0)); // expected [0,3,1,2,5,4]
    }

    // Test 5: Start not present in graph
    {
        vector<vector<int>> g(2);
        g[0] = {1};
        g[1] = {0};
        print_result("Test5", bfs(g, 99)); // expected [99]
    }

    return 0;
}