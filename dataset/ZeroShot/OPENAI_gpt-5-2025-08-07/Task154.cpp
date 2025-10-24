#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>

class Task154 {
public:
    // Performs DFS on a graph with n nodes, adjacency list "graph", starting from "start"
    // Returns the traversal order as a vector of node indices.
    static std::vector<int> dfs(int n, const std::vector<std::vector<int>>& graph, int start) {
        std::vector<int> traversal;
        if (n <= 0 || static_cast<int>(graph.size()) != n || start < 0 || start >= n) {
            return traversal;
        }

        // Sanitize adjacency: bounds-check, deduplicate, sort ascending
        std::vector<std::vector<int>> adj(n);
        for (int i = 0; i < n; ++i) {
            std::vector<char> seen(n, 0);
            if (i < static_cast<int>(graph.size())) {
                for (int v : graph[i]) {
                    if (v >= 0 && v < n && !seen[v]) {
                        seen[v] = 1;
                    }
                }
            }
            for (int v = 0; v < n; ++v) {
                if (seen[v]) adj[i].push_back(v);
            }
            // already sorted due to scan order
        }

        std::vector<char> visited(n, 0);
        std::stack<int> st;
        st.push(start);

        while (!st.empty()) {
            int v = st.top();
            st.pop();
            if (v < 0 || v >= n) continue;
            if (!visited[v]) {
                visited[v] = 1;
                traversal.push_back(v);
                // push neighbors in reverse order to visit ascending
                const auto& neighbors = adj[v];
                for (int i = static_cast<int>(neighbors.size()) - 1; i >= 0; --i) {
                    int nb = neighbors[i];
                    if (!visited[nb]) st.push(nb);
                }
            }
        }

        return traversal;
    }
};

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    // Test 1: Simple chain 0-1-2
    {
        int n = 3;
        std::vector<std::vector<int>> g = {
            {1},
            {0, 2},
            {1}
        };
        printVec(Task154::dfs(n, g, 0)); // Expected: [0, 1, 2]
    }

    // Test 2: Cycle 0-1-2-3-0
    {
        int n = 4;
        std::vector<std::vector<int>> g = {
            {1, 3},
            {0, 2},
            {1, 3},
            {2, 0}
        };
        printVec(Task154::dfs(n, g, 1));
    }

    // Test 3: Disconnected graph
    {
        int n = 5;
        std::vector<std::vector<int>> g = {
            {},
            {2},
            {1},
            {4},
            {3}
        };
        printVec(Task154::dfs(n, g, 3)); // Expected: [3, 4]
    }

    // Test 4: Self-loop and out-of-range neighbor
    {
        int n = 4;
        std::vector<std::vector<int>> g = {
            {0, 1},
            {0, 2, 4}, // 4 ignored
            {1, 3},
            {2}
        };
        printVec(Task154::dfs(n, g, 0)); // Expected: [0, 1, 2, 3]
    }

    // Test 5: Star graph centered at 0
    {
        int n = 6;
        std::vector<std::vector<int>> g = {
            {1, 2, 3, 4, 5},
            {0},
            {0},
            {0},
            {0},
            {0}
        };
        printVec(Task154::dfs(n, g, 2));
    }

    return 0;
}