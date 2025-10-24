#include <bits/stdc++.h>
using namespace std;

// Chain-of-Through process:
// 1) Problem understanding:
//    Re-root an undirected tree at newRoot and return parent array with parent[newRoot] = -1.
// 2) Security requirements:
//    Validate input sizes, edge bounds, no self-loops, and connectivity.
// 3) Secure coding generation:
//    Use iterative DFS to avoid stack overflows; thorough validation; avoid UB by checking indices.
// 4) Code review:
//    Ensured checks and safe vector usage; handle invalid inputs by returning empty vector.
// 5) Secure code output:
//    Return empty vector on invalid input; main prints results or errors accordingly.

static vector<int> reparentTree(int n, const vector<pair<int,int>>& edges, int newRoot) {
    if (n <= 0) return {};
    if ((int)edges.size() != n - 1) return {};
    if (newRoot < 0 || newRoot >= n) return {};

    vector<vector<int>> adj(n);
    for (size_t i = 0; i < edges.size(); ++i) {
        int u = edges[i].first, v = edges[i].second;
        if (u < 0 || u >= n || v < 0 || v >= n) return {};
        if (u == v) return {};
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    vector<int> parent(n, INT_MIN);
    vector<int> stack;
    stack.reserve(n);
    parent[newRoot] = -1;
    stack.push_back(newRoot);

    while (!stack.empty()) {
        int u = stack.back();
        stack.pop_back();
        for (int v : adj[u]) {
            if (parent[v] == INT_MIN) {
                parent[v] = u;
                stack.push_back(v);
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        if (parent[i] == INT_MIN) return {};
    }

    return parent;
}

static void printArray(const vector<int>& arr) {
    if (arr.empty()) {
        cout << "Invalid input or not a tree" << '\n';
        return;
    }
    cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i) cout << ", ";
        cout << arr[i];
    }
    cout << "]\n";
}

int main() {
    // 5 test cases
    {
        int n = 10;
        vector<pair<int,int>> edges = {{0,1},{0,2},{0,3},{1,4},{1,5},{2,6},{2,7},{3,8},{3,9}};
        printArray(reparentTree(n, edges, 6));
    }
    {
        int n = 10;
        vector<pair<int,int>> edges = {{0,1},{0,2},{0,3},{1,4},{1,5},{2,6},{2,7},{3,8},{3,9}};
        printArray(reparentTree(n, edges, 0));
    }
    {
        int n = 5;
        vector<pair<int,int>> edges = {{0,1},{1,2},{2,3},{3,4}};
        printArray(reparentTree(n, edges, 4));
    }
    {
        int n = 6;
        vector<pair<int,int>> edges = {{0,1},{0,2},{0,3},{0,4},{0,5}};
        printArray(reparentTree(n, edges, 3));
    }
    {
        int n = 5;
        vector<pair<int,int>> edges = {{0,1},{1,2},{1,3},{3,4}};
        printArray(reparentTree(n, edges, 4));
    }
    return 0;
}