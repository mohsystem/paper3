
import java.util.*;

public class Task153 {
    static class Graph {
        private int vertices;
        private List<List<Integer>> adjacencyList;
        
        public Graph(int vertices) {
            this.vertices = vertices;
            adjacencyList = new ArrayList<>();
            for (int i = 0; i < vertices; i++) {
                adjacencyList.add(new ArrayList<>());
            }
        }
        
        public void addEdge(int source, int destination) {
            adjacencyList.get(source).add(destination);
        }
        
        public List<Integer> bfs(int startNode) {
            List<Integer> result = new ArrayList<>();
            boolean[] visited = new boolean[vertices];
            Queue<Integer> queue = new LinkedList<>();
            
            visited[startNode] = true;
            queue.offer(startNode);
            
            while (!queue.isEmpty()) {
                int current = queue.poll();
                result.add(current);
                
                for (int neighbor : adjacencyList.get(current)) {
                    if (!visited[neighbor]) {
                        visited[neighbor] = true;
                        queue.offer(neighbor);
                    }
                }
            }
            
            return result;
        }
    }
    
    public static List<Integer> breadthFirstSearch(int vertices, int[][] edges, int startNode) {
        Graph graph = new Graph(vertices);
        for (int[] edge : edges) {
            graph.addEdge(edge[0], edge[1]);
        }
        return graph.bfs(startNode);
    }
    
    public static void main(String[] args) {
        // Test Case 1: Simple linear graph
        System.out.println("Test Case 1:");
        int[][] edges1 = {{0, 1}, {1, 2}, {2, 3}};
        System.out.println(breadthFirstSearch(4, edges1, 0));
        
        // Test Case 2: Graph with multiple branches
        System.out.println("\\nTest Case 2:");
        int[][] edges2 = {{0, 1}, {0, 2}, {1, 3}, {1, 4}, {2, 5}};
        System.out.println(breadthFirstSearch(6, edges2, 0));
        
        // Test Case 3: Disconnected graph
        System.out.println("\\nTest Case 3:");
        int[][] edges3 = {{0, 1}, {2, 3}};
        System.out.println(breadthFirstSearch(4, edges3, 0));
        
        // Test Case 4: Graph with cycles
        System.out.println("\\nTest Case 4:");
        int[][] edges4 = {{0, 1}, {0, 2}, {1, 2}, {2, 0}, {2, 3}};
        System.out.println(breadthFirstSearch(4, edges4, 0));
        
        // Test Case 5: Single node
        System.out.println("\\nTest Case 5:");
        int[][] edges5 = {};
        System.out.println(breadthFirstSearch(1, edges5, 0));
    }
}
