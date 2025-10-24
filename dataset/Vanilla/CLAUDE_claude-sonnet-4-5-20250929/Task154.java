
import java.util.*;

public class Task154 {
    static class Graph {
        private int vertices;
        private List<List<Integer>> adjList;
        
        public Graph(int vertices) {
            this.vertices = vertices;
            adjList = new ArrayList<>();
            for (int i = 0; i < vertices; i++) {
                adjList.add(new ArrayList<>());
            }
        }
        
        public void addEdge(int src, int dest) {
            adjList.get(src).add(dest);
        }
        
        public List<Integer> dfs(int startNode) {
            List<Integer> result = new ArrayList<>();
            boolean[] visited = new boolean[vertices];
            dfsUtil(startNode, visited, result);
            return result;
        }
        
        private void dfsUtil(int node, boolean[] visited, List<Integer> result) {
            visited[node] = true;
            result.add(node);
            
            for (int neighbor : adjList.get(node)) {
                if (!visited[neighbor]) {
                    dfsUtil(neighbor, visited, result);
                }
            }
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Simple linear graph
        System.out.println("Test Case 1: Linear Graph (0->1->2->3)");
        Graph g1 = new Graph(4);
        g1.addEdge(0, 1);
        g1.addEdge(1, 2);
        g1.addEdge(2, 3);
        System.out.println("DFS from node 0: " + g1.dfs(0));
        
        // Test Case 2: Graph with branching
        System.out.println("\\nTest Case 2: Branching Graph");
        Graph g2 = new Graph(7);
        g2.addEdge(0, 1);
        g2.addEdge(0, 2);
        g2.addEdge(1, 3);
        g2.addEdge(1, 4);
        g2.addEdge(2, 5);
        g2.addEdge(2, 6);
        System.out.println("DFS from node 0: " + g2.dfs(0));
        
        // Test Case 3: Graph with cycle
        System.out.println("\\nTest Case 3: Graph with Cycle");
        Graph g3 = new Graph(4);
        g3.addEdge(0, 1);
        g3.addEdge(1, 2);
        g3.addEdge(2, 0);
        g3.addEdge(2, 3);
        System.out.println("DFS from node 0: " + g3.dfs(0));
        
        // Test Case 4: Disconnected graph
        System.out.println("\\nTest Case 4: Disconnected Graph");
        Graph g4 = new Graph(5);
        g4.addEdge(0, 1);
        g4.addEdge(1, 2);
        g4.addEdge(3, 4);
        System.out.println("DFS from node 0: " + g4.dfs(0));
        System.out.println("DFS from node 3: " + g4.dfs(3));
        
        // Test Case 5: Single node
        System.out.println("\\nTest Case 5: Single Node");
        Graph g5 = new Graph(1);
        System.out.println("DFS from node 0: " + g5.dfs(0));
    }
}
