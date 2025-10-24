#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <stdexcept>

using namespace std;

// Depth-First Search (DFS) iterative
// n: number of nodes
// edges: vector of pairs (u, v)
// start: starting node
// directed: whether the graph is directed
// returns traversal order
vector<int> dfs(int n, const vector<pair<int,int>>& edges, int start, bool directed) {
    if (n <= 0) throw invalid_argument("Number of nodes must be positive.");
    if (start < 0 || start >= n) throw invalid_argument("Start node out of range.");

    vector<vector<int>> adj(n);
    for (size_t i = 0; i < edges.size(); ++i) {
        int u = edges[i].first;
        int v = edges[i].second;
        if (u < 0 || u >= n || v < 0 || v >= n) {
            throw invalid_argument("Edge vertex out of range at index " + to_string(i));
        }
        adj[u].push_back(v);
        if (!directed) {
            adj[v].push_back(u);
        }
    }

    for (int i = 0; i < n; ++i) {
        sort(adj[i].begin(), adj[i].end());
    }

    vector<bool> visited(n, false);
    vector<int> result;
    result.reserve(n);
    stack<int> st;
    st.push(start);

    while (!st.empty()) {
        int u = st.top(); st.pop();
        if (visited[u]) continue;
        visited[u] = true;
        result.push_back(u);

        // push neighbors in reverse order for ascending visitation
        for (int i = static_cast<int>(adj[u].size()) - 1; i >= 0; --i) {
            int v = adj[u][i];
            if (!visited[v]) st.push(v);
        }
    }

    return result;
}

static void printVec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ", ";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    try {
        // Test case 1: Simple chain undirected
        int n1 = 4;
        vector<pair<int,int>> edges1 = {{0,1},{1,2},{2,3}};
        cout << "Test 1: ";
        printVec(dfs(n1, edges1, 0, false));

        // Test case 2: Branching undirected
        int n2 = 5;
        vector<pair<int,int>> edges2 = {{0,1},{0,2},{1,3},{1,4}};
        cout << "Test 2: ";
        printVec(dfs(n2, edges2, 0, false));

        // Test case 3: Directed with cycle
        int n3 = 4;
        vector<pair<int,int>> edges3 = {{0,1},{1,2},{2,0},{2,3}};
        cout << "Test 3: ";
        printVec(dfs(n3, edges3, 0, true));

        // Test case 4: Isolated node
        int n4 = 5;
        vector<pair<int,int>> edges4 = {{0,1},{1,2}};
        cout << "Test 4: ";
        printVec(dfs(n4, edges4, 4, false));

        // Test case 5: Invalid start node
        int n5 = 3;
        vector<pair<int,int>> edges5 = {{0,1}};
        cout << "Test 5: ";
        printVec(dfs(n5, edges5, 7, false));
    } catch (const exception& ex) {
        cout << "Error: " << ex.what() << "\n";
    }
    return 0;
}