import java.util.*;

public class Task162 {
    static class Edge {
        final int to;
        final long w;
        Edge(int to, long w) { this.to = to; this.w = w; }
    }

    private static final long INF = Long.MAX_VALUE / 4;

    public static long[] dijkstra(int n, List<List<Edge>> adj, int start) {
        if (n <= 0) throw new IllegalArgumentException("Number of nodes must be positive.");
        if (adj == null || adj.size() != n) throw new IllegalArgumentException("Adjacency list size must be n.");
        if (start < 0 || start >= n) throw new IllegalArgumentException("Start node out of range.");

        // Validate edges
        for (int u = 0; u < n; u++) {
            List<Edge> edges = adj.get(u);
            if (edges == null) throw new IllegalArgumentException("Adjacency list entry is null at node " + u);
            for (Edge e : edges) {
                if (e == null) throw new IllegalArgumentException("Null edge at node " + u);
                if (e.to < 0 || e.to >= n) throw new IllegalArgumentException("Edge points to invalid node: " + e.to);
                if (e.w < 0) throw new IllegalArgumentException("Negative weights not allowed for Dijkstra.");
            }
        }

        long[] dist = new long[n];
        Arrays.fill(dist, INF);
        boolean[] visited = new boolean[n];
        dist[start] = 0L;

        PriorityQueue<long[]> pq = new PriorityQueue<>(Comparator.comparingLong(a -> a[0]));
        pq.offer(new long[]{0L, start});

        while (!pq.isEmpty()) {
            long[] cur = pq.poll();
            long d = cur[0];
            int u = (int) cur[1];
            if (visited[u]) continue;
            visited[u] = true;

            for (Edge e : adj.get(u)) {
                if (visited[e.to]) continue;
                if (d > INF - e.w) continue; // prevent overflow
                long nd = d + e.w;
                if (nd < dist[e.to]) {
                    dist[e.to] = nd;
                    pq.offer(new long[]{nd, e.to});
                }
            }
        }

        return dist;
    }

    static List<List<Edge>> newAdj(int n) {
        List<List<Edge>> adj = new ArrayList<>(n);
        for (int i = 0; i < n; i++) adj.add(new ArrayList<>());
        return adj;
    }

    static void addEdge(List<List<Edge>> adj, int u, int v, long w) {
        if (u < 0 || u >= adj.size() || v < 0 || v >= adj.size()) {
            throw new IllegalArgumentException("Invalid edge endpoints.");
        }
        if (w < 0) throw new IllegalArgumentException("Negative weight not allowed.");
        adj.get(u).add(new Edge(v, w));
    }

    static void printResult(long[] dist) {
        for (int i = 0; i < dist.length; i++) {
            if (i > 0) System.out.print(" ");
            System.out.print(dist[i] >= INF ? "INF" : String.valueOf(dist[i]));
        }
        System.out.println();
    }

    public static void main(String[] args) {
        // Test case 1
        {
            int n = 5, s = 0;
            List<List<Edge>> adj = newAdj(n);
            addEdge(adj, 0, 1, 2);
            addEdge(adj, 0, 2, 5);
            addEdge(adj, 1, 2, 1);
            addEdge(adj, 1, 3, 2);
            addEdge(adj, 2, 3, 1);
            addEdge(adj, 3, 4, 3);
            long[] dist = dijkstra(n, adj, s);
            printResult(dist); // Expected: 0 2 3 4 7
        }

        // Test case 2
        {
            int n = 5, s = 0;
            List<List<Edge>> adj = newAdj(n);
            addEdge(adj, 0, 1, 1);
            addEdge(adj, 1, 2, 1);
            addEdge(adj, 2, 3, 1);
            addEdge(adj, 3, 4, 1);
            long[] dist = dijkstra(n, adj, s);
            printResult(dist); // Expected: 0 1 2 3 4
        }

        // Test case 3
        {
            int n = 5, s = 0;
            List<List<Edge>> adj = newAdj(n);
            addEdge(adj, 0, 1, 10);
            addEdge(adj, 0, 2, 3);
            addEdge(adj, 2, 1, 1);
            addEdge(adj, 2, 3, 2);
            addEdge(adj, 1, 3, 4);
            addEdge(adj, 3, 4, 2);
            addEdge(adj, 4, 1, 1);
            long[] dist = dijkstra(n, adj, s);
            printResult(dist); // Expected: 0 4 3 5 7
        }

        // Test case 4 (disconnected)
        {
            int n = 5, s = 0;
            List<List<Edge>> adj = newAdj(n);
            addEdge(adj, 1, 2, 2);
            addEdge(adj, 2, 3, 2);
            long[] dist = dijkstra(n, adj, s);
            printResult(dist); // Expected: 0 INF INF INF INF
        }

        // Test case 5 (classic)
        {
            int n = 6, s = 0;
            List<List<Edge>> adj = newAdj(n);
            addEdge(adj, 0, 1, 7);
            addEdge(adj, 0, 2, 9);
            addEdge(adj, 0, 5, 14);
            addEdge(adj, 1, 2, 10);
            addEdge(adj, 1, 3, 15);
            addEdge(adj, 2, 3, 11);
            addEdge(adj, 2, 5, 2);
            addEdge(adj, 3, 4, 6);
            addEdge(adj, 4, 5, 9);
            long[] dist = dijkstra(n, adj, s);
            printResult(dist); // Expected: 0 7 9 20 26 11
        }
    }
}