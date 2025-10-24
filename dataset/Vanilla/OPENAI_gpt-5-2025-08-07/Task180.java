import java.util.*;

public class Task180 {

    // Reparent the tree to the perspective of 'root'
    // n: number of nodes labeled [0..n-1]
    // undirected edges as pairs [u, v]
    // returns directed edges (parent -> child) from the new root
    public static List<int[]> reparent(int n, int[][] edges, int root) {
        List<List<Integer>> g = new ArrayList<>(n);
        for (int i = 0; i < n; i++) g.add(new ArrayList<>());
        for (int[] e : edges) {
            int u = e[0], v = e[1];
            g.get(u).add(v);
            g.get(v).add(u);
        }
        boolean[] vis = new boolean[n];
        Queue<Integer> q = new ArrayDeque<>();
        List<int[]> res = new ArrayList<>(n - 1);

        vis[root] = true;
        q.add(root);
        while (!q.isEmpty()) {
            int u = q.poll();
            for (int v : g.get(u)) {
                if (!vis[v]) {
                    vis[v] = true;
                    res.add(new int[]{u, v});
                    q.add(v);
                }
            }
        }
        return res;
    }

    private static void printDirectedEdges(List<int[]> dedges) {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int i = 0; i < dedges.size(); i++) {
            int[] e = dedges.get(i);
            sb.append("(").append(e[0]).append("->").append(e[1]).append(")");
            if (i + 1 < dedges.size()) sb.append(", ");
        }
        sb.append("]");
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        // Test case 1: Example from prompt, root = 6
        int n1 = 10;
        int[][] edges1 = {
                {0,1},{0,2},{0,3},
                {1,4},{1,5},
                {2,6},{2,7},
                {3,8},{3,9}
        };
        System.out.println("Test 1: root=6");
        printDirectedEdges(reparent(n1, edges1, 6));

        // Test case 2: Same tree, root = 0
        System.out.println("Test 2: root=0");
        printDirectedEdges(reparent(n1, edges1, 0));

        // Test case 3: Line tree, root = 3
        int n3 = 5;
        int[][] edges3 = {{0,1},{1,2},{2,3},{3,4}};
        System.out.println("Test 3: root=3");
        printDirectedEdges(reparent(n3, edges3, 3));

        // Test case 4: Star tree, root = 4
        int n4 = 6;
        int[][] edges4 = {{0,1},{0,2},{0,3},{0,4},{0,5}};
        System.out.println("Test 4: root=4");
        printDirectedEdges(reparent(n4, edges4, 4));

        // Test case 5: Random tree, root = 4
        int n5 = 7;
        int[][] edges5 = {{0,1},{1,2},{1,3},{3,4},{2,5},{5,6}};
        System.out.println("Test 5: root=4");
        printDirectedEdges(reparent(n5, edges5, 4));
    }
}