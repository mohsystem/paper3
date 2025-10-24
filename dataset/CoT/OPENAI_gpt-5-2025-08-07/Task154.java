import java.util.*;

public class Task154 {
    // Depth-First Search (DFS) on a graph using iterative approach with a stack.
    // Parameters:
    // n: number of nodes (0..n-1)
    // edges: array of edges where each edge is [u, v]
    // start: starting node
    // directed: if true, treat graph as directed; otherwise undirected
    // Returns: List<Integer> traversal order
    public static List<Integer> dfs(int n, int[][] edges, int start, boolean directed) {
        // Input validation
        if (n <= 0) {
            throw new IllegalArgumentException("Number of nodes must be positive.");
        }
        if (start < 0 || start >= n) {
            throw new IllegalArgumentException("Start node out of range.");
        }
        if (edges == null) {
            edges = new int[0][];
        }

        // Initialize adjacency list
        List<List<Integer>> adj = new ArrayList<>(n);
        for (int i = 0; i < n; i++) {
            adj.add(new ArrayList<>());
        }

        // Build adjacency list with validation
        for (int i = 0; i < edges.length; i++) {
            int[] e = edges[i];
            if (e == null || e.length != 2) {
                throw new IllegalArgumentException("Invalid edge at index " + i);
            }
            int u = e[0];
            int v = e[1];
            if (u < 0 || u >= n || v < 0 || v >= n) {
                throw new IllegalArgumentException("Edge vertex out of range at index " + i);
            }
            adj.get(u).add(v);
            if (!directed) {
                adj.get(v).add(u);
            }
        }

        // Sort adjacency lists to ensure deterministic traversal order
        for (int i = 0; i < n; i++) {
            Collections.sort(adj.get(i));
        }

        boolean[] visited = new boolean[n];
        List<Integer> result = new ArrayList<>(n);
        Deque<Integer> stack = new ArrayDeque<>();
        stack.push(start);

        while (!stack.isEmpty()) {
            int u = stack.pop();
            if (visited[u]) continue;
            visited[u] = true;
            result.add(u);

            // Push neighbors in reverse order so that the smallest neighbor is processed first
            List<Integer> neighbors = adj.get(u);
            for (int i = neighbors.size() - 1; i >= 0; i--) {
                int v = neighbors.get(i);
                if (!visited[v]) {
                    stack.push(v);
                }
            }
        }

        return result;
    }

    private static void printList(List<Integer> list) {
        System.out.println(list.toString());
    }

    public static void main(String[] args) {
        // Test case 1: Simple chain undirected
        int n1 = 4;
        int[][] edges1 = { {0,1}, {1,2}, {2,3} };
        System.out.print("Test 1: ");
        printList(dfs(n1, edges1, 0, false));

        // Test case 2: Branching undirected
        int n2 = 5;
        int[][] edges2 = { {0,1}, {0,2}, {1,3}, {1,4} };
        System.out.print("Test 2: ");
        printList(dfs(n2, edges2, 0, false));

        // Test case 3: Directed with cycle
        int n3 = 4;
        int[][] edges3 = { {0,1}, {1,2}, {2,0}, {2,3} };
        System.out.print("Test 3: ");
        printList(dfs(n3, edges3, 0, true));

        // Test case 4: Isolated node
        int n4 = 5;
        int[][] edges4 = { {0,1}, {1,2} };
        System.out.print("Test 4: ");
        printList(dfs(n4, edges4, 4, false));

        // Test case 5: Invalid start node (handled by exception)
        int n5 = 3;
        int[][] edges5 = { {0,1} };
        System.out.print("Test 5: ");
        try {
            printList(dfs(n5, edges5, 5, false));
        } catch (IllegalArgumentException ex) {
            System.out.println("Error: " + ex.getMessage());
        }
    }
}