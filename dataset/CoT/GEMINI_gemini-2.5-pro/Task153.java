import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

public class Task153 {

    static class Graph {
        private int V; // Number of vertices
        private List<List<Integer>> adj; // Adjacency List

        Graph(int v) {
            V = v;
            adj = new ArrayList<>(v);
            for (int i = 0; i < v; ++i) {
                adj.add(new LinkedList<>());
            }
        }

        void addEdge(int v, int w) {
            if (v >= 0 && v < V && w >= 0 && w < V) {
                adj.get(v).add(w);
                adj.get(w).add(v); // For undirected graph
            }
        }

        List<Integer> bfs(int startNode) {
            List<Integer> result = new ArrayList<>();
            if (startNode < 0 || startNode >= V) {
                System.err.println("Error: Start node " + startNode + " is out of bounds.");
                return result; // Return empty list for invalid start node
            }

            boolean[] visited = new boolean[V];
            Queue<Integer> queue = new LinkedList<>();

            visited[startNode] = true;
            queue.add(startNode);

            while (!queue.isEmpty()) {
                int currentNode = queue.poll();
                result.add(currentNode);

                for (int neighbor : adj.get(currentNode)) {
                    if (!visited[neighbor]) {
                        visited[neighbor] = true;
                        queue.add(neighbor);
                    }
                }
            }
            return result;
        }
    }

    public static void main(String[] args) {
        // Test Cases
        System.out.println("--- Breadth-First Search (BFS) Traversal ---");

        // Graph structure for all test cases
        //        0
        //       / \
        //      1---2
        //     / \ |
        //    3---4 5
        //         |
        //         6
        int numVertices = 7;
        Graph g = new Graph(numVertices);
        g.addEdge(0, 1);
        g.addEdge(0, 2);
        g.addEdge(1, 3);
        g.addEdge(1, 4);
        g.addEdge(2, 5);
        g.addEdge(3, 4);
        g.addEdge(5, 6);

        int[] startNodes = {0, 3, 6, 4, 1};

        for (int i = 0; i < startNodes.length; i++) {
            System.out.println("\nTest Case " + (i + 1) + ": Starting from node " + startNodes[i]);
            List<Integer> bfsResult = g.bfs(startNodes[i]);
            System.out.print("BFS Traversal: ");
            for (int node : bfsResult) {
                System.out.print(node + " ");
            }
            System.out.println();
        }
    }
}