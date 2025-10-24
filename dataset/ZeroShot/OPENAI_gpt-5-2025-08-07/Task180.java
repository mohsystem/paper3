import java.util.*;

public class Task180 {

    // Reparent the tree at the specified root. Returns oriented children adjacency lists.
    // Throws IllegalArgumentException on invalid input.
    public static int[][] reparentTree(int n, int[][] edges, int root) {
        if (n < 1) throw new IllegalArgumentException("Number of nodes must be >= 1");
        if (root < 0 || root >= n) throw new IllegalArgumentException("Root out of range");
        if (edges == null) throw new IllegalArgumentException("Edges cannot be null");
        if (edges.length != n - 1) throw new IllegalArgumentException("Edges must be n-1 for a valid tree");

        // Validate edges and detect duplicates/self-loops
        long keyBaseShift = 32L;
        HashSet<Long> seen = new HashSet<>();
        List<List<Integer>> adj = new ArrayList<>(n);
        for (int i = 0; i < n; i++) adj.add(new ArrayList<>());

        for (int i = 0; i < edges.length; i++) {
            int[] e = edges[i];
            if (e == null || e.length != 2) throw new IllegalArgumentException("Invalid edge at index " + i);
            int u = e[0], v = e[1];
            if (u < 0 || u >= n || v < 0 || v >= n) throw new IllegalArgumentException("Edge node out of range");
            if (u == v) throw new IllegalArgumentException("Self-loop detected");
            int a = Math.min(u, v), b = Math.max(u, v);
            long key = ((long) a << keyBaseShift) ^ (long) b;
            if (!seen.add(key)) throw new IllegalArgumentException("Duplicate edge detected");
            adj.get(u).add(v);
            adj.get(v).add(u);
        }

        // BFS to orient away from root
        int[] parent = new int[n];
        Arrays.fill(parent, -1);
        ArrayDeque<Integer> q = new ArrayDeque<>();
        parent[root] = root;
        q.add(root);

        List<List<Integer>> children = new ArrayList<>(n);
        for (int i = 0; i < n; i++) children.add(new ArrayList<>());

        int visited = 0;
        while (!q.isEmpty()) {
            int u = q.poll();
            visited++;
            for (int v : adj.get(u)) {
                if (v != parent[u]) {
                    parent[v] = u;
                    children.get(u).add(v);
                    q.add(v);
                }
            }
        }
        if (visited != n) throw new IllegalArgumentException("Graph is not connected");

        int[][] result = new int[n][];
        for (int i = 0; i < n; i++) {
            List<Integer> lst = children.get(i);
            result[i] = new int[lst.size()];
            for (int j = 0; j < lst.size(); j++) result[i][j] = lst.get(j);
        }
        return result;
    }

    private static void printChildren(int[][] children) {
        if (children == null) {
            System.out.println("null");
            return;
        }
        for (int i = 0; i < children.length; i++) {
            System.out.print(i + ": [");
            for (int j = 0; j < children[i].length; j++) {
                if (j > 0) System.out.print(", ");
                System.out.print(children[i][j]);
            }
            System.out.println("]");
        }
    }

    public static void main(String[] args) {
        // Test 1: Example tree reparented at 6
        int n1 = 10;
        int[][] e1 = new int[][]{
                {0, 1}, {0, 2}, {0, 3},
                {1, 4}, {1, 5},
                {2, 6}, {2, 7},
                {3, 8}, {3, 9}
        };
        int root1 = 6;
        System.out.println("Test 1:");
        try {
            int[][] r1 = reparentTree(n1, e1, root1);
            printChildren(r1);
        } catch (Exception ex) {
            System.out.println("Error: " + ex.getMessage());
        }

        // Test 2: Chain 0-1-2-3 reparented at 3
        int n2 = 4;
        int[][] e2 = new int[][]{
                {0, 1}, {1, 2}, {2, 3}
        };
        int root2 = 3;
        System.out.println("Test 2:");
        try {
            int[][] r2 = reparentTree(n2, e2, root2);
            printChildren(r2);
        } catch (Exception ex) {
            System.out.println("Error: " + ex.getMessage());
        }

        // Test 3: Single node
        int n3 = 1;
        int[][] e3 = new int[][]{};
        int root3 = 0;
        System.out.println("Test 3:");
        try {
            int[][] r3 = reparentTree(n3, e3, root3);
            printChildren(r3);
        } catch (Exception ex) {
            System.out.println("Error: " + ex.getMessage());
        }

        // Test 4: Star centered at 0 reparented at 3
        int n4 = 5;
        int[][] e4 = new int[][]{
                {0, 1}, {0, 2}, {0, 3}, {0, 4}
        };
        int root4 = 3;
        System.out.println("Test 4:");
        try {
            int[][] r4 = reparentTree(n4, e4, root4);
            printChildren(r4);
        } catch (Exception ex) {
            System.out.println("Error: " + ex.getMessage());
        }

        // Test 5: Invalid (wrong number of edges)
        int n5 = 3;
        int[][] e5 = new int[][]{
                {0, 1}
        };
        int root5 = 0;
        System.out.println("Test 5:");
        try {
            int[][] r5 = reparentTree(n5, e5, root5);
            printChildren(r5);
        } catch (Exception ex) {
            System.out.println("Error: " + ex.getMessage());
        }
    }
}