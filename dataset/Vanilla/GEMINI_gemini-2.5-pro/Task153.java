import java.util.*;

public class Task153 {

    /**
     * Performs a Breadth-First Search on a graph starting from a given node.
     *
     * @param startNode The node to start the search from.
     * @param V         The total number of vertices in the graph.
     * @param adj       The adjacency list representation of the graph.
     * @return A list of integers representing the BFS traversal order.
     */
    public List<Integer> bfs(int startNode, int V, ArrayList<ArrayList<Integer>> adj) {
        List<Integer> bfsTraversal = new ArrayList<>();
        if (startNode >= V) {
            return bfsTraversal; // Return empty list if start node is invalid
        }
        boolean[] visited = new boolean[V];
        Queue<Integer> q = new LinkedList<>();

        visited[startNode] = true;
        q.add(startNode);

        while (!q.isEmpty()) {
            int currentNode = q.poll();
            bfsTraversal.add(currentNode);

            for (int neighbor : adj.get(currentNode)) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.add(neighbor);
                }
            }
        }
        return bfsTraversal;
    }

    /**
     * Helper function to add an edge to an undirected graph.
     * @param adj The adjacency list.
     * @param u   The source vertex.
     * @param v   The destination vertex.
     */
    public static void addEdge(ArrayList<ArrayList<Integer>> adj, int u, int v) {
        adj.get(u).add(v);
        adj.get(v).add(u);
    }

    public static void main(String[] args) {
        Task153 solution = new Task153();

        // Test Case 1: Connected graph
        System.out.println("--- Test Case 1 ---");
        int V1 = 5;
        ArrayList<ArrayList<Integer>> adj1 = new ArrayList<>(V1);
        for (int i = 0; i < V1; i++) {
            adj1.add(new ArrayList<>());
        }
        addEdge(adj1, 0, 1);
        addEdge(adj1, 0, 2);
        addEdge(adj1, 1, 3);
        addEdge(adj1, 2, 4);
        System.out.println("Graph: Connected graph with 5 vertices.");
        System.out.println("BFS starting from node 0: " + solution.bfs(0, V1, adj1));

        // Test Case 2: Different start node on a more complex graph
        System.out.println("\n--- Test Case 2 ---");
        int V2 = 5;
        ArrayList<ArrayList<Integer>> adj2 = new ArrayList<>(V2);
        for (int i = 0; i < V2; i++) {
            adj2.add(new ArrayList<>());
        }
        addEdge(adj2, 0, 1);
        addEdge(adj2, 0, 2);
        addEdge(adj2, 1, 2);
        addEdge(adj2, 1, 3);
        addEdge(adj2, 2, 4);
        System.out.println("Graph: More complex connected graph.");
        System.out.println("BFS starting from node 3: " + solution.bfs(3, V2, adj2));

        // Test Case 3: Disconnected graph
        System.out.println("\n--- Test Case 3 ---");
        int V3 = 7;
        ArrayList<ArrayList<Integer>> adj3 = new ArrayList<>(V3);
        for (int i = 0; i < V3; i++) {
            adj3.add(new ArrayList<>());
        }
        // Component 1
        addEdge(adj3, 0, 1);
        addEdge(adj3, 0, 2);
        // Component 2
        addEdge(adj3, 3, 4);
        addEdge(adj3, 5, 6);
        System.out.println("Graph: Disconnected graph with 7 vertices.");
        System.out.println("BFS starting from node 3: " + solution.bfs(3, V3, adj3));

        // Test Case 4: Single node graph
        System.out.println("\n--- Test Case 4 ---");
        int V4 = 1;
        ArrayList<ArrayList<Integer>> adj4 = new ArrayList<>(V4);
        adj4.add(new ArrayList<>());
        System.out.println("Graph: Single node graph.");
        System.out.println("BFS starting from node 0: " + solution.bfs(0, V4, adj4));

        // Test Case 5: Linear graph (like a linked list)
        System.out.println("\n--- Test Case 5 ---");
        int V5 = 5;
        ArrayList<ArrayList<Integer>> adj5 = new ArrayList<>(V5);
        for (int i = 0; i < V5; i++) {
            adj5.add(new ArrayList<>());
        }
        addEdge(adj5, 0, 1);
        addEdge(adj5, 1, 2);
        addEdge(adj5, 2, 3);
        addEdge(adj5, 3, 4);
        System.out.println("Graph: Linear graph with 5 vertices.");
        System.out.println("BFS starting from node 2: " + solution.bfs(2, V5, adj5));
    }
}