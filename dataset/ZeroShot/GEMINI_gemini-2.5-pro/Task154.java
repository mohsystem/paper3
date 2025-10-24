import java.util.*;

public class Task154 {

    /**
     * Performs an iterative Depth-First Search (DFS) on a graph.
     *
     * @param V         The number of vertices in the graph.
     * @param adj       The adjacency list representation of the graph.
     * @param startNode The node to start the search from.
     * @return A list of integers representing the DFS traversal order.
     */
    public List<Integer> dfs(int V, List<List<Integer>> adj, int startNode) {
        // --- Security: Input Validation ---
        if (V <= 0 || startNode < 0 || startNode >= V || adj == null || adj.size() != V) {
            // Return an empty list for invalid input to prevent exceptions.
            return new ArrayList<>();
        }

        List<Integer> result = new ArrayList<>();
        boolean[] visited = new boolean[V];
        
        // Using ArrayDeque as a Stack is generally preferred over the legacy Stack class.
        Deque<Integer> stack = new ArrayDeque<>();

        stack.push(startNode);

        while (!stack.isEmpty()) {
            int u = stack.pop();

            if (!visited[u]) {
                visited[u] = true;
                result.add(u);

                // Get all adjacent vertices of the popped vertex u.
                // We iterate in reverse to approximate the order of a recursive DFS.
                List<Integer> neighbors = adj.get(u);
                if (neighbors != null) {
                    for (int i = neighbors.size() - 1; i >= 0; i--) {
                        int v = neighbors.get(i);
                        // --- Security: Check neighbor validity ---
                        if (v >= 0 && v < V && !visited[v]) {
                            stack.push(v);
                        }
                    }
                }
            }
        }
        return result;
    }
    
    // Helper function to add an edge to the graph
    private static void addEdge(List<List<Integer>> adj, int u, int v) {
        if (u >= 0 && u < adj.size() && v >= 0 && v < adj.size()) {
            adj.get(u).add(v);
        }
    }

    public static void main(String[] args) {
        Task154 solution = new Task154();
        
        // --- Test Cases ---
        System.out.println("--- 5 Test Cases for DFS ---");

        // Test Case 1: Connected Graph, Start 0
        int V1 = 7;
        List<List<Integer>> adj1 = new ArrayList<>(V1);
        for (int i = 0; i < V1; i++) adj1.add(new ArrayList<>());
        addEdge(adj1, 0, 1); addEdge(adj1, 0, 3);
        addEdge(adj1, 1, 2); addEdge(adj1, 1, 4);
        addEdge(adj1, 2, 1);
        addEdge(adj1, 3, 0); addEdge(adj1, 3, 4); addEdge(adj1, 3, 5);
        addEdge(adj1, 4, 1); addEdge(adj1, 4, 3);
        addEdge(adj1, 5, 3); addEdge(adj1, 5, 6);
        addEdge(adj1, 6, 5);
        System.out.println("Test Case 1 (Connected Graph, Start 0): " + solution.dfs(V1, adj1, 0));

        // Test Case 2: Connected Graph, Start 4
        System.out.println("Test Case 2 (Connected Graph, Start 4): " + solution.dfs(V1, adj1, 4));

        // Test Case 3: Disconnected Graph, Start 0
        int V3 = 5;
        List<List<Integer>> adj3 = new ArrayList<>(V3);
        for (int i = 0; i < V3; i++) adj3.add(new ArrayList<>());
        addEdge(adj3, 0, 1); addEdge(adj3, 1, 0); addEdge(adj3, 1, 2); addEdge(adj3, 2, 1); // Component 1
        addEdge(adj3, 3, 4); addEdge(adj3, 4, 3); // Component 2
        System.out.println("Test Case 3 (Disconnected Graph, Start 0): " + solution.dfs(V3, adj3, 0));

        // Test Case 4: Line Graph, Start 2
        int V4 = 5;
        List<List<Integer>> adj4 = new ArrayList<>(V4);
        for (int i = 0; i < V4; i++) adj4.add(new ArrayList<>());
        addEdge(adj4, 0, 1);
        addEdge(adj4, 1, 0); addEdge(adj4, 1, 2);
        addEdge(adj4, 2, 1); addEdge(adj4, 2, 3);
        addEdge(adj4, 3, 2); addEdge(adj4, 3, 4);
        addEdge(adj4, 4, 3);
        System.out.println("Test Case 4 (Line Graph, Start 2): " + solution.dfs(V4, adj4, 2));

        // Test Case 5: Single Node Graph, Start 0
        int V5 = 1;
        List<List<Integer>> adj5 = new ArrayList<>(V5);
        adj5.add(new ArrayList<>());
        System.out.println("Test Case 5 (Single Node Graph, Start 0): " + solution.dfs(V5, adj5, 0));
    }
}