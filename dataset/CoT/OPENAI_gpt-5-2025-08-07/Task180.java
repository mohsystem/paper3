import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Deque;
import java.util.List;

public class Task180 {
    // Chain-of-Through process:
    // 1) Problem understanding:
    //    Re-orient (reparent) a tree on a selected node. Input is an undirected tree (n nodes, n-1 edges).
    //    Output is a parent array where parent[newRoot] = -1 and for others parent[v] is the parent in the rooted tree.
    // 2) Security requirements:
    //    Validate inputs: node range, edges length, self-loops, bounds, and connectivity.
    // 3) Secure coding generation:
    //    Use safe iterations, avoid recursion depth issues by using iterative DFS, and avoid mutable shared state leaks.
    // 4) Code review:
    //    Ensure bounds checks, defensive copies not needed for primitives, handle all exceptional paths gracefully.
    // 5) Secure code output:
    //    Throw exceptions on invalid inputs; callers can catch and handle safely.

    public static int[] reparentTree(int n, int[][] edges, int newRoot) {
        if (n <= 0) {
            throw new IllegalArgumentException("n must be positive");
        }
        if (edges == null) {
            throw new IllegalArgumentException("edges must not be null");
        }
        if (edges.length != n - 1) {
            throw new IllegalArgumentException("edges must contain exactly n-1 edges for a tree");
        }
        if (newRoot < 0 || newRoot >= n) {
            throw new IllegalArgumentException("newRoot out of bounds");
        }

        // Build adjacency list with validation
        List<List<Integer>> adj = new ArrayList<>(n);
        for (int i = 0; i < n; i++) adj.add(new ArrayList<>());
        for (int i = 0; i < edges.length; i++) {
            int[] e = edges[i];
            if (e == null || e.length != 2) {
                throw new IllegalArgumentException("Invalid edge at index " + i);
            }
            int u = e[0], v = e[1];
            if (u < 0 || u >= n || v < 0 || v >= n) {
                throw new IllegalArgumentException("Edge node out of bounds at index " + i);
            }
            if (u == v) {
                throw new IllegalArgumentException("Self-loop detected at index " + i);
            }
            adj.get(u).add(v);
            adj.get(v).add(u);
        }

        // Iterative DFS/BFS to orient the tree from newRoot
        int[] parent = new int[n];
        Arrays.fill(parent, Integer.MIN_VALUE); // mark unvisited
        Deque<Integer> stack = new ArrayDeque<>();
        parent[newRoot] = -1;
        stack.push(newRoot);

        while (!stack.isEmpty()) {
            int u = stack.pop();
            for (int v : adj.get(u)) {
                if (parent[v] == Integer.MIN_VALUE) {
                    parent[v] = u;
                    stack.push(v);
                }
            }
        }

        // Validate connectivity
        for (int i = 0; i < n; i++) {
            if (parent[i] == Integer.MIN_VALUE) {
                throw new IllegalArgumentException("Input is not a connected tree");
            }
        }

        return parent;
    }

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        // 5 test cases
        try {
            // Test 1: Example tree, re-root at 6
            int n1 = 10;
            int[][] edges1 = {
                {0,1},{0,2},{0,3},{1,4},{1,5},{2,6},{2,7},{3,8},{3,9}
            };
            printArray(reparentTree(n1, edges1, 6));
        } catch (Exception ex) {
            System.out.println("Test 1 error: " + ex.getMessage());
        }

        try {
            // Test 2: Example tree, re-root at 0 (original root)
            int n2 = 10;
            int[][] edges2 = {
                {0,1},{0,2},{0,3},{1,4},{1,5},{2,6},{2,7},{3,8},{3,9}
            };
            printArray(reparentTree(n2, edges2, 0));
        } catch (Exception ex) {
            System.out.println("Test 2 error: " + ex.getMessage());
        }

        try {
            // Test 3: Line tree 0-1-2-3-4, re-root at 4
            int n3 = 5;
            int[][] edges3 = {
                {0,1},{1,2},{2,3},{3,4}
            };
            printArray(reparentTree(n3, edges3, 4));
        } catch (Exception ex) {
            System.out.println("Test 3 error: " + ex.getMessage());
        }

        try {
            // Test 4: Star centered at 0 with leaves 1..5, re-root at 3
            int n4 = 6;
            int[][] edges4 = {
                {0,1},{0,2},{0,3},{0,4},{0,5}
            };
            printArray(reparentTree(n4, edges4, 3));
        } catch (Exception ex) {
            System.out.println("Test 4 error: " + ex.getMessage());
        }

        try {
            // Test 5: Small arbitrary tree, re-root at 4
            int n5 = 5;
            int[][] edges5 = {
                {0,1},{1,2},{1,3},{3,4}
            };
            printArray(reparentTree(n5, edges5, 4));
        } catch (Exception ex) {
            System.out.println("Test 5 error: " + ex.getMessage());
        }
    }
}