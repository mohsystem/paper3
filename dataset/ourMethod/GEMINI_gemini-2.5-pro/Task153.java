import java.util.*;

class Graph {
    private int numVertices;
    private List<List<Integer>> adj;

    public Graph(int vertices) {
        if (vertices < 0) {
            throw new IllegalArgumentException("Number of vertices cannot be negative");
        }
        this.numVertices = vertices;
        adj = new ArrayList<>(vertices);
        for (int i = 0; i < vertices; i++) {
            adj.add(new LinkedList<>());
        }
    }

    public void addEdge(int src, int dest) {
        if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) {
            System.err.println("Error: Invalid vertex for edge (" + src + ", " + dest + ")");
            return;
        }
        adj.get(src).add(dest);
    }

    public List<Integer> bfs(int startNode) {
        List<Integer> result = new ArrayList<>();
        if (startNode < 0 || startNode >= numVertices) {
            System.err.println("Error: Start node " + startNode + " is out of bounds.");
            return result;
        }

        boolean[] visited = new boolean[numVertices];
        Queue<Integer> queue = new LinkedList<>();

        visited[startNode] = true;
        queue.add(startNode);

        while (!queue.isEmpty()) {
            int currentNode = queue.poll();
            result.add(currentNode);

            for (Integer neighbor : adj.get(currentNode)) {
                if (neighbor >= 0 && neighbor < numVertices && !visited[neighbor]) {
                    visited[neighbor] = true;
                    queue.add(neighbor);
                }
            }
        }
        return result;
    }
}

public class Task153 {
    public static void main(String[] args) {
        // Test Case 1: Simple connected graph
        System.out.println("Test Case 1: Simple connected graph");
        Graph g1 = new Graph(6);
        g1.addEdge(0, 1);
        g1.addEdge(0, 2);
        g1.addEdge(1, 3);
        g1.addEdge(2, 4);
        g1.addEdge(3, 5);
        System.out.println("BFS starting from node 0: " + g1.bfs(0));

        // Test Case 2: Start from a different node
        System.out.println("\nTest Case 2: Start from a different node");
        Graph g2 = new Graph(4);
        g2.addEdge(0, 1);
        g2.addEdge(0, 2);
        g2.addEdge(1, 2);
        g2.addEdge(2, 0);
        g2.addEdge(2, 3);
        g2.addEdge(3, 3);
        System.out.println("BFS starting from node 2: " + g2.bfs(2));

        // Test Case 3: Graph with a cycle
        System.out.println("\nTest Case 3: Graph with a cycle");
        Graph g3 = new Graph(3);
        g3.addEdge(0, 1);
        g3.addEdge(1, 2);
        g3.addEdge(2, 0);
        System.out.println("BFS starting from node 0: " + g3.bfs(0));

        // Test Case 4: Disconnected graph
        System.out.println("\nTest Case 4: Disconnected graph");
        Graph g4 = new Graph(5);
        g4.addEdge(0, 1);
        g4.addEdge(1, 2);
        g4.addEdge(3, 4);
        System.out.println("BFS starting from node 0: " + g4.bfs(0));
        System.out.println("BFS starting from node 3: " + g4.bfs(3));

        // Test Case 5: Invalid start node
        System.out.println("\nTest Case 5: Invalid start node");
        Graph g5 = new Graph(3);
        g5.addEdge(0, 1);
        System.out.println("BFS starting from node 5: " + g5.bfs(5));
    }
}