// Secure BFS implementation in Java with test cases.
// Main class name as requested: Task153
import java.util.*;

public class Task153 {

    // Step 1: Problem understanding
    // Performs a BFS on a graph given as a map from node to list of neighbors, starting at 'start'.
    // Returns the visitation order as a list of integers.

    // Step 2 and 3: Security requirements and secure coding generation
    // - Null checks for graph
    // - Defensive copying avoided for performance but we never modify input lists directly
    // - Use of safe collections and checks to avoid NPEs
    public static List<Integer> bfs(Map<Integer, List<Integer>> graph, int start) {
        List<Integer> order = new ArrayList<>();
        // Treat null graph as empty graph
        Set<Integer> visited = new HashSet<>();
        ArrayDeque<Integer> q = new ArrayDeque<>();

        visited.add(start);
        q.add(start);

        while (!q.isEmpty()) {
            Integer cur = q.remove();
            order.add(cur);

            List<Integer> neighbors = (graph != null) ? graph.get(cur) : null;
            if (neighbors == null) {
                continue;
            }
            for (Integer nb : neighbors) {
                if (nb == null) continue; // defensive: ignore null neighbors
                if (!visited.contains(nb)) {
                    visited.add(nb);
                    q.add(nb);
                }
            }
        }
        return order;
    }

    // Utility to add undirected edge while preventing duplicate entries
    private static void addEdgeUndirected(Map<Integer, List<Integer>> g, int u, int v) {
        addNeighbor(g, u, v);
        addNeighbor(g, v, u);
    }

    private static void addNeighbor(Map<Integer, List<Integer>> g, int u, int v) {
        List<Integer> list = g.get(u);
        if (list == null) {
            list = new ArrayList<>();
            g.put(u, list);
        }
        // Avoid duplicates to keep output predictable
        if (!list.contains(v)) list.add(v);
    }

    // Utility to put node with an exact ordered neighbor list (defensive copy to avoid external mutation)
    private static void putWithOrder(Map<Integer, List<Integer>> g, int u, List<Integer> neighbors) {
        if (neighbors == null) {
            g.put(u, new ArrayList<>());
        } else {
            g.put(u, new ArrayList<>(neighbors));
        }
    }

    private static void printResult(String label, List<Integer> order) {
        System.out.println(label + ": " + order);
    }

    // Step 4: Code review (conceptual) - checks performed via careful coding above
    // Step 5: Secure code output

    // Main with 5 test cases as requested
    public static void main(String[] args) {
        // Test 1: Simple chain 0-1-2-3 (undirected)
        Map<Integer, List<Integer>> g1 = new HashMap<>();
        addEdgeUndirected(g1, 0, 1);
        addEdgeUndirected(g1, 1, 2);
        addEdgeUndirected(g1, 2, 3);
        printResult("Test1", bfs(g1, 0)); // expected [0,1,2,3]

        // Test 2: Cycle 1-2-3-1 with branch 2-4 (undirected)
        Map<Integer, List<Integer>> g2 = new HashMap<>();
        addEdgeUndirected(g2, 1, 2);
        addEdgeUndirected(g2, 2, 3);
        addEdgeUndirected(g2, 3, 1);
        addEdgeUndirected(g2, 2, 4);
        printResult("Test2", bfs(g2, 1)); // expected [1,2,3,4] (order of neighbors preserved)

        // Test 3: Disconnected with isolated node 5
        Map<Integer, List<Integer>> g3 = new HashMap<>();
        g3.put(5, new ArrayList<>());
        printResult("Test3", bfs(g3, 5)); // expected [5]

        // Test 4: Ordered neighbors: 0 -> [3,1,2], deeper nodes
        Map<Integer, List<Integer>> g4 = new HashMap<>();
        putWithOrder(g4, 0, Arrays.asList(3, 1, 2));
        putWithOrder(g4, 1, Arrays.asList(4));
        putWithOrder(g4, 2, Arrays.asList());
        putWithOrder(g4, 3, Arrays.asList(5));
        putWithOrder(g4, 4, Arrays.asList());
        putWithOrder(g4, 5, Arrays.asList());
        printResult("Test4", bfs(g4, 0)); // expected [0,3,1,2,5,4] depending on exact exploration

        // Test 5: Start not present in graph
        Map<Integer, List<Integer>> g5 = new HashMap<>();
        addEdgeUndirected(g5, 10, 11);
        printResult("Test5", bfs(g5, 99)); // expected [99]
    }
}