#include <bits/stdc++.h>
using namespace std;

vector<int> dfs(int n, const vector<pair<int,int>>& edges, int start, bool directed) {
    vector<vector<int>> adj(n);
    for (auto &e : edges) {
        int u = e.first, v = e.second;
        if (u < 0 || u >= n || v < 0 || v >= n) continue;
        adj[u].push_back(v);
        if (!directed) adj[v].push_back(u);
    }
    for (auto &nbrs : adj) sort(nbrs.begin(), nbrs.end());
    vector<int> order;
    if (start < 0 || start >= n) return order;
    vector<char> vis(n, 0);
    function<void(int)> rec = [&](int u){
        vis[u] = 1;
        order.push_back(u);
        for (int v : adj[u]) if (!vis[v]) rec(v);
    };
    rec(start);
    return order;
}

static void printVec(const string& label, const vector<int>& a) {
    cout << label << ": [";
    for (size_t i = 0; i < a.size(); ++i) {
        if (i) cout << ", ";
        cout << a[i];
    }
    cout << "]\n";
}

int main() {
    // Test 1: Line graph undirected
    int n1 = 5;
    vector<pair<int,int>> e1 = {{0,1},{1,2},{2,3},{3,4}};
    printVec("Test1", dfs(n1, e1, 0, false));

    // Test 2: Branching undirected
    int n2 = 6;
    vector<pair<int,int>> e2 = {{0,1},{0,2},{1,3},{1,4},{2,5}};
    printVec("Test2", dfs(n2, e2, 0, false));

    // Test 3: Cycle undirected
    int n3 = 4;
    vector<pair<int,int>> e3 = {{0,1},{1,2},{2,0},{2,3}};
    printVec("Test3", dfs(n3, e3, 0, false));

    // Test 4: Disconnected, start isolated
    int n4 = 5;
    vector<pair<int,int>> e4 = {{0,1},{1,2}};
    printVec("Test4", dfs(n4, e4, 3, false));

    // Test 5: Directed graph
    int n5 = 5;
    vector<pair<int,int>> e5 = {{0,1},{0,2},{1,3},{3,4}};
    printVec("Test5", dfs(n5, e5, 0, true));

    return 0;
}