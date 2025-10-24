import java.util.ArrayList;
import java.util.List;

public class Task154 {

    // Graph class representing an adjacency list
    static class Graph {
        private final int V; // Number of vertices
        private final List<List<Integer>> adj; // Adjacency list

        public Graph(int V) {
            if (V < 0) {
                throw new IllegalArgumentException("Number of vertices must be non-negative");
            }
            this.V = V;
            adj = new ArrayList<>(V);
            for (int i = 0; i < V; i++) {
                adj.add(new ArrayList<>());
            }
        }

        // Function to add an edge to the graph (undirected)
        public void addEdge(int v, int w) {
            if (v < 0 || v >= V || w < 0 || w >= V) {
                System.err.println("Error: Vertex out of bounds.");
                return;
            }
            adj.get(v).add(w);
            adj.get(w).add(v); // For undirected graph
        }

        // A recursive helper function for DFS
        private void dfsUtil(int v, boolean[] visited, List<Integer> result) {
            visited[v] = true;
            result.add(v);

            for (int neighbor : adj.get(v)) {
                if (!visited[neighbor]) {
                    dfsUtil(neighbor, visited, result);
                }
            }
        }

        // The main function that performs DFS traversal
        public List<Integer> dfs(int startNode) {
            List<Integer> result = new ArrayList<>();
            if (startNode < 0 || startNode >= V) {
                System.err.println("Error: Start node is out of bounds.");
                return result; // Return empty list for invalid start node
            }
            
            boolean[] visited = new boolean[V];
            dfsUtil(startNode, visited, result);
            return result;
        }
    }

    public static void main(String[] args) {
        int numVertices = 8;
        Graph g = new Graph(numVertices);

        // Create a sample graph
        g.addEdge(0, 1);
        g.addEdge(0, 2);
        g.addEdge(1, 3);
        g.addEdge(1, 4);
        g.addEdge(2, 5);
        g.addEdge(2, 6);
        g.addEdge(3, 7);
        // Node 7 is connected to 3

        System.out.println("Depth First Traversal (starting from various nodes):");

        // Test Case 1
        int startNode1 = 0;
        System.out.println("Test Case 1 (Start from " + startNode1 + "): " + g.dfs(startNode1));

        // Test Case 2
        int startNode2 = 3;
        System.out.println("Test Case 2 (Start from " + startNode2 + "): " + g.dfs(startNode2));

        // Test Case 3
        int startNode3 = 2;
        System.out.println("Test Case 3 (Start from " + startNode3 + "): " + g.dfs(startNode3));

        // Test Case 4
        int startNode4 = 6;
        System.out.println("Test Case 4 (Start from " + startNode4 + "): " + g.dfs(startNode4));

        // Test Case 5
        int startNode5 = 5;
        System.out.println("Test Case 5 (Start from " + startNode5 + "): " + g.dfs(startNode5));
    }
}