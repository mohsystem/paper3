
import java.util.*;

public class Task154 {
    static class Graph {
        private int vertices;
        private LinkedList<Integer>[] adjacencyList;
        
        @SuppressWarnings("unchecked")
        public Graph(int vertices) {
            this.vertices = vertices;
            adjacencyList = new LinkedList[vertices];
            for (int i = 0; i < vertices; i++) {
                adjacencyList[i] = new LinkedList<>();
            }
        }
        
        public void addEdge(int source, int destination) {
            if (source >= 0 && source < vertices && destination >= 0 && destination < vertices) {
                adjacencyList[source].add(destination);
            }
        }
        
        public List<Integer> depthFirstSearch(int startNode) {
            List<Integer> result = new ArrayList<>();
            if (startNode < 0 || startNode >= vertices) {
                return result;
            }
            
            boolean[] visited = new boolean[vertices];
            dfsUtil(startNode, visited, result);
            return result;
        }
        
        private void dfsUtil(int node, boolean[] visited, List<Integer> result) {
            visited[node] = true;
            result.add(node);
            
            for (int neighbor : adjacencyList[node]) {
                if (!visited[neighbor]) {
                    dfsUtil(neighbor, visited, result);
                }
            }
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Simple linear graph
        System.out.println("Test Case 1: Linear Graph");
        Graph graph1 = new Graph(5);
        graph1.addEdge(0, 1);
        graph1.addEdge(1, 2);
        graph1.addEdge(2, 3);
        graph1.addEdge(3, 4);
        System.out.println("DFS from node 0: " + graph1.depthFirstSearch(0));
        
        // Test Case 2: Graph with branches
        System.out.println("\\nTest Case 2: Branched Graph");
        Graph graph2 = new Graph(7);
        graph2.addEdge(0, 1);
        graph2.addEdge(0, 2);
        graph2.addEdge(1, 3);
        graph2.addEdge(1, 4);
        graph2.addEdge(2, 5);
        graph2.addEdge(2, 6);
        System.out.println("DFS from node 0: " + graph2.depthFirstSearch(0));
        
        // Test Case 3: Graph with cycle
        System.out.println("\\nTest Case 3: Graph with Cycle");
        Graph graph3 = new Graph(4);
        graph3.addEdge(0, 1);
        graph3.addEdge(1, 2);
        graph3.addEdge(2, 3);
        graph3.addEdge(3, 1);
        System.out.println("DFS from node 0: " + graph3.depthFirstSearch(0));
        
        // Test Case 4: Disconnected graph
        System.out.println("\\nTest Case 4: Disconnected Graph");
        Graph graph4 = new Graph(6);
        graph4.addEdge(0, 1);
        graph4.addEdge(1, 2);
        graph4.addEdge(3, 4);
        graph4.addEdge(4, 5);
        System.out.println("DFS from node 0: " + graph4.depthFirstSearch(0));
        System.out.println("DFS from node 3: " + graph4.depthFirstSearch(3));
        
        // Test Case 5: Single node
        System.out.println("\\nTest Case 5: Single Node");
        Graph graph5 = new Graph(1);
        System.out.println("DFS from node 0: " + graph5.depthFirstSearch(0));
    }
}
