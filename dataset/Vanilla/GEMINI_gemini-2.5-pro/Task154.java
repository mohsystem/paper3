import java.util.*;

public class Task154 {

    /**
     * Inner class to represent a graph using an adjacency list.
     */
    static class Graph {
        private int V; // Number of vertices
        private LinkedList<Integer>[] adj; // Adjacency List representation

        /**
         * Graph constructor.
         * @param v Number of vertices in the graph.
         */
        @SuppressWarnings("unchecked")
        Graph(int v) {
            V = v;
            adj = new LinkedList[v];
            for (int i = 0; i < v; ++i) {
                adj[i] = new LinkedList<>();
            }
        }

        /**
         * Adds an edge to the graph.
         * @param v The source vertex.
         * @param w The destination vertex.
         */
        void addEdge(int v, int w) {
            adj[v].add(w);
        }

        /**
         * A recursive helper function for DFS.
         * @param v The current vertex.
         * @param visited An array to keep track of visited vertices.
         * @param result The list to store the DFS traversal order.
         */
        private void DFSUtil(int v, boolean[] visited, List<Integer> result) {
            visited[v] = true;
            result.add(v);

            for (Integer neighbor : adj[v]) {
                if (!visited[neighbor]) {
                    DFSUtil(neighbor, visited, result);
                }
            }
        }

        /**
         * Performs Depth First Search traversal starting from a given node.
         * @param startNode The starting node for the DFS.
         * @return A list of integers representing the DFS traversal.
         */
        public List<Integer> performDFS(int startNode) {
            List<Integer> result = new ArrayList<>();
            if (startNode >= V) {
                System.out.println("Start node is out of bounds.");
                return result;
            }
            boolean[] visited = new boolean[V];
            DFSUtil(startNode, visited, result);
            return result;
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Simple linear graph
        System.out.println("Test Case 1:");
        Graph g1 = new Graph(4);
        g1.addEdge(0, 1);
        g1.addEdge(1, 2);
        g1.addEdge(2, 3);
        System.out.println("DFS starting from node 0: " + g1.performDFS(0));

        // Test Case 2: Graph with a fork
        System.out.println("\nTest Case 2:");
        Graph g2 = new Graph(5);
        g2.addEdge(0, 1);
        g2.addEdge(0, 2);
        g2.addEdge(1, 3);
        g2.addEdge(1, 4);
        System.out.println("DFS starting from node 0: " + g2.performDFS(0));

        // Test Case 3: Graph with a cycle
        System.out.println("\nTest Case 3:");
        Graph g3 = new Graph(4);
        g3.addEdge(0, 1);
        g3.addEdge(1, 2);
        g3.addEdge(2, 0);
        g3.addEdge(2, 3);
        System.out.println("DFS starting from node 0: " + g3.performDFS(0));

        // Test Case 4: Disconnected graph (DFS only visits connected component)
        System.out.println("\nTest Case 4:");
        Graph g4 = new Graph(5);
        g4.addEdge(0, 1);
        g4.addEdge(0, 2);
        g4.addEdge(3, 4);
        System.out.println("DFS starting from node 0: " + g4.performDFS(0));
        System.out.println("DFS starting from node 3: " + g4.performDFS(3));

        // Test Case 5: Single-node graph
        System.out.println("\nTest Case 5:");
        Graph g5 = new Graph(1);
        System.out.println("DFS starting from node 0: " + g5.performDFS(0));
    }
}