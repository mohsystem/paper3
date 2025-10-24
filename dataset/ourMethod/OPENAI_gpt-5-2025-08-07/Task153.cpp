#include <iostream>
#include <vector>
#include <queue>
#include <stdexcept>
#include <string>

static void validateGraph(int n, const std::vector<std::vector<int>>& adj, int start) {
    if (n < 0) {
        throw std::invalid_argument("n must be >= 0");
    }
    if (static_cast<int>(adj.size()) != n) {
        throw std::invalid_argument("adjacency size must equal n");
    }
    if (n == 0) {
        throw std::invalid_argument("graph has no nodes");
    }
    if (start < 0 || start >= n) {
        throw std::invalid_argument("start node out of range");
    }
    for (int i = 0; i < n; ++i) {
        const auto& neighbors = adj[i];
        for (int v : neighbors) {
            if (v < 0 || v >= n) {
                throw std::invalid_argument("edge from " + std::to_string(i) + " to invalid node " + std::to_string(v));
            }
        }
    }
}

std::vector<int> bfs(int n, const std::vector<std::vector<int>>& adj, int start) {
    validateGraph(n, adj, start);
    std::vector<int> order;
    order.reserve(n);
    std::vector<char> visited(n, 0);
    std::queue<int> q;
    visited[start] = 1;
    q.push(start);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);
        for (int v : adj[u]) {
            if (!visited[v]) {
                visited[v] = 1;
                q.push(v);
            }
        }
    }
    return order;
}

static void printOrder(const std::vector<int>& order) {
    std::cout << "[";
    for (size_t i = 0; i < order.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << order[i];
    }
    std::cout << "]\n";
}

int main() {
    // Test case 1: Simple line graph 0-1-2-3 from 0
    try {
        int n1 = 4;
        std::vector<std::vector<int>> adj1 = {
            {1}, {2}, {3}, {}
        };
        auto res1 = bfs(n1, adj1, 0);
        std::cout << "Test 1: ";
        printOrder(res1);
    } catch (const std::exception& e) {
        std::cout << "Test 1 Error: " << e.what() << "\n";
    }

    // Test case 2: Graph with cycles
    try {
        int n2 = 4;
        std::vector<std::vector<int>> adj2 = {
            {1, 2}, // 0
            {2},    // 1
            {0, 3}, // 2
            {}      // 3
        };
        auto res2 = bfs(n2, adj2, 1);
        std::cout << "Test 2: ";
        printOrder(res2);
    } catch (const std::exception& e) {
        std::cout << "Test 2 Error: " << e.what() << "\n";
    }

    // Test case 3: Disconnected graph start at 3
    try {
        int n3 = 5;
        std::vector<std::vector<int>> adj3 = {
            {1}, {2}, {}, {4}, {}
        };
        auto res3 = bfs(n3, adj3, 3);
        std::cout << "Test 3: ";
        printOrder(res3);
    } catch (const std::exception& e) {
        std::cout << "Test 3 Error: " << e.what() << "\n";
    }

    // Test case 4: Invalid start node
    try {
        int n4 = 3;
        std::vector<std::vector<int>> adj4 = {
            {1}, {2}, {}
        };
        auto res4 = bfs(n4, adj4, 5);
        std::cout << "Test 4: ";
        printOrder(res4);
    } catch (const std::exception& e) {
        std::cout << "Test 4 Error: " << e.what() << "\n";
    }

    // Test case 5: Invalid edge reference
    try {
        int n5 = 3;
        std::vector<std::vector<int>> adj5 = {
            {1, 99}, {2}, {}
        };
        auto res5 = bfs(n5, adj5, 0);
        std::cout << "Test 5: ";
        printOrder(res5);
    } catch (const std::exception& e) {
        std::cout << "Test 5 Error: " << e.what() << "\n";
    }

    return 0;
}