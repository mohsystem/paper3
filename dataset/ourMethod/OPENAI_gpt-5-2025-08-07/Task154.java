import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;

public class Task154 {

    public static final class DFSResult {
        public final boolean ok;
        public final List<Integer> order;
        public final String error;

        public DFSResult(boolean ok, List<Integer> order, String error) {
            this.ok = ok;
            this.order = order;
            this.error = error;
        }
    }

    public static DFSResult dfsTraversal(int n, List<List<Integer>> adj, int start) {
        // Validate inputs
        if (n <= 0) {
            return new DFSResult(false, new ArrayList<>(), "Invalid number of nodes (must be > 0).");
        }
        if (adj == null || adj.size() != n) {
            return new DFSResult(false, new ArrayList<>(), "Adjacency list is null or size does not match node count.");
        }
        if (start < 0 || start >= n) {
            return new DFSResult(false, new ArrayList<>(), "Start node is out of range.");
        }
        for (int i = 0; i < n; i++) {
            List<Integer> neighbors = adj.get(i);
            if (neighbors == null) {
                return new DFSResult(false, new ArrayList<>(), "Adjacency list contains a null entry at node " + i + ".");
            }
            for (int v : neighbors) {
                if (v < 0 || v >= n) {
                    return new DFSResult(false, new ArrayList<>(), "Edge from " + i + " to out-of-range node " + v + ".");
                }
            }
        }

        boolean[] visited = new boolean[n];
        List<Integer> order = new ArrayList<>(n);
        Deque<Integer> stack = new ArrayDeque<>();
        stack.push(start);

        while (!stack.isEmpty()) {
            int u = stack.pop();
            if (visited[u]) {
                continue;
            }
            visited[u] = true;
            order.add(u);
            // Push neighbors in reverse order to maintain a predictable traversal for ordered inputs
            List<Integer> neighbors = adj.get(u);
            for (int idx = neighbors.size() - 1; idx >= 0; idx--) {
                int v = neighbors.get(idx);
                if (!visited[v]) {
                    stack.push(v);
                }
            }
        }
        return new DFSResult(true, order, "");
    }

    // Helper to build adjacency lists for tests
    private static List<List<Integer>> buildAdj(int n, int[][] edges, boolean undirected) {
        List<List<Integer>> adj = new ArrayList<>(n);
        for (int i = 0; i < n; i++) {
            adj.add(new ArrayList<>());
        }
        for (int[] e : edges) {
            if (e.length != 2) continue;
            int u = e[0], v = e[1];
            if (u >= 0 && u < n && v >= 0 && v < n) {
                adj.get(u).add(v);
                if (undirected) {
                    adj.get(v).add(u);
                }
            }
        }
        return adj;
    }

    public static void main(String[] args) {
        // Test 1: Undirected chain 0-1-2-3, start=0
        {
            int n = 4;
            int[][] edges = { {0,1}, {1,2}, {2,3} };
            List<List<Integer>> adj = buildAdj(n, edges, true);
            DFSResult res = dfsTraversal(n, adj, 0);
            System.out.println("Test 1: " + (res.ok ? res.order.toString() : ("Error: " + res.error)));
        }

        // Test 2: Directed cycle 0->1->2->0 and 2->3, start=0
        {
            int n = 4;
            int[][] edges = { {0,1}, {1,2}, {2,0}, {2,3} };
            List<List<Integer>> adj = buildAdj(n, edges, false);
            DFSResult res = dfsTraversal(n, adj, 0);
            System.out.println("Test 2: " + (res.ok ? res.order.toString() : ("Error: " + res.error)));
        }

        // Test 3: Disconnected graph, start in second component
        {
            int n = 5;
            int[][] edges = { {0,1}, {1,0}, {2,3}, {3,2} };
            List<List<Integer>> adj = buildAdj(n, edges, false);
            DFSResult res = dfsTraversal(n, adj, 2);
            System.out.println("Test 3: " + (res.ok ? res.order.toString() : ("Error: " + res.error)));
        }

        // Test 4: Single node graph
        {
            int n = 1;
            int[][] edges = {};
            List<List<Integer>> adj = buildAdj(n, edges, false);
            DFSResult res = dfsTraversal(n, adj, 0);
            System.out.println("Test 4: " + (res.ok ? res.order.toString() : ("Error: " + res.error)));
        }

        // Test 5: Invalid start node
        {
            int n = 3;
            int[][] edges = { {0,1}, {1,2} };
            List<List<Integer>> adj = buildAdj(n, edges, false);
            DFSResult res = dfsTraversal(n, adj, 5);
            System.out.println("Test 5: " + (res.ok ? res.order.toString() : ("Error: " + res.error)));
        }
    }
}