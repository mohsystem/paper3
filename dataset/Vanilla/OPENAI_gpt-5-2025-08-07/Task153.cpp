#include <bits/stdc++.h>
using namespace std;

class Task153 {
public:
    static vector<int> bfs(int n, const vector<pair<int,int>>& edges, int start) {
        if (n <= 0 || start < 0 || start >= n) return {};
        vector<vector<int>> adj(n);
        for (auto &e : edges) {
            int u = e.first, v = e.second;
            if (0 <= u && u < n && 0 <= v && v < n) {
                adj[u].push_back(v);
                adj[v].push_back(u);
            }
        }
        for (auto &lst : adj) sort(lst.begin(), lst.end());
        vector<int> order;
        order.reserve(n);
        vector<char> vis(n, false);
        deque<int> q;
        vis[start] = true;
        q.push_back(start);
        while (!q.empty()) {
            int u = q.front(); q.pop_front();
            order.push_back(u);
            for (int v : adj[u]) {
                if (!vis[v]) {
                    vis[v] = true;
                    q.push_back(v);
                }
            }
        }
        return order;
    }
};

int main() {
    auto print = [](const vector<int>& v){
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) cout << " ";
            cout << v[i];
        }
        cout << "\n";
    };

    vector<pair<int,int>> e1 = {{0,1},{1,2},{2,3}};
    print(Task153::bfs(4, e1, 0)); // 0 1 2 3

    vector<pair<int,int>> e2 = {{3,4},{0,1}};
    print(Task153::bfs(5, e2, 3)); // 3 4

    vector<pair<int,int>> e3 = {{0,1},{1,2},{2,0},{2,3}};
    print(Task153::bfs(4, e3, 1)); // 1 0 2 3

    vector<pair<int,int>> e4 = {{0,1},{1,2},{2,3},{3,0},{0,2},{4,5}};
    print(Task153::bfs(6, e4, 2)); // 2 0 1 3

    vector<pair<int,int>> e5 = {};
    print(Task153::bfs(1, e5, 0)); // 0

    return 0;
}