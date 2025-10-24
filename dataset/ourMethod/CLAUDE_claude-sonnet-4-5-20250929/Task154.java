
import java.util.*;

public class Task154 {
    private static final int MAX_NODES = 10000;
    private static final int MAX_EDGES = 100000;
    
    public static List<Integer> depthFirstSearch(Map<Integer, List<Integer>> graph, int startNode) {
        if (graph == null) {
            throw new IllegalArgumentException("Graph cannot be null");
        }
        
        if (!graph.containsKey(startNode)) {
            throw new IllegalArgumentException("Start node not found in graph");
        }
        
        if (graph.size() > MAX_NODES) {
            throw new IllegalArgumentException("Graph exceeds maximum node limit");
        }
        
        int edgeCount = 0;
        for (List<Integer> neighbors : graph.values()) {
            if (neighbors != null) {
                edgeCount += neighbors.size();
            }
        }
        if (edgeCount > MAX_EDGES) {
            throw new IllegalArgumentException("Graph exceeds maximum edge limit");
        }
        
        List<Integer> result = new ArrayList<>();
        Set<Integer> visited = new HashSet<>();
        dfsHelper(graph, startNode, visited, result);
        return result;
    }
    
    private static void dfsHelper(Map<Integer, List<Integer>> graph, int node, 
                                   Set<Integer> visited, List<Integer> result) {
        if (visited.contains(node)) {
            return;
        }
        
        visited.add(node);
        result.add(node);
        
        List<Integer> neighbors = graph.get(node);
        if (neighbors != null) {
            for (Integer neighbor : neighbors) {
                if (neighbor == null) {
                    continue;
                }
                if (!graph.containsKey(neighbor)) {
                    continue;
                }
                dfsHelper(graph, neighbor, visited, result);
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple linear graph
        Map<Integer, List<Integer>> graph1 = new HashMap<>();
        graph1.put(1, Arrays.asList(2));
        graph1.put(2, Arrays.asList(3));
        graph1.put(3, new ArrayList<>());
        System.out.println("Test 1: " + depthFirstSearch(graph1, 1));
        
        // Test case 2: Graph with branches
        Map<Integer, List<Integer>> graph2 = new HashMap<>();
        graph2.put(1, Arrays.asList(2, 3));
        graph2.put(2, Arrays.asList(4));
        graph2.put(3, Arrays.asList(5));
        graph2.put(4, new ArrayList<>());
        graph2.put(5, new ArrayList<>());
        System.out.println("Test 2: " + depthFirstSearch(graph2, 1));
        
        // Test case 3: Graph with cycle
        Map<Integer, List<Integer>> graph3 = new HashMap<>();
        graph3.put(1, Arrays.asList(2));
        graph3.put(2, Arrays.asList(3));
        graph3.put(3, Arrays.asList(1, 4));
        graph3.put(4, new ArrayList<>());
        System.out.println("Test 3: " + depthFirstSearch(graph3, 1));
        
        // Test case 4: Single node
        Map<Integer, List<Integer>> graph4 = new HashMap<>();
        graph4.put(1, new ArrayList<>());
        System.out.println("Test 4: " + depthFirstSearch(graph4, 1));
        
        // Test case 5: Disconnected components
        Map<Integer, List<Integer>> graph5 = new HashMap<>();
        graph5.put(1, Arrays.asList(2));
        graph5.put(2, new ArrayList<>());
        graph5.put(3, Arrays.asList(4));
        graph5.put(4, new ArrayList<>());
        System.out.println("Test 5: " + depthFirstSearch(graph5, 1));
    }
}
