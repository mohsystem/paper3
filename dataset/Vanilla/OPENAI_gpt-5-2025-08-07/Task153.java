import java.util.*;

public class Task153 {
    public static int[] bfs(int n, int[][] edges, int start) {
        if (n <= 0 || start < 0 || start >= n) return new int[0];
        List<List<Integer>> adj = new ArrayList<>(n);
        for (int i = 0; i < n; i++) adj.add(new ArrayList<>());
        if (edges != null) {
            for (int[] e : edges) {
                if (e == null || e.length < 2) continue;
                int u = e[0], v = e[1];
                if (0 <= u && u < n && 0 <= v && v < n) {
                    adj.get(u).add(v);
                    adj.get(v).add(u);
                }
            }
        }
        for (List<Integer> lst : adj) Collections.sort(lst);
        boolean[] vis = new boolean[n];
        ArrayDeque<Integer> q = new ArrayDeque<>();
        List<Integer> order = new ArrayList<>();
        vis[start] = true;
        q.add(start);
        while (!q.isEmpty()) {
            int u = q.poll();
            order.add(u);
            for (int v : adj.get(u)) {
                if (!vis[v]) {
                    vis[v] = true;
                    q.add(v);
                }
            }
        }
        int[] res = new int[order.size()];
        for (int i = 0; i < order.size(); i++) res[i] = order.get(i);
        return res;
    }

    public static void main(String[] args) {
        int[][] e1 = {{0, 1}, {1, 2}, {2, 3}};
        System.out.println(Arrays.toString(bfs(4, e1, 0))); // [0, 1, 2, 3]

        int[][] e2 = {{3, 4}, {0, 1}};
        System.out.println(Arrays.toString(bfs(5, e2, 3))); // [3, 4]

        int[][] e3 = {{0, 1}, {1, 2}, {2, 0}, {2, 3}};
        System.out.println(Arrays.toString(bfs(4, e3, 1))); // [1, 0, 2, 3]

        int[][] e4 = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 2}, {4, 5}};
        System.out.println(Arrays.toString(bfs(6, e4, 2))); // [2, 0, 1, 3]

        int[][] e5 = {};
        System.out.println(Arrays.toString(bfs(1, e5, 0))); // [0]
    }
}