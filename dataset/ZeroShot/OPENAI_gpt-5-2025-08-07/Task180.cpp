#include <bits/stdc++.h>
using namespace std;

static bool validate_tree(int n, const vector<pair<int,int>>& edges, int root, string &err) {
    if (n < 1) { err = "Number of nodes must be >= 1"; return false; }
    if (root < 0 || root >= n) { err = "Root out of range"; return false; }
    if ((int)edges.size() != n - 1) { err = "Edges must be n-1 for a valid tree"; return false; }
    unordered_set<long long> seen;
    auto keyOf = [](int a, int b) -> long long {
        long long x = min(a,b), y = max(a,b);
        return (x << 32) ^ y;
    };
    for (size_t i = 0; i < edges.size(); ++i) {
        int u = edges[i].first, v = edges[i].second;
        if (u < 0 || u >= n || v < 0 || v >= n) { err = "Edge node out of range"; return false; }
        if (u == v) { err = "Self-loop detected"; return false; }
        long long k = keyOf(u, v);
        if (!seen.insert(k).second) { err = "Duplicate edge detected"; return false; }
    }
    err.clear();
    return true;
}

// Returns oriented children adjacency list. Returns empty on invalid input.
vector<vector<int>> reparent_tree(int n, const vector<pair<int,int>>& edges, int root) {
    string err;
    if (!validate_tree(n, edges, root, err)) {
        return {};
    }

    vector<vector<int>> adj(n);
    for (auto &e : edges) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }

    vector<int> parent(n, -1);
    queue<int> q;
    parent[root] = root;
    q.push(root);

    vector<vector<int>> children(n);
    int visited = 0;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        visited++;
        for (int v : adj[u]) {
            if (v != parent[u]) {
                parent[v] = u;
                children[u].push_back(v);
                q.push(v);
            }
        }
    }
    if (visited != n) {
        return {};
    }
    return children;
}

static void print_children(const vector<vector<int>>& children) {
    if (children.empty()) {
        cout << "Invalid or empty result" << '\n';
        return;
    }
    for (size_t i = 0; i < children.size(); ++i) {
        cout << i << ": [";
        for (size_t j = 0; j < children[i].size(); ++j) {
            if (j) cout << ", ";
            cout << children[i][j];
        }
        cout << "]\n";
    }
}

int main() {
    // Test 1: Example tree reparented at 6
    int n1 = 10;
    vector<pair<int,int>> e1 = {
        {0,1},{0,2},{0,3},{1,4},{1,5},{2,6},{2,7},{3,8},{3,9}
    };
    int root1 = 6;
    cout << "Test 1:\n";
    auto r1 = reparent_tree(n1, e1, root1);
    print_children(r1);

    // Test 2: Chain 0-1-2-3 reparented at 3
    int n2 = 4;
    vector<pair<int,int>> e2 = { {0,1},{1,2},{2,3} };
    int root2 = 3;
    cout << "Test 2:\n";
    auto r2 = reparent_tree(n2, e2, root2);
    print_children(r2);

    // Test 3: Single node
    int n3 = 1;
    vector<pair<int,int>> e3 = {};
    int root3 = 0;
    cout << "Test 3:\n";
    auto r3 = reparent_tree(n3, e3, root3);
    print_children(r3);

    // Test 4: Star centered at 0 reparented at 3
    int n4 = 5;
    vector<pair<int,int>> e4 = { {0,1},{0,2},{0,3},{0,4} };
    int root4 = 3;
    cout << "Test 4:\n";
    auto r4 = reparent_tree(n4, e4, root4);
    print_children(r4);

    // Test 5: Invalid (wrong number of edges)
    int n5 = 3;
    vector<pair<int,int>> e5 = { {0,1} };
    int root5 = 0;
    cout << "Test 5:\n";
    auto r5 = reparent_tree(n5, e5, root5);
    print_children(r5);

    return 0;
}