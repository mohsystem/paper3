import java.util.*;

public class Task154 {
    // Performs DFS on a graph with n nodes, adjacency list "graph", starting from "start"
    // Returns the traversal order as a list of node indices.
    public static List<Integer> dfs(int n, List<List<Integer>> graph, int start) {
        List<Integer> traversal = new ArrayList<>();
        if (n <= 0 || graph == null || graph.size() != n || start < 0 || start >= n) {
            return traversal;
        }

        // Sanitize adjacency list: bounds-check, deduplicate, sort ascending
        List<List<Integer>> adj = new ArrayList<>(n);
        for (int i = 0; i < n; i++) {
            boolean[] seen = new boolean[n];
            List<Integer> cleaned = new ArrayList<>();
            List<Integer> neighbors = graph.get(i);
            if (neighbors != null) {
                for (Integer v : neighbors) {
                    if (v != null && v >= 0 && v < n && !seen[v]) {
                        seen[v] = true;
                        cleaned.add(v);
                    }
                }
            }
            Collections.sort(cleaned);
            adj.add(cleaned);
        }

        boolean[] visited = new boolean[n];
        Deque<Integer> stack = new ArrayDeque<>();
        stack.push(start);

        while (!stack.isEmpty()) {
            int v = stack.pop();
            if (v < 0 || v >= n) {
                continue;
            }
            if (!visited[v]) {
                visited[v] = true;
                traversal.add(v);
                List<Integer> neighbors = adj.get(v);
                // Push in reverse sorted order so nodes are visited in ascending order
                for (int i = neighbors.size() - 1; i >= 0; i--) {
                    int nb = neighbors.get(i);
                    if (!visited[nb]) {
                        stack.push(nb);
                    }
                }
            }
        }

        return traversal;
    }

    private static void printResult(List<Integer> result) {
        System.out.println(result.toString());
    }

    public static void main(String[] args) {
        // Test 1: Simple chain 0-1-2
        {
            int n = 3;
            List<List<Integer>> g = Arrays.asList(
                Arrays.asList(1),
                Arrays.asList(0, 2),
                Arrays.asList(1)
            );
            printResult(dfs(n, g, 0)); // Expected: [0, 1, 2]
        }

        // Test 2: Cycle 0-1-2-3-0
        {
            int n = 4;
            List<List<Integer>> g = Arrays.asList(
                Arrays.asList(1, 3),
                Arrays.asList(0, 2),
                Arrays.asList(1, 3),
                Arrays.asList(2, 0)
            );
            printResult(dfs(n, g, 1)); // Deterministic order due to sorting
        }

        // Test 3: Disconnected graph
        {
            int n = 5;
            List<List<Integer>> g = Arrays.asList(
                Arrays.asList(),
                Arrays.asList(2),
                Arrays.asList(1),
                Arrays.asList(4),
                Arrays.asList(3)
            );
            printResult(dfs(n, g, 3)); // Expected: [3, 4]
        }

        // Test 4: Self-loop and out-of-range neighbor
        {
            int n = 4;
            List<List<Integer>> g = Arrays.asList(
                Arrays.asList(0, 1),   // self-loop at 0
                Arrays.asList(0, 2, 4),// 4 is out-of-range and should be ignored
                Arrays.asList(1, 3),
                Arrays.asList(2)
            );
            printResult(dfs(n, g, 0)); // Expected: [0, 1, 2, 3]
        }

        // Test 5: Star graph centered at 0
        {
            int n = 6;
            List<List<Integer>> g = Arrays.asList(
                Arrays.asList(1, 2, 3, 4, 5),
                Arrays.asList(0),
                Arrays.asList(0),
                Arrays.asList(0),
                Arrays.asList(0),
                Arrays.asList(0)
            );
            printResult(dfs(n, g, 2)); // Expected deterministic order due to sorting/push strategy
        }
    }
}