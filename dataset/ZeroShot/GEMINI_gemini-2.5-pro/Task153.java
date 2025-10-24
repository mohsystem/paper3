import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

public class Task153 {

    /**
     * Performs a Breadth-First Search on a graph.
     *
     * @param V         The number of vertices in the graph.
     * @param adj       The adjacency list representation of the graph.
     * @param startNode The starting node for the BFS.
     * @return A list of integers representing the BFS traversal order.
     */
    public static List<Integer> bfs(int V, List<List<Integer>> adj, int startNode) {
        List<Integer> bfsTraversal = new ArrayList<>();

        // Input validation
        if (adj == null || V <= 0 || startNode < 0 || startNode >= V) {
            System.err.println("Error: Invalid input. Graph is null, V is non-positive, or startNode is out of bounds.");
            return bfsTraversal; // Return empty list for invalid input
        }

        boolean[] visited = new boolean[V];
        Queue<Integer> queue = new LinkedList<>();

        // Start BFS from the startNode
        visited[startNode] = true;
        queue.add(startNode);

        while (!queue.isEmpty()) {
            int u = queue.poll();
            bfsTraversal.add(u);

            // Get all adjacent vertices of the dequeued vertex u
            // If an adjacent has not been visited, then mark it visited and enqueue it
            if (u < adj.size() && adj.get(u) != null) {
                for (int v : adj.get(u)) {
                    if (v >= 0 && v < V && !visited[v]) {
                        visited[v] = true;
                        queue.add(v);
                    }
                }
            }
        }

        return bfsTraversal;
    }

    // Helper function to add an edge to an undirected graph
    public static void addEdge(List<List<Integer>> adj, int u, int v) {
        if (adj == null || u < 0 || v < 0 || u >= adj.size() || v >= adj.size()) return;
        adj.get(u).add(v);
        adj.get(v).add(u);
    }

    public static void main(String[] args) {
        int V = 7;
        List<List<Integer>> adj = new ArrayList<>(V);
        for (int i = 0; i < V; i++) {
            adj.add(new ArrayList<>());
        }

        // Creating a sample graph
        //      0 --- 1
        //      | \   |
        //      |  \  |
        //      2---3 4 --- 6
        //      |
        //      5
        addEdge(adj, 0, 1);
        addEdge(adj, 0, 2);
        addEdge(adj, 0, 3);
        addEdge(adj, 1, 4);
        addEdge(adj, 2, 3);
        addEdge(adj, 2, 5);
        addEdge(adj, 4, 6);

        // --- Test Cases ---
        int[] testStarts = {0, 3, 5, 6, 99}; // 99 is an invalid node

        for (int i = 0; i < testStarts.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": BFS starting from node " + testStarts[i]);
            List<Integer> result = bfs(V, adj, testStarts[i]);
            if (result.isEmpty() && (testStarts[i] < 0 || testStarts[i] >= V)) {
                System.out.println(" -> Handled invalid start node correctly.");
            } else {
                System.out.println(" -> Traversal: " + result);
            }
            System.out.println("--------------------");
        }
    }
}