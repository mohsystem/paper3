import java.util.*;

/*
Chain-of-Through process:
1) Problem understanding: Implement Dijkstra's algorithm to compute shortest paths from a given start node in a weighted graph with non-negative weights.
2) Security requirements: Validate inputs (node indices, non-negative weights), avoid integer overflow by using safe INF and checks, avoid unsafe casts, handle unreachable nodes.
3) Secure coding generation: Use adjacency lists, priority queue (min-heap), immutable checks on popped queue entries, and bounds checks.
4) Code review: Ensure no unvalidated accesses, avoid overflow in relaxation, guard against invalid inputs.
5) Secure code output: Final code incorporates validations, safe arithmetic, and clear data structures.
*/

public class Task162 {

    public static class Result {
        public final long[] dist;
        public final int[] parent;
        public Result(long[] d, int[] p) {
            this.dist = d;
            this.parent = p;
        }
    }

    // Dijkstra's algorithm: n = number of nodes [0..n-1], edges = {u,v,w} for directed edges, start = start node.
    // Returns distances and parents arrays.
    public static Result dijkstra(int n, int[][] edges, int start) {
        if (n <= 0) throw new IllegalArgumentException("Number of nodes must be positive");
        if (start < 0 || start >= n) throw new IllegalArgumentException("Start node out of range");
        // Build adjacency list with validation
        List<List<long[]>> g = new ArrayList<>(n); // each entry: {to, weight}
        for (int i = 0; i < n; i++) g.add(new ArrayList<>());
        for (int i = 0; i < edges.length; i++) {
            int[] e = edges[i];
            if (e == null || e.length < 3) continue;
            int u = e[0], v = e[1];
            long w = e[2];
            if (u < 0 || u >= n || v < 0 || v >= n) continue; // skip invalid indices
            if (w < 0) continue; // Dijkstra requires non-negative weights
            g.get(u).add(new long[]{v, w});
        }

        final long INF = Long.MAX_VALUE / 4; // safe INF to prevent overflow on addition
        long[] dist = new long[n];
        int[] parent = new int[n];
        Arrays.fill(dist, INF);
        Arrays.fill(parent, -1);
        dist[start] = 0;

        PriorityQueue<long[]> pq = new PriorityQueue<>(Comparator.comparingLong(a -> a[0])); // {dist, node}
        pq.offer(new long[]{0L, start});

        while (!pq.isEmpty()) {
            long[] cur = pq.poll();
            long d = cur[0];
            int u = (int) cur[1];
            if (d != dist[u]) continue; // stale entry

            for (long[] edge : g.get(u)) {
                int v = (int) edge[0];
                long w = edge[1];
                if (dist[u] <= INF - w) { // prevent overflow
                    long nd = dist[u] + w;
                    if (nd < dist[v]) {
                        dist[v] = nd;
                        parent[v] = u;
                        pq.offer(new long[]{nd, v});
                    }
                }
            }
        }

        return new Result(dist, parent);
    }

    private static void printArray(long[] arr) {
        System.out.print("[");
        for (int i = 0; i < arr.length; i++) {
            if (i > 0) System.out.print(", ");
            if (arr[i] >= Long.MAX_VALUE / 8) System.out.print("INF");
            else System.out.print(arr[i]);
        }
        System.out.println("]");
    }

    private static void printArray(int[] arr) {
        System.out.print("[");
        for (int i = 0; i < arr.length; i++) {
            if (i > 0) System.out.print(", ");
            System.out.print(arr[i]);
        }
        System.out.println("]");
    }

    public static void main(String[] args) {
        // Test case 1
        int n1 = 5;
        int[][] edges1 = new int[][]{
            {0,1,10},{0,2,3},{1,2,1},{1,3,2},{2,1,4},{2,3,8},{2,4,2},{3,4,7},{4,3,9}
        };
        Result r1 = dijkstra(n1, edges1, 0);
        System.out.println("Test 1 distances:"); printArray(r1.dist);
        System.out.println("Test 1 parents:"); printArray(r1.parent);

        // Test case 2 (zero-weight edges)
        int n2 = 4;
        int[][] edges2 = new int[][]{
            {0,1,0},{0,2,5},{1,2,1},{1,3,4},{2,3,0}
        };
        Result r2 = dijkstra(n2, edges2, 0);
        System.out.println("Test 2 distances:"); printArray(r2.dist);
        System.out.println("Test 2 parents:"); printArray(r2.parent);

        // Test case 3 (disconnected)
        int n3 = 5;
        int[][] edges3 = new int[][]{
            {0,1,2},{1,2,2}
        };
        Result r3 = dijkstra(n3, edges3, 0);
        System.out.println("Test 3 distances:"); printArray(r3.dist);
        System.out.println("Test 3 parents:"); printArray(r3.parent);

        // Test case 4 (undirected classic, added as both directions)
        int n4 = 6;
        int[][] edges4 = new int[][]{
            {0,1,7},{1,0,7},{0,2,9},{2,0,9},{0,5,14},{5,0,14},
            {1,2,10},{2,1,10},{1,3,15},{3,1,15},{2,3,11},{3,2,11},
            {2,5,2},{5,2,2},{3,4,6},{4,3,6},{4,5,9},{5,4,9}
        };
        Result r4 = dijkstra(n4, edges4, 0);
        System.out.println("Test 4 distances:"); printArray(r4.dist);
        System.out.println("Test 4 parents:"); printArray(r4.parent);

        // Test case 5 (directed asymmetric)
        int n5 = 5;
        int[][] edges5 = new int[][]{
            {0,1,2},{1,2,3},{0,2,10},{2,3,1},{3,4,1},{1,4,100}
        };
        Result r5 = dijkstra(n5, edges5, 0);
        System.out.println("Test 5 distances:"); printArray(r5.dist);
        System.out.println("Test 5 parents:"); printArray(r5.parent);
    }
}