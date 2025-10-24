
import java.util.*;

public class Task153 {
    private static final int MAX_NODES = 10000;
    private static final int MAX_EDGES = 100000;
    
    public static class Graph {
        private final Map<Integer, List<Integer>> adjacencyList;
        private final int maxNodes;
        
        public Graph(int maxNodes) {
            if (maxNodes <= 0 || maxNodes > MAX_NODES) {
                throw new IllegalArgumentException("Invalid maxNodes");
            }
            this.maxNodes = maxNodes;
            this.adjacencyList = new HashMap<>();
        }
        
        public void addEdge(int from, int to) {
            if (from < 0 || from >= maxNodes || to < 0 || to >= maxNodes) {
                throw new IllegalArgumentException("Invalid node");
            }
            adjacencyList.computeIfAbsent(from, k -> new ArrayList<>()).add(to);
        }
        
        public List<Integer> getNeighbors(int node) {
            if (node < 0 || node >= maxNodes) {
                throw new IllegalArgumentException("Invalid node");
            }
            return adjacencyList.getOrDefault(node, Collections.emptyList());
        }
    }
    
    public static List<Integer> breadthFirstSearch(Graph graph, int startNode) {
        if (graph == null) {
            throw new IllegalArgumentException("Graph cannot be null");
        }
        if (startNode < 0 || startNode >= graph.maxNodes) {
            throw new IllegalArgumentException("Invalid start node");
        }
        
        List<Integer> result = new ArrayList<>();
        Set<Integer> visited = new HashSet<>();
        Queue<Integer> queue = new LinkedList<>();
        
        queue.add(startNode);
        visited.add(startNode);
        
        while (!queue.isEmpty()) {
            Integer current = queue.poll();
            result.add(current);
            
            for (Integer neighbor : graph.getNeighbors(current)) {
                if (!visited.contains(neighbor)) {
                    visited.add(neighbor);
                    queue.add(neighbor);
                }
            }
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple linear graph
        Graph graph1 = new Graph(5);
        graph1.addEdge(0, 1);
        graph1.addEdge(1, 2);
        graph1.addEdge(2, 3);
        graph1.addEdge(3, 4);
        List<Integer> result1 = breadthFirstSearch(graph1, 0);
        System.out.println("Test 1: " + result1);
        
        // Test case 2: Tree structure
        Graph graph2 = new Graph(7);
        graph2.addEdge(0, 1);
        graph2.addEdge(0, 2);
        graph2.addEdge(1, 3);
        graph2.addEdge(1, 4);
        graph2.addEdge(2, 5);
        graph2.addEdge(2, 6);
        List<Integer> result2 = breadthFirstSearch(graph2, 0);
        System.out.println("Test 2: " + result2);
        
        // Test case 3: Graph with cycle
        Graph graph3 = new Graph(4);
        graph3.addEdge(0, 1);
        graph3.addEdge(1, 2);
        graph3.addEdge(2, 3);
        graph3.addEdge(3, 1);
        List<Integer> result3 = breadthFirstSearch(graph3, 0);
        System.out.println("Test 3: " + result3);
        
        // Test case 4: Disconnected graph
        Graph graph4 = new Graph(6);
        graph4.addEdge(0, 1);
        graph4.addEdge(1, 2);
        graph4.addEdge(3, 4);
        graph4.addEdge(4, 5);
        List<Integer> result4 = breadthFirstSearch(graph4, 0);
        System.out.println("Test 4: " + result4);
        
        // Test case 5: Single node
        Graph graph5 = new Graph(1);
        List<Integer> result5 = breadthFirstSearch(graph5, 0);
        System.out.println("Test 5: " + result5);
    }
}
