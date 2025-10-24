
import java.util.*;

public class Task153 {
    private Map<Integer, List<Integer>> adjacencyList;
    
    public Task153() {
        this.adjacencyList = new HashMap<>();
    }
    
    public void addEdge(int source, int destination) {
        if (source < 0 || destination < 0) {
            throw new IllegalArgumentException("Node values must be non-negative");
        }
        adjacencyList.computeIfAbsent(source, k -> new ArrayList<>()).add(destination);
        adjacencyList.computeIfAbsent(destination, k -> new ArrayList<>());
    }
    
    public List<Integer> breadthFirstSearch(int startNode) {
        if (startNode < 0) {
            throw new IllegalArgumentException("Start node must be non-negative");
        }
        
        List<Integer> result = new ArrayList<>();
        if (!adjacencyList.containsKey(startNode)) {
            return result;
        }
        
        Set<Integer> visited = new HashSet<>();
        Queue<Integer> queue = new LinkedList<>();
        
        queue.offer(startNode);
        visited.add(startNode);
        
        while (!queue.isEmpty()) {
            int currentNode = queue.poll();
            result.add(currentNode);
            
            List<Integer> neighbors = adjacencyList.get(currentNode);
            if (neighbors != null) {
                for (int neighbor : neighbors) {
                    if (!visited.contains(neighbor)) {
                        visited.add(neighbor);
                        queue.offer(neighbor);
                    }
                }
            }
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple linear graph
        System.out.println("Test Case 1: Linear Graph");
        Task153 graph1 = new Task153();
        graph1.addEdge(1, 2);
        graph1.addEdge(2, 3);
        graph1.addEdge(3, 4);
        System.out.println("BFS from node 1: " + graph1.breadthFirstSearch(1));
        
        // Test case 2: Graph with multiple branches
        System.out.println("\\nTest Case 2: Graph with Multiple Branches");
        Task153 graph2 = new Task153();
        graph2.addEdge(0, 1);
        graph2.addEdge(0, 2);
        graph2.addEdge(1, 3);
        graph2.addEdge(1, 4);
        graph2.addEdge(2, 5);
        System.out.println("BFS from node 0: " + graph2.breadthFirstSearch(0));
        
        // Test case 3: Disconnected graph
        System.out.println("\\nTest Case 3: Disconnected Graph");
        Task153 graph3 = new Task153();
        graph3.addEdge(0, 1);
        graph3.addEdge(2, 3);
        System.out.println("BFS from node 0: " + graph3.breadthFirstSearch(0));
        
        // Test case 4: Graph with cycle
        System.out.println("\\nTest Case 4: Graph with Cycle");
        Task153 graph4 = new Task153();
        graph4.addEdge(0, 1);
        graph4.addEdge(1, 2);
        graph4.addEdge(2, 0);
        graph4.addEdge(2, 3);
        System.out.println("BFS from node 0: " + graph4.breadthFirstSearch(0));
        
        // Test case 5: Single node
        System.out.println("\\nTest Case 5: Single Node");
        Task153 graph5 = new Task153();
        graph5.addEdge(5, 5);
        System.out.println("BFS from node 5: " + graph5.breadthFirstSearch(5));
    }
}
