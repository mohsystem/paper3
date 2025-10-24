import java.util.*;

public class Task153 {
    public static List<Integer> bfs(int n, int[][] edges, int start, boolean undirected) {
        List<Integer> result = new ArrayList<>();
        if (n <= 0 || start < 0 || start >= n) {
            return result;
        }
        // Build adjacency list
        List<List<Integer>> adj = new ArrayList<>(n);
        for (int i = 0; i < n; i++) adj.add(new ArrayList<>());

        if (edges != null) {
            for (int[] e : edges) {
                if (e == null || e.length < 2) continue;
                int u = e[0], v = e[1];
                if (u < 0 || u >= n || v < 0 || v >= n) continue;
                adj.get(u).add(v);
                if (undirected && u != v) adj.get(v).add(u);
            }
        }

        // Sort and deduplicate neighbors for deterministic and efficient BFS
        for (int i = 0; i < n; i++) {
            List<Integer> list = adj.get(i);
            Collections.sort(list);
            List<Integer> dedup = new ArrayList<>(list.size());
            Integer prev = null;
            for (Integer val : list) {
                if (!Objects.equals(prev, val)) {
                    dedup.add(val);
                    prev = val;
                }
            }
            adj.set(i, dedup);
        }

        boolean[] visited = new boolean[n];
        ArrayDeque<Integer> q = new ArrayDeque<>();
        visited[start] = true;
        q.add(start);

        while (!q.isEmpty()) {
            int u = q.remove();
            result.add(u);
            for (int v : adj.get(u)) {
                if (!visited[v]) {
                    visited[v] = true;
                    q.add(v);
                }
            }
        }
        return result;
    }

    private static void runTest(int testNo, int n, int[][] edges, int start, boolean undirected) {
        List<Integer> order = bfs(n, edges, start, undirected);
        System.out.println("Test " + testNo + ": " + order);
    }

    public static void main(String[] args) {
        // Test 1: Simple undirected chain
        runTest(1, 4, new int[][]{{0,1},{1,2},{2,3}}, 0, true);
        // Test 2: Undirected graph with cycles
        runTest(2, 4, new int[][]{{0,1},{0,2},{1,2},{2,3}}, 1, true);
        // Test 3: Disconnected graph
        runTest(3, 5, new int[][]{{3,4}}, 3, true);
        // Test 4: Directed cycle
        runTest(4, 3, new int[][]{{0,1},{1,2},{2,0}}, 1, false);
        // Test 5: Invalid edges and self-loop
        runTest(5, 4, new int[][]{{-1,2},{1,4},{0,0}}, 0, true);
    }
}