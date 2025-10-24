import java.util.*;

public class Task162 {
    private static final long INF = Long.MAX_VALUE / 4;

    public static long[] dijkstra(int n, int[][] edges, int start) {
        if (n <= 0 || start < 0 || start >= n) {
            return null;
        }
        if (edges == null) {
            edges = new int[0][];
        }

        // Validate edges and build adjacency list
        List<List<Edge>> graph = new ArrayList<>(n);
        for (int i = 0; i < n; i++) {
            graph.add(new ArrayList<>());
        }

        for (int i = 0; i < edges.length; i++) {
            int[] e = edges[i];
            if (e == null || e.length != 3) {
                return null;
            }
            int u = e[0];
            int v = e[1];
            long w = (long) e[2];
            if (u < 0 || u >= n || v < 0 || v >= n) {
                return null;
            }
            if (w < 0 || w > INF / 2) {
                return null;
            }
            graph.get(u).add(new Edge(v, w));
        }

        long[] dist = new long[n];
        Arrays.fill(dist, INF);
        dist[start] = 0L;

        boolean[] visited = new boolean[n];
        PriorityQueue<Node> pq = new PriorityQueue<>(Comparator.comparingLong(a -> a.dist));
        pq.offer(new Node(start, 0L));

        while (!pq.isEmpty()) {
            Node cur = pq.poll();
            int u = cur.id;
            if (visited[u]) continue;
            visited[u] = true;

            for (Edge ed : graph.get(u)) {
                int v = ed.to;
                long w = ed.w;
                if (dist[u] <= INF - w) {
                    long alt = dist[u] + w;
                    if (alt < dist[v]) {
                        dist[v] = alt;
                        pq.offer(new Node(v, alt));
                    }
                }
            }
        }

        return dist;
    }

    private static final class Edge {
        final int to;
        final long w;
        Edge(int to, long w) {
            this.to = to;
            this.w = w;
        }
    }

    private static final class Node {
        final int id;
        final long dist;
        Node(int id, long dist) {
            this.id = id;
            this.dist = dist;
        }
    }

    private static void printResult(String title, long[] dist) {
        System.out.println(title);
        if (dist == null) {
            System.out.println("Invalid input");
            return;
        }
        for (int i = 0; i < dist.length; i++) {
            String s = (dist[i] >= INF) ? "INF" : Long.toString(dist[i]);
            System.out.print(s + (i + 1 == dist.length ? "" : " "));
        }
        System.out.println();
    }

    public static void main(String[] args) {
        // Test 1: Simple directed graph
        int n1 = 3;
        int[][] edges1 = new int[][]{
            {0, 1, 4},
            {0, 2, 1},
            {2, 1, 2}
        };
        printResult("Test 1", dijkstra(n1, edges1, 0));

        // Test 2: Disconnected nodes
        int n2 = 4;
        int[][] edges2 = new int[][]{
            {0, 1, 5}
        };
        printResult("Test 2", dijkstra(n2, edges2, 0));

        // Test 3: Zero-weight edges
        int n3 = 4;
        int[][] edges3 = new int[][]{
            {0, 1, 0},
            {1, 2, 0},
            {2, 3, 1}
        };
        printResult("Test 3", dijkstra(n3, edges3, 0));

        // Test 4: Larger weights with alternative shorter path
        int n4 = 5;
        int[][] edges4 = new int[][]{
            {0, 1, 1000000000},
            {1, 2, 1000000000},
            {0, 3, 1},
            {3, 4, 1},
            {4, 2, 1}
        };
        printResult("Test 4", dijkstra(n4, edges4, 0));

        // Test 5: Invalid input (negative weight)
        int n5 = 2;
        int[][] edges5 = new int[][]{
            {0, 1, -1}
        };
        printResult("Test 5", dijkstra(n5, edges5, 0));
    }
}