import java.util.*;

public class Task162 {

    // Dijkstra's algorithm: n nodes (0..n-1), edges as [u, v, w], start node
    public static long[] dijkstra(int n, int[][] edges, int start) {
        List<int[]>[] adj = new ArrayList[n];
        for (int i = 0; i < n; i++) adj[i] = new ArrayList<>();
        for (int[] e : edges) {
            int u = e[0], v = e[1], w = e[2];
            adj[u].add(new int[]{v, w});
        }
        long INF = Long.MAX_VALUE / 4;
        long[] dist = new long[n];
        Arrays.fill(dist, INF);
        dist[start] = 0;

        PriorityQueue<long[]> pq = new PriorityQueue<>(Comparator.comparingLong(a -> a[0]));
        pq.offer(new long[]{0L, start});

        boolean[] visited = new boolean[n];
        while (!pq.isEmpty()) {
            long[] cur = pq.poll();
            long d = cur[0];
            int u = (int) cur[1];
            if (visited[u]) continue;
            visited[u] = true;
            for (int[] vw : adj[u]) {
                int v = vw[0], w = vw[1];
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    pq.offer(new long[]{dist[v], v});
                }
            }
        }
        return dist;
    }

    private static void printDistances(long[] dist) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < dist.length; i++) {
            if (i > 0) sb.append(" ");
            sb.append(dist[i] >= Long.MAX_VALUE / 8 ? "INF" : Long.toString(dist[i]));
        }
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        // Test 1: Directed graph
        int n1 = 5;
        int[][] edges1 = {
                {0,1,10},{0,2,3},{1,2,1},{2,1,4},{1,3,2},{2,3,2},{3,4,7},{2,4,8},{4,3,9}
        };
        long[] d1 = dijkstra(n1, edges1, 0);
        printDistances(d1); // Expected: 0 7 3 5 11

        // Test 2: Undirected graph (edges both directions)
        int n2 = 4;
        int[][] edges2 = {
                {0,1,5},{1,0,5},{1,2,6},{2,1,6},{0,3,10},{3,0,10},{2,3,2},{3,2,2}
        };
        long[] d2 = dijkstra(n2, edges2, 0);
        printDistances(d2); // Expected: 0 5 11 10

        // Test 3: Unreachable nodes
        int n3 = 4;
        int[][] edges3 = {
                {0,1,2}
        };
        long[] d3 = dijkstra(n3, edges3, 0);
        printDistances(d3); // Expected: 0 2 INF INF

        // Test 4: Zero-weight edges
        int n4 = 5;
        int[][] edges4 = {
                {0,1,0},{1,2,0},{2,3,0},{3,4,0}
        };
        long[] d4 = dijkstra(n4, edges4, 0);
        printDistances(d4); // Expected: 0 0 0 0 0

        // Test 5: Undirected weighted graph (classic)
        int n5 = 6;
        int[][] edges5 = {
                {0,1,7},{1,0,7},{0,2,9},{2,0,9},{0,5,14},{5,0,14},
                {1,2,10},{2,1,10},{1,3,15},{3,1,15},{2,3,11},{3,2,11},
                {2,5,2},{5,2,2},{3,4,6},{4,3,6},{4,5,9},{5,4,9}
        };
        long[] d5 = dijkstra(n5, edges5, 0);
        printDistances(d5); // Expected: 0 7 9 20 20 11
    }
}