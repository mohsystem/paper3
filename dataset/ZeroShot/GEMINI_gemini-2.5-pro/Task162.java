import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.PriorityQueue;

class Task162 {

    // Inner class to represent a node in the graph's adjacency list
    // and in the priority queue.
    // It stores the destination vertex and the weight of the edge or path.
    static class Node implements Comparable<Node> {
        public int vertex;
        public long distance; // Use long to prevent overflow

        public Node(int vertex, long distance) {
            this.vertex = vertex;
            this.distance = distance;
        }

        @Override
        public int compareTo(Node other) {
            return Long.compare(this.distance, other.distance);
        }
    }

    /**
     * Finds the shortest paths from a starting node to all other nodes in a weighted graph
     * using Dijkstra's algorithm.
     *
     * @param adj       Adjacency list representation of the graph. adj.get(u) contains a list of Nodes (v, weight).
     * @param numNodes  The total number of nodes in the graph, indexed from 0 to numNodes-1.
     * @param startNode The starting node for the shortest path calculation.
     * @return An array of long integers where the i-th element is the shortest distance from startNode to node i.
     *         Returns Long.MAX_VALUE if a node is unreachable.
     * @throws IllegalArgumentException if the input is invalid or if the graph contains negative edge weights.
     */
    public static long[] dijkstra(List<List<Node>> adj, int numNodes, int startNode) {
        // Input validation
        if (adj == null || numNodes <= 0 || startNode < 0 || startNode >= numNodes) {
            throw new IllegalArgumentException("Invalid input for Dijkstra's algorithm.");
        }

        long[] distances = new long[numNodes];
        Arrays.fill(distances, Long.MAX_VALUE);
        distances[startNode] = 0;

        // Min-priority queue to store nodes to visit, ordered by distance.
        PriorityQueue<Node> pq = new PriorityQueue<>();
        pq.add(new Node(startNode, 0));

        while (!pq.isEmpty()) {
            Node currentNode = pq.poll();
            int u = currentNode.vertex;
            long currentDist = currentNode.distance;

            // If we have already found a shorter path to u, we skip this one.
            if (currentDist > distances[u]) {
                continue;
            }

            // Iterate over all neighbors of the current node.
            for (Node neighborNode : adj.get(u)) {
                int v = neighborNode.vertex;
                long weight = neighborNode.distance; // Here 'distance' in Node represents edge weight

                // Security check: Dijkstra's algorithm does not work with negative weights.
                if (weight < 0) {
                    throw new IllegalArgumentException("Graph contains negative edge weights.");
                }
                
                // Relaxation step: if we found a shorter path to v through u.
                if (distances[u] != Long.MAX_VALUE && distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    pq.add(new Node(v, distances[v]));
                }
            }
        }

        return distances;
    }
    
    // Main method with test cases
    public static void main(String[] args) {
        System.out.println("Java Dijkstra's Algorithm Test Cases:");

        // Test Case 1: Simple graph
        int numNodes1 = 5;
        List<List<Node>> adj1 = new ArrayList<>();
        for (int i = 0; i < numNodes1; i++) {
            adj1.add(new ArrayList<>());
        }
        adj1.get(0).add(new Node(1, 10));
        adj1.get(0).add(new Node(4, 3));
        adj1.get(1).add(new Node(2, 2));
        adj1.get(2).add(new Node(3, 9));
        adj1.get(4).add(new Node(1, 4));
        adj1.get(4).add(new Node(2, 8));
        adj1.get(4).add(new Node(3, 2));
        runTest(adj1, numNodes1, 0, 1);

        // Test Case 2: Disconnected graph
        int numNodes2 = 5;
        List<List<Node>> adj2 = new ArrayList<>();
        for (int i = 0; i < numNodes2; i++) {
            adj2.add(new ArrayList<>());
        }
        adj2.get(0).add(new Node(1, 1));
        adj2.get(1).add(new Node(0, 1));
        adj2.get(2).add(new Node(3, 2));
        adj2.get(3).add(new Node(2, 2));
        runTest(adj2, numNodes2, 0, 2);
        
        // Test Case 3: Linear graph
        int numNodes3 = 4;
        List<List<Node>> adj3 = new ArrayList<>();
        for (int i = 0; i < numNodes3; i++) {
            adj3.add(new ArrayList<>());
        }
        adj3.get(0).add(new Node(1, 5));
        adj3.get(1).add(new Node(2, 5));
        adj3.get(2).add(new Node(3, 5));
        runTest(adj3, numNodes3, 0, 3);

        // Test Case 4: Graph with a cycle
        int numNodes4 = 4;
        List<List<Node>> adj4 = new ArrayList<>();
        for (int i = 0; i < numNodes4; i++) {
            adj4.add(new ArrayList<>());
        }
        adj4.get(0).add(new Node(1, 1));
        adj4.get(1).add(new Node(2, 2));
        adj4.get(2).add(new Node(0, 3)); // Cycle back to start
        adj4.get(1).add(new Node(3, 4));
        runTest(adj4, numNodes4, 0, 4);

        // Test Case 5: Start node with no outgoing edges
        int numNodes5 = 3;
        List<List<Node>> adj5 = new ArrayList<>();
        for (int i = 0; i < numNodes5; i++) {
            adj5.add(new ArrayList<>());
        }
        adj5.get(1).add(new Node(2, 10));
        runTest(adj5, numNodes5, 0, 5);
    }
    
    private static void runTest(List<List<Node>> adj, int numNodes, int startNode, int testCaseNum) {
        System.out.println("\nTest Case " + testCaseNum + ":");
        try {
            long[] distances = dijkstra(adj, numNodes, startNode);
            System.out.println("Shortest distances from node " + startNode + ":");
            for (int i = 0; i < distances.length; i++) {
                System.out.println("  to node " + i + ": " + (distances[i] == Long.MAX_VALUE ? "Infinity" : distances[i]));
            }
        } catch (IllegalArgumentException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}