#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <limits>

using namespace std;

// Type alias for an edge represented as a pair (destination, weight)
using Edge = pair<int, int>;
// Type alias for the adjacency list
using AdjList = vector<vector<Edge>>;

/**
 * Implements Dijkstra's algorithm to find the shortest path from a starting node.
 *
 * @param V         The number of vertices in the graph.
 * @param adj       The adjacency list representation of the graph.
 * @param startNode The starting node.
 * @return A vector containing the shortest distances. Returns an empty vector on invalid input.
 */
vector<int> dijkstra(int V, const AdjList& adj, int startNode) {
    if (V <= 0 || startNode < 0 || startNode >= V) {
        cerr << "Error: Invalid input. V must be positive and startNode must be a valid index." << endl;
        return {};
    }

    vector<int> distances(V, numeric_limits<int>::max());
    distances[startNode] = 0;

    // Min-priority queue to store pairs of (distance, vertex)
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.push({0, startNode});

    while (!pq.empty()) {
        int u = pq.top().second;
        int dist = pq.top().first;
        pq.pop();

        // If we've found a shorter path already, skip
        if (dist > distances[u]) {
            continue;
        }

        for (const auto& edge : adj[u]) {
            int v = edge.first;
            int weight = edge.second;

            // Relaxation step
            if (distances[u] != numeric_limits<int>::max() && distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                pq.push({distances[v], v});
            }
        }
    }
    return distances;
}

void printDistances(const vector<int>& distances, int startNode) {
    if (distances.empty()) return;
    cout << "Shortest distances from source node " << startNode << ":" << endl;
    for (int i = 0; i < distances.size(); ++i) {
        cout << "Node " << i << ": ";
        if (distances[i] == numeric_limits<int>::max()) {
            cout << "Infinity" << endl;
        } else {
            cout << distances[i] << endl;
        }
    }
}

int main() {
    // Test Case 1: Standard case
    int V1 = 5;
    AdjList adj1(V1);
    adj1[0].push_back({1, 9});
    adj1[0].push_back({2, 6});
    adj1[0].push_back({3, 5});
    adj1[0].push_back({4, 3});
    adj1[2].push_back({1, 2});
    adj1[2].push_back({3, 4});
    int startNode1 = 0;
    cout << "Test Case 1: Standard graph from source 0" << endl;
    vector<int> distances1 = dijkstra(V1, adj1, startNode1);
    printDistances(distances1, startNode1);
    cout << endl;

    // Test Case 2: Graph with unreachable nodes
    int V2 = 6;
    AdjList adj2(V2);
    adj2[0].push_back({1, 7});
    adj2[0].push_back({2, 9});
    adj2[1].push_back({2, 10});
    adj2[1].push_back({3, 15});
    adj2[2].push_back({3, 11});
    // Node 4 and 5 are unreachable from 0
    adj2[4].push_back({5, 6});
    int startNode2 = 0;
    cout << "Test Case 2: Graph with unreachable nodes from source 0" << endl;
    vector<int> distances2 = dijkstra(V2, adj2, startNode2);
    printDistances(distances2, startNode2);
    cout << endl;
    
    // Test Case 3: Different starting node
    int startNode3 = 3;
    cout << "Test Case 3: Same as graph 1, but from source 3" << endl;
    // Re-using graph from test case 1, start node 3
    vector<int> distances3 = dijkstra(V1, adj1, startNode3);
    printDistances(distances3, startNode3);
    cout << endl;

    // Test Case 4: Linear graph
    int V4 = 4;
    AdjList adj4(V4);
    adj4[0].push_back({1, 10});
    adj4[1].push_back({2, 20});
    adj4[2].push_back({3, 30});
    int startNode4 = 0;
    cout << "Test Case 4: Linear graph from source 0" << endl;
    vector<int> distances4 = dijkstra(V4, adj4, startNode4);
    printDistances(distances4, startNode4);
    cout << endl;

    // Test Case 5: Invalid input
    cout << "Test Case 5: Invalid start node" << endl;
    vector<int> distances5 = dijkstra(V1, adj1, 10);
    if (distances5.empty()) {
        cout << "Function correctly handled invalid input by returning an empty vector." << endl;
    }
    
    return 0;
}