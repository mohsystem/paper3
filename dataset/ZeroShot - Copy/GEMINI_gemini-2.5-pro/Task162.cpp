#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <stdexcept>

// Use a type alias for clarity. Graph stores pairs of {destination, weight}.
using Edge = std::pair<int, int>;
using Graph = std::vector<std::vector<Edge>>;

/**
 * @brief Finds the shortest paths from a starting node using Dijkstra's algorithm.
 * 
 * @param adj The graph represented as an adjacency list.
 * @param numNodes The total number of nodes in the graph.
 * @param startNode The source node.
 * @return A vector of long long containing the shortest distances from the startNode.
 * @throw std::invalid_argument if inputs are invalid or graph has negative weights.
 */
std::vector<long long> dijkstra(const Graph& adj, int numNodes, int startNode) {
    // Input validation
    if (numNodes <= 0 || startNode < 0 || startNode >= numNodes) {
        throw std::invalid_argument("Invalid input for Dijkstra's algorithm.");
    }

    // Use long long for distances to prevent overflow. Initialize with infinity.
    const long long INF = std::numeric_limits<long long>::max();
    std::vector<long long> distances(numNodes, INF);
    distances[startNode] = 0;

    // Min-priority queue storing {distance, vertex}.
    // std::greater makes it a min-heap.
    std::priority_queue<std::pair<long long, int>, 
                        std::vector<std::pair<long long, int>>, 
                        std::greater<std::pair<long long, int>>> pq;

    pq.push({0, startNode});

    while (!pq.empty()) {
        long long currentDist = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        // If we've found a shorter path already, skip this one.
        if (currentDist > distances[u]) {
            continue;
        }

        // Iterate through all adjacent vertices
        for (const auto& edge : adj[u]) {
            int v = edge.first;
            int weight = edge.second;

            // Security check: Dijkstra's does not support negative weights.
            if (weight < 0) {
                throw std::invalid_argument("Graph contains a negative edge weight.");
            }
            
            // Relaxation step: if a shorter path to v is found through u.
            if (distances[u] != INF && distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                pq.push({distances[v], v});
            }
        }
    }

    return distances;
}

void runTest(const Graph& adj, int numNodes, int startNode, int testCaseNum) {
    std::cout << "\nTest Case " << testCaseNum << ":" << std::endl;
    try {
        std::vector<long long> distances = dijkstra(adj, numNodes, startNode);
        std::cout << "Shortest distances from node " << startNode << ":" << std::endl;
        for (size_t i = 0; i < distances.size(); ++i) {
            std::cout << "  to node " << i << ": ";
            if (distances[i] == std::numeric_limits<long long>::max()) {
                std::cout << "Infinity" << std::endl;
            } else {
                std::cout << distances[i] << std::endl;
            }
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}


int main() {
    std::cout << "C++ Dijkstra's Algorithm Test Cases:" << std::endl;

    // Test Case 1: Simple graph
    int numNodes1 = 5;
    Graph adj1(numNodes1);
    adj1[0].push_back({1, 10}); adj1[0].push_back({4, 3});
    adj1[1].push_back({2, 2});
    adj1[2].push_back({3, 9});
    adj1[4].push_back({1, 4}); adj1[4].push_back({2, 8}); adj1[4].push_back({3, 2});
    runTest(adj1, numNodes1, 0, 1);

    // Test Case 2: Disconnected graph
    int numNodes2 = 5;
    Graph adj2(numNodes2);
    adj2[0].push_back({1, 1});
    adj2[1].push_back({0, 1});
    adj2[2].push_back({3, 2});
    adj2[3].push_back({2, 2});
    runTest(adj2, numNodes2, 0, 2);

    // Test Case 3: Linear graph
    int numNodes3 = 4;
    Graph adj3(numNodes3);
    adj3[0].push_back({1, 5});
    adj3[1].push_back({2, 5});
    adj3[2].push_back({3, 5});
    runTest(adj3, numNodes3, 0, 3);
    
    // Test Case 4: Graph with a cycle
    int numNodes4 = 4;
    Graph adj4(numNodes4);
    adj4[0].push_back({1, 1});
    adj4[1].push_back({2, 2}); adj4[1].push_back({3, 4});
    adj4[2].push_back({0, 3});
    runTest(adj4, numNodes4, 0, 4);

    // Test Case 5: Start node with no outgoing edges
    int numNodes5 = 3;
    Graph adj5(numNodes5);
    adj5[1].push_back({2, 10});
    runTest(adj5, numNodes5, 0, 5);
    
    return 0;
}