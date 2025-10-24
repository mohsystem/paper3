import java.util.*;

public class Task162 {

    // A class to represent an edge in the graph
    static class Edge {
        int destination;
        int weight;

        public Edge(int destination, int weight) {
            this.destination = destination;
            this.weight = weight;
        }
    }

    /**
     * Implements Dijkstra's algorithm to find the shortest path from a starting node.
     *
     * @param V         The number of vertices in the graph.
     * @param adj       The adjacency list representation of the graph.
     * @param startNode The starting node.
     * @return An array containing the shortest distances from the start node to all other nodes.
     */
    public static int[] dijkstra(int V, List<List<Edge>> adj, int startNode) {
        if (V <= 0 || startNode < 0 || startNode >= V) {
            throw new IllegalArgumentException("Invalid input: V must be positive and startNode must be a valid index.");
        }

        int[] distances = new int[V];
        Arrays.fill(distances, Integer.MAX_VALUE);
        distances[startNode] = 0;

        // Priority queue to store {distance, vertex}.
        // Java's PriorityQueue is a min-heap by default.
        PriorityQueue<int[]> pq = new PriorityQueue<>(Comparator.comparingInt(a -> a[0]));
        pq.add(new int[]{0, startNode});

        while (!pq.isEmpty()) {
            int[] current = pq.poll();
            int currentDist = current[0];
            int u = current[1];

            // If we've found a shorter path already, skip
            if (currentDist > distances[u]) {
                continue;
            }

            for (Edge edge : adj.get(u)) {
                int v = edge.destination;
                int weight = edge.weight;

                // Relaxation step
                if (distances[u] != Integer.MAX_VALUE && distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    pq.add(new int[]{distances[v], v});
                }
            }
        }
        return distances;
    }

    public static void main(String[] args) {
        // Test Case 1: Standard case
        int V1 = 5;
        List<List<Edge>> adj1 = new ArrayList<>(V1);
        for (int i = 0; i < V1; i++) {
            adj1.add(new ArrayList<>());
        }
        adj1.get(0).add(new Edge(1, 9));
        adj1.get(0).add(new Edge(2, 6));
        adj1.get(0).add(new Edge(3, 5));
        adj1.get(0).add(new Edge(4, 3));
        adj1.get(2).add(new Edge(1, 2));
        adj1.get(2).add(new Edge(3, 4));

        int startNode1 = 0;
        System.out.println("Test Case 1: Standard graph from source 0");
        int[] distances1 = dijkstra(V1, adj1, startNode1);
        printDistances(distances1, startNode1);
        System.out.println();

        // Test Case 2: Graph with unreachable nodes
        int V2 = 6;
        List<List<Edge>> adj2 = new ArrayList<>(V2);
        for (int i = 0; i < V2; i++) {
            adj2.add(new ArrayList<>());
        }
        adj2.get(0).add(new Edge(1, 7));
        adj2.get(0).add(new Edge(2, 9));
        adj2.get(1).add(new Edge(2, 10));
        adj2.get(1).add(new Edge(3, 15));
        adj2.get(2).add(new Edge(3, 11));
        // Node 4 and 5 are unreachable from 0
        adj2.get(4).add(new Edge(5, 6));

        int startNode2 = 0;
        System.out.println("Test Case 2: Graph with unreachable nodes from source 0");
        int[] distances2 = dijkstra(V2, adj2, startNode2);
        printDistances(distances2, startNode2);
        System.out.println();

        // Test Case 3: Different starting node
        int startNode3 = 3;
        System.out.println("Test Case 3: Same as graph 1, but from source 3");
        // Re-using graph from test case 1, start node 3
        int[] distances3 = dijkstra(V1, adj1, startNode3);
        printDistances(distances3, startNode3);
        System.out.println();


        // Test Case 4: Linear graph
        int V4 = 4;
        List<List<Edge>> adj4 = new ArrayList<>(V4);
        for (int i = 0; i < V4; i++) {
            adj4.add(new ArrayList<>());
        }
        adj4.get(0).add(new Edge(1, 10));
        adj4.get(1).add(new Edge(2, 20));
        adj4.get(2).add(new Edge(3, 30));
        int startNode4 = 0;
        System.out.println("Test Case 4: Linear graph from source 0");
        int[] distances4 = dijkstra(V4, adj4, startNode4);
        printDistances(distances4, startNode4);
        System.out.println();

        // Test Case 5: Invalid input
        System.out.println("Test Case 5: Invalid start node");
        try {
            dijkstra(V1, adj1, -1);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
    }

    private static void printDistances(int[] distances, int startNode) {
        System.out.println("Shortest distances from source node " + startNode + ":");
        for (int i = 0; i < distances.length; i++) {
            if (distances[i] == Integer.MAX_VALUE) {
                System.out.println("Node " + i + ": Infinity");
            } else {
                System.out.println("Node " + i + ": " + distances[i]);
            }
        }
    }
}