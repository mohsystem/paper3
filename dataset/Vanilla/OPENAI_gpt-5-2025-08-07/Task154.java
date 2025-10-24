import java.util.*;

public class Task154 {
    public static List<Integer> dfs(int n, int[][] edges, int start, boolean directed) {
        List<List<Integer>> adj = new ArrayList<>();
        for (int i = 0; i < n; i++) adj.add(new ArrayList<>());
        for (int[] e : edges) {
            int u = e[0], v = e[1];
            if (u < 0 || u >= n || v < 0 || v >= n) continue;
            adj.get(u).add(v);
            if (!directed) adj.get(v).add(u);
        }
        for (List<Integer> nbrs : adj) Collections.sort(nbrs);
        List<Integer> order = new ArrayList<>();
        if (start < 0 || start >= n) return order;
        boolean[] vis = new boolean[n];
        dfsRec(start, adj, vis, order);
        return order;
    }

    private static void dfsRec(int u, List<List<Integer>> adj, boolean[] vis, List<Integer> order) {
        vis[u] = true;
        order.add(u);
        for (int v : adj.get(u)) {
            if (!vis[v]) dfsRec(v, adj, vis, order);
        }
    }

    private static void printResult(String label, List<Integer> res) {
        System.out.println(label + ": " + res);
    }

    public static void main(String[] args) {
        // Test 1: Line graph undirected
        int n1 = 5;
        int[][] e1 = { {0,1},{1,2},{2,3},{3,4} };
        printResult("Test1", dfs(n1, e1, 0, false));

        // Test 2: Branching undirected
        int n2 = 6;
        int[][] e2 = { {0,1},{0,2},{1,3},{1,4},{2,5} };
        printResult("Test2", dfs(n2, e2, 0, false));

        // Test 3: Cycle undirected
        int n3 = 4;
        int[][] e3 = { {0,1},{1,2},{2,0},{2,3} };
        printResult("Test3", dfs(n3, e3, 0, false));

        // Test 4: Disconnected, start isolated
        int n4 = 5;
        int[][] e4 = { {0,1},{1,2} };
        printResult("Test4", dfs(n4, e4, 3, false));

        // Test 5: Directed graph
        int n5 = 5;
        int[][] e5 = { {0,1},{0,2},{1,3},{3,4} };
        printResult("Test5", dfs(n5, e5, 0, true));
    }
}