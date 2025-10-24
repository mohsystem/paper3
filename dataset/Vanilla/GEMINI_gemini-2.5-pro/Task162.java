import java.util.*;

class Task162 {

    // Helper class for storing nodes in the priority queue and adjacency list
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
     * Finds the shortest paths from a starting node to all other nodes in a weighted graph.
     *
     * @param V         The number of vertices in the graph.
     * @param adj       The adjacency list representation of the graph.
     * @param startNode The starting node.
     * @return An array containing the shortest distances from the start node.
     */
    public static int[] dijkstra(int V, List<List<Node>> adj, int startNode) {
        int[] dist = new int[V];
        Arrays.fill(dist, Integer.MAX_VALUE);
        dist[startNode] = 0;

        PriorityQueue<Node> pq = new PriorityQueue<>();
        pq.add(new Node(startNode, 0));

        while (!pq.isEmpty()) {
            Node currentNode = pq.poll();
            int u = currentNode.vertex;

            // Optimization: If a shorter path has already been found, skip.
            if (currentNode.weight > dist[u]) {
                continue;
            }

            for (Node neighbor : adj.get(u)) {
                int v = neighbor.vertex;
                int weight = neighbor.weight;

                if (dist[u] != Integer.MAX_VALUE && dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    pq.add(new Node(v, dist[v]));
                }
            }
        }
        return dist;
    }

    public static void main(String[] args) {
        // Test Case 1: Simple connected graph
        System.out.println("--- Test Case 1 ---");
        int V1 = 5;
        List<List<Node>> adj1 = new ArrayList<>();
        for (int i = 0; i < V1; i++) adj1.add(new ArrayList<>());
        adj1.get(0).add(new Node(1, 9));
        adj1.get(0).add(new Node(2, 6));
        adj1.get(0).add(new Node(3, 5));
        adj1.get(0).add(new Node(4, 3));
        adj1.get(2).add(new Node(1, 2));
        adj1.get(2).add(new Node(3, 4));
        int startNode1 = 0;
        int[] dist1 = dijkstra(V1, adj1, startNode1);
        System.out.println("Shortest distances from node " + startNode1 + ": " + Arrays.toString(dist1));

        // Test Case 2: Graph with unreachable node
        System.out.println("\n--- Test Case 2 ---");
        int V2 = 4;
        List<List<Node>> adj2 = new ArrayList<>();
        for (int i = 0; i < V2; i++) adj2.add(new ArrayList<>());
        adj2.get(0).add(new Node(1, 10));
        adj2.get(1).add(new Node(2, 20));
        int startNode2 = 0;
        int[] dist2 = dijkstra(V2, adj2, startNode2);
        System.out.print("Shortest distances from node " + startNode2 + ": [");
        for (int i = 0; i < dist2.length; i++) {
            System.out.print(dist2[i] == Integer.MAX_VALUE ? "INF" : dist2[i]);
            if (i < dist2.length - 1) System.out.print(", ");
        }
        System.out.println("]");

        // Test Case 3: Linear graph
        System.out.println("\n--- Test Case 3 ---");
        int V3 = 4;
        List<List<Node>> adj3 = new ArrayList<>();
        for (int i = 0; i < V3; i++) adj3.add(new ArrayList<>());
        adj3.get(0).add(new Node(1, 5));
        adj3.get(1).add(new Node(2, 5));
        adj3.get(2).add(new Node(3, 5));
        int startNode3 = 0;
        int[] dist3 = dijkstra(V3, adj3, startNode3);
        System.out.println("Shortest distances from node " + startNode3 + ": " + Arrays.toString(dist3));

        // Test Case 4: Graph with a cycle
        System.out.println("\n--- Test Case 4 ---");
        int V4 = 4;
        List<List<Node>> adj4 = new ArrayList<>();
        for (int i = 0; i < V4; i++) adj4.add(new ArrayList<>());
        adj4.get(0).add(new Node(1, 1));
        adj4.get(1).add(new Node(2, 2));
        adj4.get(2).add(new Node(0, 3));
        adj4.get(0).add(new Node(3, 10));
        int startNode4 = 0;
        int[] dist4 = dijkstra(V4, adj4, startNode4);
        System.out.println("Shortest distances from node " + startNode4 + ": " + Arrays.toString(dist4));

        // Test Case 5: More complex graph with multiple paths
        System.out.println("\n--- Test Case 5 ---");
        int V5 = 6;
        List<List<Node>> adj5 = new ArrayList<>();
        for (int i = 0; i < V5; i++) adj5.add(new ArrayList<>());
        adj5.get(0).add(new Node(1, 7));
        adj5.get(0).add(new Node(2, 9));
        adj5.get(0).add(new Node(5, 14));
        adj5.get(1).add(new Node(2, 10));
        adj5.get(1).add(new Node(3, 15));
        adj5.get(2).add(new Node(3, 11));
        adj5.get(2).add(new Node(5, 2));
        adj5.get(3).add(new Node(4, 6));
        adj5.get(4).add(new Node(5, 9));
        int startNode5 = 0;
        int[] dist5 = dijkstra(V5, adj5, startNode5);
        System.out.println("Shortest distances from node " + startNode5 + ": " + Arrays.toString(dist5));
    }
}