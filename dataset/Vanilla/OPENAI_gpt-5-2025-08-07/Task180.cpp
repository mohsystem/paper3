#include <bits/stdc++.h>
using namespace std;

vector<pair<int,int>> reparent(int n, const vector<pair<int,int>>& edges, int root) {
    vector<vector<int>> g(n);
    g.reserve(n);
    for (auto &e : edges) {
        int u = e.first, v = e.second;
        g[u].push_back(v);
        g[v].push_back(u);
    }
    vector<char> vis(n, 0);
    queue<int> q;
    q.push(root);
    vis[root] = 1;
    vector<pair<int,int>> res;
    res.reserve(n - 1);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : g[u]) {
            if (!vis[v]) {
                vis[v] = 1;
                res.emplace_back(u, v);
                q.push(v);
            }
        }
    }
    return res;
}

void printDirectedEdges(const vector<pair<int,int>>& dedges) {
    cout << "[";
    for (size_t i = 0; i < dedges.size(); ++i) {
        cout << "(" << dedges[i].first << "->" << dedges[i].second << ")";
        if (i + 1 < dedges.size()) cout << ", ";
    }
    cout << "]\n";
}

int main() {
    // Test case 1
    int n1 = 10;
    vector<pair<int,int>> edges1 = {
        {0,1},{0,2},{0,3},{1,4},{1,5},{2,6},{2,7},{3,8},{3,9}
    };
    cout << "Test 1: root=6\n";
    printDirectedEdges(reparent(n1, edges1, 6));

    // Test case 2
    cout << "Test 2: root=0\n";
    printDirectedEdges(reparent(n1, edges1, 0));

    // Test case 3
    int n3 = 5;
    vector<pair<int,int>> edges3 = {{0,1},{1,2},{2,3},{3,4}};
    cout << "Test 3: root=3\n";
    printDirectedEdges(reparent(n3, edges3, 3));

    // Test case 4
    int n4 = 6;
    vector<pair<int,int>> edges4 = {{0,1},{0,2},{0,3},{0,4},{0,5}};
    cout << "Test 4: root=4\n";
    printDirectedEdges(reparent(n4, edges4, 4));

    // Test case 5
    int n5 = 7;
    vector<pair<int,int>> edges5 = {{0,1},{1,2},{1,3},{3,4},{2,5},{5,6}};
    cout << "Test 5: root=4\n";
    printDirectedEdges(reparent(n5, edges5, 4));

    return 0;
}