
import java.util.*;

public class Task180 {
    private Map<Integer, List<Integer>> adjacencyList;
    
    public Task180(Map<Integer, List<Integer>> adjacencyList) {
        this.adjacencyList = adjacencyList;
    }
    
    public Map<Integer, List<Integer>> reparent(int newRoot) {
        Map<Integer, List<Integer>> newTree = new HashMap<>();
        Set<Integer> visited = new HashSet<>();
        
        // Initialize all nodes in the new tree
        for (Integer node : adjacencyList.keySet()) {
            newTree.put(node, new ArrayList<>());
        }
        
        // Build new tree using BFS from newRoot
        buildTree(newRoot, -1, newTree, visited);
        
        return newTree;
    }
    
    private void buildTree(int current, int parent, Map<Integer, List<Integer>> newTree, Set<Integer> visited) {
        visited.add(current);
        
        // Get all neighbors of current node
        List<Integer> neighbors = adjacencyList.getOrDefault(current, new ArrayList<>());
        
        for (Integer neighbor : neighbors) {
            if (!visited.contains(neighbor)) {
                newTree.get(current).add(neighbor);
                buildTree(neighbor, current, newTree, visited);
            }
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Example from problem description
        Map<Integer, List<Integer>> tree1 = new HashMap<>();
        tree1.put(0, Arrays.asList(1, 2, 3));
        tree1.put(1, Arrays.asList(0, 4, 5));
        tree1.put(2, Arrays.asList(0, 6, 7));
        tree1.put(3, Arrays.asList(0, 8, 9));
        tree1.put(4, Arrays.asList(1));
        tree1.put(5, Arrays.asList(1));
        tree1.put(6, Arrays.asList(2));
        tree1.put(7, Arrays.asList(2));
        tree1.put(8, Arrays.asList(3));
        tree1.put(9, Arrays.asList(3));
        
        Task180 t1 = new Task180(tree1);
        Map<Integer, List<Integer>> result1 = t1.reparent(6);
        System.out.println("Test 1 - Reparent on node 6: " + result1);
        
        // Test Case 2: Simple tree
        Map<Integer, List<Integer>> tree2 = new HashMap<>();
        tree2.put(0, Arrays.asList(1, 2));
        tree2.put(1, Arrays.asList(0));
        tree2.put(2, Arrays.asList(0));
        
        Task180 t2 = new Task180(tree2);
        Map<Integer, List<Integer>> result2 = t2.reparent(1);
        System.out.println("Test 2 - Reparent on node 1: " + result2);
        
        // Test Case 3: Linear tree
        Map<Integer, List<Integer>> tree3 = new HashMap<>();
        tree3.put(0, Arrays.asList(1));
        tree3.put(1, Arrays.asList(0, 2));
        tree3.put(2, Arrays.asList(1, 3));
        tree3.put(3, Arrays.asList(2));
        
        Task180 t3 = new Task180(tree3);
        Map<Integer, List<Integer>> result3 = t3.reparent(3);
        System.out.println("Test 3 - Reparent on node 3: " + result3);
        
        // Test Case 4: Single node
        Map<Integer, List<Integer>> tree4 = new HashMap<>();
        tree4.put(0, new ArrayList<>());
        
        Task180 t4 = new Task180(tree4);
        Map<Integer, List<Integer>> result4 = t4.reparent(0);
        System.out.println("Test 4 - Single node: " + result4);
        
        // Test Case 5: Three nodes
        Map<Integer, List<Integer>> tree5 = new HashMap<>();
        tree5.put(0, Arrays.asList(1));
        tree5.put(1, Arrays.asList(0, 2));
        tree5.put(2, Arrays.asList(1));
        
        Task180 t5 = new Task180(tree5);
        Map<Integer, List<Integer>> result5 = t5.reparent(2);
        System.out.println("Test 5 - Reparent on node 2: " + result5);
    }
}
