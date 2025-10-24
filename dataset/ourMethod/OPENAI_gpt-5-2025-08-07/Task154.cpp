#include <iostream>
#include <vector>
#include <string>

struct DFSResult {
    bool ok;
    std::vector<int> order;
    std::string error;
};

static DFSResult dfsTraversal(int n, const std::vector<std::vector<int>>& adj, int start) {
    // Validate inputs
    if (n <= 0) {
        return DFSResult{false, {}, "Invalid number of nodes (must be > 0)."};
    }
    if (static_cast<int>(adj.size()) != n) {
        return DFSResult{false, {}, "Adjacency list size does not match node count."};
    }
    if (start < 0 || start >= n) {
        return DFSResult{false, {}, "Start node is out of range."};
    }
    for (int i = 0; i < n; ++i) {
        const auto& neighbors = adj[i];
        for (int v : neighbors) {
            if (v < 0 || v >= n) {
                return DFSResult{false, {}, "Edge from " + std::to_string(i) + " to out-of-range node " + std::to_string(v) + "."};
            }
        }
    }

    std::vector<char> visited(static_cast<size_t>(n), 0);
    std::vector<int> order;
    order.reserve(static_cast<size_t>(n));
    std::vector<int> stack;
    stack.reserve(static_cast<size_t>(n));
    stack.push_back(start);

    while (!stack.empty()) {
        int u = stack.back();
        stack.pop_back();
        if (visited[static_cast<size_t>(u)]) {
            continue;
        }
        visited[static_cast<size_t>(u)] = 1;
        order.push_back(u);
        const auto& neighbors = adj[static_cast<size_t>(u)];
        for (int i = static_cast<int>(neighbors.size()) - 1; i >= 0; --i) {
            int v = neighbors[static_cast<size_t>(i)];
            if (!visited[static_cast<size_t>(v)]) {
                stack.push_back(v);
            }
        }
    }

    return DFSResult{true, order, ""};
}

static std::vector<std::vector<int>> buildAdj(int n, const std::vector<std::pair<int,int>>& edges, bool undirected) {
    std::vector<std::vector<int>> adj(static_cast<size_t>(n));
    for (const auto& e : edges) {
        int u = e.first, v = e.second;
        if (u >= 0 && u < n && v >= 0 && v < n) {
            adj[static_cast<size_t>(u)].push_back(v);
            if (undirected) {
                adj[static_cast<size_t>(v)].push_back(u);
            }
        }
    }
    return adj;
}

int main() {
    // Test 1: Undirected chain 0-1-2-3, start=0
    {
        int n = 4;
        std::vector<std::pair<int,int>> edges = {{0,1},{1,2},{2,3}};
        auto adj = buildAdj(n, edges, true);
        DFSResult res = dfsTraversal(n, adj, 0);
        std::cout << "Test 1: " << (res.ok ? "" : "Error: ");
        if (res.ok) {
            std::cout << "[";
            for (size_t i = 0; i < res.order.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << res.order[i];
            }
            std::cout << "]";
        } else {
            std::cout << res.error;
        }
        std::cout << "\n";
    }

    // Test 2: Directed cycle 0->1->2->0 and 2->3, start=0
    {
        int n = 4;
        std::vector<std::pair<int,int>> edges = {{0,1},{1,2},{2,0},{2,3}};
        auto adj = buildAdj(n, edges, false);
        DFSResult res = dfsTraversal(n, adj, 0);
        std::cout << "Test 2: " << (res.ok ? "" : "Error: ");
        if (res.ok) {
            std::cout << "[";
            for (size_t i = 0; i < res.order.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << res.order[i];
            }
            std::cout << "]";
        } else {
            std::cout << res.error;
        }
        std::cout << "\n";
    }

    // Test 3: Disconnected graph, start in second component
    {
        int n = 5;
        std::vector<std::pair<int,int>> edges = {{0,1},{1,0},{2,3},{3,2}};
        auto adj = buildAdj(n, edges, false);
        DFSResult res = dfsTraversal(n, adj, 2);
        std::cout << "Test 3: " << (res.ok ? "" : "Error: ");
        if (res.ok) {
            std::cout << "[";
            for (size_t i = 0; i < res.order.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << res.order[i];
            }
            std::cout << "]";
        } else {
            std::cout << res.error;
        }
        std::cout << "\n";
    }

    // Test 4: Single node graph
    {
        int n = 1;
        std::vector<std::pair<int,int>> edges = {};
        auto adj = buildAdj(n, edges, false);
        DFSResult res = dfsTraversal(n, adj, 0);
        std::cout << "Test 4: " << (res.ok ? "" : "Error: ");
        if (res.ok) {
            std::cout << "[";
            for (size_t i = 0; i < res.order.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << res.order[i];
            }
            std::cout << "]";
        } else {
            std::cout << res.error;
        }
        std::cout << "\n";
    }

    // Test 5: Invalid start node
    {
        int n = 3;
        std::vector<std::pair<int,int>> edges = {{0,1},{1,2}};
        auto adj = buildAdj(n, edges, false);
        DFSResult res = dfsTraversal(n, adj, 5);
        std::cout << "Test 5: " << (res.ok ? "" : "Error: ");
        if (res.ok) {
            std::cout << "[";
            for (size_t i = 0; i < res.order.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << res.order[i];
            }
            std::cout << "]";
        } else {
            std::cout << res.error;
        }
        std::cout << "\n";
    }

    return 0;
}