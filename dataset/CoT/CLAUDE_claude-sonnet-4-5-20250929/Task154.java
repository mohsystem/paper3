
import java.util.*;

public class Task154 {
    private Map<Integer, List<Integer>> adjacencyList;
    
    public Task154() {
        this.adjacencyList = new HashMap<>();
    }
    
    public void addEdge(int source, int destination) {
        if (source < 0 || destination < 0) {
            throw new IllegalArgumentException("Node values must be non-negative");
        }
        adjacencyList.putIfAbsent(source, new ArrayList<>());
        adjacencyList.putIfAbsent(destination, new ArrayList<>());
        adjacencyList.get(source).add(destination);
    }
    
    public List<Integer> depthFirstSearch(int startNode) {
        if (startNode < 0) {
            throw new IllegalArgumentException("Start node must be non-negative");
        }
        
        List<Integer> result = new ArrayList<>();
        Set<Integer> visited = new HashSet<>();
        
        if (!adjacencyList.containsKey(startNode)) {
            return result;
        }
        
        dfsHelper(startNode, visited, result);
        return result;
    }
    
    private void dfsHelper(int node, Set<Integer> visited, List<Integer> result) {
        if (visited.contains(node)) {
            return;
        }
        
        visited.add(node);
        result.add(node);
        
        List<Integer> neighbors = adjacencyList.getOrDefault(node, new ArrayList<>());
        for (int neighbor : neighbors) {
            if (!visited.contains(neighbor)) {
                dfsHelper(neighbor, visited, result);
            }
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Simple linear graph
        System.out.println("Test Case 1: Linear Graph");
        Task154 graph1 = new Task154();
        graph1.addEdge(1, 2);
        graph1.addEdge(2, 3);
        graph1.addEdge(3, 4);
        System.out.println("DFS from node 1: " + graph1.depthFirstSearch(1));
        
        // Test Case 2: Graph with branches
        System.out.println("\\nTest Case 2: Branched Graph");
        Task154 graph2 = new Task154();
        graph2.addEdge(1, 2);
        graph2.addEdge(1, 3);
        graph2.addEdge(2, 4);
        graph2.addEdge(2, 5);
        graph2.addEdge(3, 6);
        System.out.println("DFS from node 1: " + graph2.depthFirstSearch(1));
        
        // Test Case 3: Graph with cycle
        System.out.println("\\nTest Case 3: Graph with Cycle");
        Task154 graph3 = new Task154();
        graph3.addEdge(1, 2);
        graph3.addEdge(2, 3);
        graph3.addEdge(3, 1);
        graph3.addEdge(3, 4);
        System.out.println("DFS from node 1: " + graph3.depthFirstSearch(1));
        
        // Test Case 4: Disconnected graph
        System.out.println("\\nTest Case 4: Disconnected Graph");
        Task154 graph4 = new Task154();
        graph4.addEdge(1, 2);
        graph4.addEdge(3, 4);
        graph4.addEdge(5, 6);
        System.out.println("DFS from node 1: " + graph4.depthFirstSearch(1));
        System.out.println("DFS from node 3: " + graph4.depthFirstSearch(3));
        
        // Test Case 5: Single node
        System.out.println("\\nTest Case 5: Single Node");
        Task154 graph5 = new Task154();
        graph5.addEdge(1, 1);
        System.out.println("DFS from node 1: " + graph5.depthFirstSearch(1));
    }
}
