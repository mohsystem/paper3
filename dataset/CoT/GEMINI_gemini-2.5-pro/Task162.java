import java.util.*;

class Task162 {

    // Inner class to represent a node in the graph's adjacency list
    // and for the priority queue
    static class Node implements Comparable<Node> {
        public int vertex;
        public int weight;

        public Node(int vertex, int weight) {
            this.vertex = vertex;
            this.weight = weight;
        }

        @Override
        public int compareTo(Node other) {
            return Integer.compare(this.weight, other.weight);
        }
    }

    /**
     * Implements Dijkstra's algorithm to find the shortest path from a source
     * node to all other nodes in a weighted graph.
     *
     * @param V The number of vertices in the graph.
     * @param adj The adjacency list representation of the graph.
     * @param src The source vertex.
     * @return An array containing the shortest distances from the source vertex.
     */
    public int[] dijkstra(int V, List<List<Node>> adj, int src) {
        int[] dist = new int[V];
        Arrays.fill(dist, Integer.MAX_VALUE);
        dist[src] = 0;

        PriorityQueue<Node> pq = new PriorityQueue<>();
        pq.add(new Node(src, 0));

        while (!pq.isEmpty()) {
            Node currentNode = pq.poll();
            int u = currentNode.vertex;

            // If we've found a shorter path already, skip
            if (currentNode.weight > dist[u]) {
                continue;
            }

            for (Node neighbor : adj.get(u)) {
                int v = neighbor.vertex;
                int weight = neighbor.weight;

                // Relaxation step
                if (dist[u] != Integer.MAX_VALUE && dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    pq.add(new Node(v, dist[v]));
                }
            }
        }
        return dist;
    }

    public static void main(String[] args) {
        Task162 solution = new Task162();

        // --- Test Case 1: Simple Graph ---
        System.out.println("--- Test Case 1 ---");
        int V1 = 5, src1 = 0;
        List<List<Node>> adj1 = new ArrayList<>();
        for (int i = 0; i < V1; i++) adj1.add(new ArrayList<>());
        adj1.get(0).add(new Node(1, 10));
        adj1.get(0).add(new Node(3, 5));
        adj1.get(1).add(new Node(2, 1));
        adj1.get(1).add(new Node(3, 2));
        adj1.get(2).add(new Node(4, 4));
        adj1.get(3).add(new Node(1, 3));
        adj1.get(3).add(new Node(2, 9));
        adj1.get(3).add(new Node(4, 2));
        adj1.get(4).add(new Node(0, 7));
        adj1.get(4).add(new Node(2, 6));
        int[] dist1 = solution.dijkstra(V1, adj1, src1);
        System.out.println("Distances from source " + src1 + ": " + Arrays.toString(dist1));

        // --- Test Case 2: Disconnected Graph ---
        System.out.println("\n--- Test Case 2 ---");
        int V2 = 6, src2 = 0;
        List<List<Node>> adj2 = new ArrayList<>();
        for (int i = 0; i < V2; i++) adj2.add(new ArrayList<>());
        adj2.get(0).add(new Node(1, 2));
        adj2.get(1).add(new Node(2, 3));
        adj2.get(3).add(new Node(4, 5));
        adj2.get(4).add(new Node(5, 6));
        int[] dist2 = solution.dijkstra(V2, adj2, src2);
        System.out.println("Distances from source " + src2 + ": " + Arrays.toString(dist2));

        // --- Test Case 3: Line Graph ---
        System.out.println("\n--- Test Case 3 ---");
        int V3 = 4, src3 = 0;
        List<List<Node>> adj3 = new ArrayList<>();
        for (int i = 0; i < V3; i++) adj3.add(new ArrayList<>());
        adj3.get(0).add(new Node(1, 1));
        adj3.get(1).add(new Node(2, 2));
        adj3.get(2).add(new Node(3, 3));
        int[] dist3 = solution.dijkstra(V3, adj3, src3);
        System.out.println("Distances from source " + src3 + ": " + Arrays.toString(dist3));

        // --- Test Case 4: Graph with alternative paths ---
        System.out.println("\n--- Test Case 4 ---");
        int V4 = 6, src4 = 0;
        List<List<Node>> adj4 = new ArrayList<>();
        for (int i = 0; i < V4; i++) adj4.add(new ArrayList<>());
        adj4.get(0).add(new Node(1, 4));
        adj4.get(0).add(new Node(2, 1));
        adj4.get(1).add(new Node(3, 1));
        adj4.get(2).add(new Node(1, 2));
        adj4.get(2).add(new Node(3, 5));
        adj4.get(3).add(new Node(4, 3));
        adj4.get(4).add(new Node(5, 2));
        adj4.get(2).add(new Node(5, 10));
        int[] dist4 = solution.dijkstra(V4, adj4, src4);
        System.out.println("Distances from source " + src4 + ": " + Arrays.toString(dist4));

        // --- Test Case 5: Single Node Graph ---
        System.out.println("\n--- Test Case 5 ---");
        int V5 = 1, src5 = 0;
        List<List<Node>> adj5 = new ArrayList<>();
        for (int i = 0; i < V5; i++) adj5.add(new ArrayList<>());
        int[] dist5 = solution.dijkstra(V5, adj5, src5);
        System.out.println("Distances from source " + src5 + ": " + Arrays.toString(dist5));
    }
}