import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Implements Depth-First Search (DFS) for a directed graph.
 */
public class Task154 {

    private final int V; // Number of vertices
    private final List<List<Integer>> adj; // Adjacency List

    /**
     * Constructor for the Graph.
     * @param v Number of vertices.
     */
    public Task154(int v) {
        if (v < 0) {
            throw new IllegalArgumentException("Number of vertices must be non-negative.");
        }
        V = v;
        adj = new ArrayList<>(v);
        for (int i = 0; i < v; ++i) {
            adj.add(new ArrayList<>());
        }
    }

    /**
     * Adds an edge from vertex v to vertex w.
     * @param v The source vertex.
     * @param w The destination vertex.
     */
    public void addEdge(int v, int w) {
        if (v < 0 || v >= V || w < 0 || w >= V) {
            System.err.println("Error: Vertex out of bounds. Cannot add edge (" + v + ", " + w + ")");
            return;
        }
        adj.get(v).add(w);
    }

    /**
     * Recursive helper function for DFS traversal.
     * @param v The current vertex.
     * @param visited An array to keep track of visited vertices.
     * @param result The list to store the DFS traversal order.
     */
    private void dfsUtil(int v, boolean[] visited, List<Integer> result) {
        visited[v] = true;
        result.add(v);

        for (int n : adj.get(v)) {
            if (!visited[n]) {
                dfsUtil(n, visited, result);
            }
        }
    }

    /**
     * Performs a Depth-First Search traversal starting from a given node.
     * @param startNode The starting node for the DFS.
     * @return A list of integers representing the DFS traversal path.
     */
    public List<Integer> dfs(int startNode) {
        List<Integer> result = new ArrayList<>();
        if (startNode < 0 || startNode >= V) {
            System.err.println("Error: Start node " + startNode + " is invalid.");
            return result; // Return empty list for invalid start node
        }

        boolean[] visited = new boolean[V];
        Arrays.fill(visited, false);

        dfsUtil(startNode, visited, result);
        return result;
    }

    public static void main(String[] args) {
        // Test Case 1: Standard DFS
        System.out.println("Test Case 1:");
        Task154 g1 = new Task154(7);
        g1.addEdge(0, 1);
        g1.addEdge(0, 2);
        g1.addEdge(1, 3);
        g1.addEdge(1, 4);
        g1.addEdge(2, 5);
        g1.addEdge(2, 6);
        System.out.println("DFS starting from vertex 0: " + g1.dfs(0));

        // Test Case 2: Graph with a cycle
        System.out.println("\nTest Case 2:");
        Task154 g2 = new Task154(4);
        g2.addEdge(0, 1);
        g2.addEdge(0, 2);
        g2.addEdge(1, 2);
        g2.addEdge(2, 0);
        g2.addEdge(2, 3);
        g2.addEdge(3, 3);
        System.out.println("DFS starting from vertex 2: " + g2.dfs(2));

        // Test Case 3: Disconnected graph component
        System.out.println("\nTest Case 3:");
        Task154 g3 = new Task154(5);
        g3.addEdge(0, 1);
        g3.addEdge(1, 2);
        g3.addEdge(3, 4);
        System.out.println("DFS starting from vertex 0: " + g3.dfs(0));
        System.out.println("DFS starting from vertex 3: " + g3.dfs(3));

        // Test Case 4: Start node has no outgoing edges
        System.out.println("\nTest Case 4:");
        Task154 g4 = new Task154(3);
        g4.addEdge(0, 1);
        g4.addEdge(0, 2);
        System.out.println("DFS starting from vertex 1: " + g4.dfs(1));

        // Test Case 5: Invalid start node
        System.out.println("\nTest Case 5:");
        Task154 g5 = new Task154(3);
        g5.addEdge(0, 1);
        g5.addEdge(1, 2);
        System.out.println("DFS starting from vertex 5: " + g5.dfs(5));
        System.out.println("DFS starting from vertex -1: " + g5.dfs(-1));
    }
}