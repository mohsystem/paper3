import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Deque;
import java.util.List;

public class Task153 {

    public static List<Integer> bfs(int n, List<List<Integer>> adjacency, int start) {
        validateGraph(n, adjacency, start);

        List<Integer> order = new ArrayList<>();
        boolean[] visited = new boolean[n];
        Deque<Integer> queue = new ArrayDeque<>();

        visited[start] = true;
        queue.add(start);

        while (!queue.isEmpty()) {
            int u = queue.removeFirst();
            order.add(u);
            List<Integer> neighbors = adjacency.get(u);
            for (int v : neighbors) {
                // neighbors already validated in validateGraph
                if (!visited[v]) {
                    visited[v] = true;
                    queue.addLast(v);
                }
            }
        }
        return order;
    }

    private static void validateGraph(int n, List<List<Integer>> adjacency, int start) {
        if (n < 0) {
            throw new IllegalArgumentException("n must be >= 0");
        }
        if (adjacency == null) {
            throw new IllegalArgumentException("adjacency must not be null");
        }
        if (adjacency.size() != n) {
            throw new IllegalArgumentException("adjacency size must equal n");
        }
        if (n == 0) {
            throw new IllegalArgumentException("graph has no nodes");
        }
        if (start < 0 || start >= n) {
            throw new IllegalArgumentException("start node out of range");
        }
        for (int i = 0; i < n; i++) {
            List<Integer> neighbors = adjacency.get(i);
            if (neighbors == null) {
                throw new IllegalArgumentException("adjacency list at index " + i + " is null");
            }
            for (int v : neighbors) {
                if (v < 0 || v >= n) {
                    throw new IllegalArgumentException("edge from " + i + " to invalid node " + v);
                }
            }
        }
    }

    private static void printOrder(List<Integer> order) {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int i = 0; i < order.size(); i++) {
            if (i > 0) sb.append(" ");
            sb.append(order.get(i));
        }
        sb.append("]");
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        // Test case 1: Simple line graph 0-1-2-3 from 0
        try {
            int n1 = 4;
            List<List<Integer>> adj1 = Arrays.asList(
                Arrays.asList(1),
                Arrays.asList(2),
                Arrays.asList(3),
                Arrays.asList()
            );
            List<Integer> res1 = bfs(n1, adj1, 0);
            System.out.print("Test 1: ");
            printOrder(res1);
        } catch (IllegalArgumentException e) {
            System.out.println("Test 1 Error: " + e.getMessage());
        }

        // Test case 2: Graph with cycles
        try {
            int n2 = 4;
            List<List<Integer>> adj2 = Arrays.asList(
                Arrays.asList(1, 2), // 0
                Arrays.asList(2),    // 1
                Arrays.asList(0, 3), // 2
                Arrays.asList()      // 3
            );
            List<Integer> res2 = bfs(n2, adj2, 1);
            System.out.print("Test 2: ");
            printOrder(res2);
        } catch (IllegalArgumentException e) {
            System.out.println("Test 2 Error: " + e.getMessage());
        }

        // Test case 3: Disconnected graph start at 3
        try {
            int n3 = 5;
            List<List<Integer>> adj3 = Arrays.asList(
                Arrays.asList(1), // 0
                Arrays.asList(2), // 1
                Arrays.asList(),  // 2
                Arrays.asList(4), // 3
                Arrays.asList()   // 4
            );
            List<Integer> res3 = bfs(n3, adj3, 3);
            System.out.print("Test 3: ");
            printOrder(res3);
        } catch (IllegalArgumentException e) {
            System.out.println("Test 3 Error: " + e.getMessage());
        }

        // Test case 4: Invalid start node
        try {
            int n4 = 3;
            List<List<Integer>> adj4 = Arrays.asList(
                Arrays.asList(1),
                Arrays.asList(2),
                Arrays.asList()
            );
            List<Integer> res4 = bfs(n4, adj4, 5);
            System.out.print("Test 4: ");
            printOrder(res4);
        } catch (IllegalArgumentException e) {
            System.out.println("Test 4 Error: " + e.getMessage());
        }

        // Test case 5: Invalid edge reference
        try {
            int n5 = 3;
            List<List<Integer>> adj5 = Arrays.asList(
                Arrays.asList(1, 99), // invalid neighbor
                Arrays.asList(2),
                Arrays.asList()
            );
            List<Integer> res5 = bfs(n5, adj5, 0);
            System.out.print("Test 5: ");
            printOrder(res5);
        } catch (IllegalArgumentException e) {
            System.out.println("Test 5 Error: " + e.getMessage());
        }
    }
}