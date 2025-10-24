
#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <cstring>
#include <stdexcept>

// Maximum nodes to prevent excessive memory allocation
const int MAX_NODES = 10000;
const int MAX_EDGES = 100000;

// Structure to represent an edge in the adjacency list
struct Edge {
    int destination;
    int weight;
    
    Edge(int dest, int w) : destination(dest), weight(w) {}
};

// Comparator for priority queue (min-heap based on distance)
struct NodeDistance {
    int node;
    int distance;
    
    NodeDistance(int n, int d) : node(n), distance(d) {}
    
    // Greater comparator for min-heap
    bool operator>(const NodeDistance& other) const {
        return distance > other.distance;
    }
};

class DijkstraGraph {
private:
    int numNodes;
    std::vector<std::vector<Edge>> adjacencyList;
    
public:
    // Constructor with input validation
    DijkstraGraph(int nodes) {
        // Validate node count to prevent excessive memory allocation
        if (nodes <= 0 || nodes > MAX_NODES) {
            throw std::invalid_argument("Number of nodes must be between 1 and 10000");
        }
        numNodes = nodes;
        adjacencyList.resize(numNodes);
    }
    
    // Add edge with validation
    void addEdge(int source, int destination, int weight) {
        // Validate inputs to prevent out-of-bounds access
        if (source < 0 || source >= numNodes) {
            throw std::out_of_range("Source node out of range");
        }
        if (destination < 0 || destination >= numNodes) {
            throw std::out_of_range("Destination node out of range");
        }
        if (weight < 0) {
            throw std::invalid_argument("Weight must be non-negative for Dijkstra's algorithm");
        }
        
        // Check edge count to prevent excessive memory usage
        size_t totalEdges = 0;
        for (const auto& edges : adjacencyList) {
            totalEdges += edges.size();
        }
        if (totalEdges >= MAX_EDGES) {
            throw std::runtime_error("Maximum edge limit exceeded");
        }
        
        adjacencyList[source].push_back(Edge(destination, weight));
    }
    
    // Dijkstra's algorithm implementation
    std::vector<int> dijkstra(int startNode) {
        // Validate start node
        if (startNode < 0 || startNode >= numNodes) {
            throw std::out_of_range("Start node out of range");
        }
        
        // Initialize distances with maximum value (infinity)
        std::vector<int> distances(numNodes, std::numeric_limits<int>::max());
        std::vector<bool> visited(numNodes, false);
        
        // Priority queue for selecting minimum distance node
        std::priority_queue<NodeDistance, std::vector<NodeDistance>, std::greater<NodeDistance>> pq;
        
        // Distance to start node is 0
        distances[startNode] = 0;
        pq.push(NodeDistance(startNode, 0));
        
        while (!pq.empty()) {
            NodeDistance current = pq.top();
            pq.pop();
            
            int currentNode = current.node;
            
            // Skip if already visited
            if (visited[currentNode]) {
                continue;
            }
            
            visited[currentNode] = true;
            
            // Explore neighbors with bounds checking
            const std::vector<Edge>& edges = adjacencyList[currentNode];
            for (size_t i = 0; i < edges.size(); ++i) {
                const Edge& edge = edges[i];
                int neighbor = edge.destination;
                int weight = edge.weight;
                
                // Bounds check for neighbor (defense in depth)
                if (neighbor < 0 || neighbor >= numNodes) {
                    continue;
                }
                
                // Check for integer overflow before addition
                if (distances[currentNode] != std::numeric_limits<int>::max() &&
                    weight <= std::numeric_limits<int>::max() - distances[currentNode]) {
                    
                    int newDistance = distances[currentNode] + weight;
                    
                    // Relaxation step
                    if (newDistance < distances[neighbor]) {
                        distances[neighbor] = newDistance;
                        pq.push(NodeDistance(neighbor, newDistance));
                    }
                }
            }
        }
        
        return distances;
    }
    
    int getNumNodes() const {
        return numNodes;
    }
};

int main() {
    try {
        // Test Case 1: Simple graph
        std::cout << "Test Case 1: Simple graph" << std::endl;
        DijkstraGraph graph1(5);
        graph1.addEdge(0, 1, 4);
        graph1.addEdge(0, 2, 1);
        graph1.addEdge(2, 1, 2);
        graph1.addEdge(1, 3, 1);
        graph1.addEdge(2, 3, 5);
        graph1.addEdge(3, 4, 3);
        
        std::vector<int> distances1 = graph1.dijkstra(0);
        for (int i = 0; i < graph1.getNumNodes(); ++i) {
            std::cout << "Distance from 0 to " << i << ": ";
            if (distances1[i] == std::numeric_limits<int>::max()) {
                std::cout << "INF" << std::endl;
            } else {
                std::cout << distances1[i] << std::endl;
            }
        }
        
        // Test Case 2: Disconnected graph
        std::cout << "\\nTest Case 2: Disconnected graph" << std::endl;
        DijkstraGraph graph2(4);
        graph2.addEdge(0, 1, 5);
        graph2.addEdge(1, 2, 3);
        // Node 3 is disconnected
        
        std::vector<int> distances2 = graph2.dijkstra(0);
        for (int i = 0; i < graph2.getNumNodes(); ++i) {
            std::cout << "Distance from 0 to " << i << ": ";
            if (distances2[i] == std::numeric_limits<int>::max()) {
                std::cout << "INF" << std::endl;
            } else {
                std::cout << distances2[i] << std::endl;
            }
        }
        
        // Test Case 3: Single node
        std::cout << "\\nTest Case 3: Single node" << std::endl;
        DijkstraGraph graph3(1);
        std::vector<int> distances3 = graph3.dijkstra(0);
        std::cout << "Distance from 0 to 0: " << distances3[0] << std::endl;
        
        // Test Case 4: Multiple paths
        std::cout << "\\nTest Case 4: Multiple paths" << std::endl;
        DijkstraGraph graph4(6);
        graph4.addEdge(0, 1, 7);
        graph4.addEdge(0, 2, 9);
        graph4.addEdge(0, 5, 14);
        graph4.addEdge(1, 2, 10);
        graph4.addEdge(1, 3, 15);
        graph4.addEdge(2, 3, 11);
        graph4.addEdge(2, 5, 2);
        graph4.addEdge(3, 4, 6);
        graph4.addEdge(4, 5, 9);
        
        std::vector<int> distances4 = graph4.dijkstra(0);
        for (int i = 0; i < graph4.getNumNodes(); ++i) {
            std::cout << "Distance from 0 to " << i << ": " << distances4[i] << std::endl;
        }
        
        // Test Case 5: Linear chain
        std::cout << "\\nTest Case 5: Linear chain" << std::endl;
        DijkstraGraph graph5(4);
        graph5.addEdge(0, 1, 1);
        graph5.addEdge(1, 2, 2);
        graph5.addEdge(2, 3, 3);
        
        std::vector<int> distances5 = graph5.dijkstra(0);
        for (int i = 0; i < graph5.getNumNodes(); ++i) {
            std::cout << "Distance from 0 to " << i << ": " << distances5[i] << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
