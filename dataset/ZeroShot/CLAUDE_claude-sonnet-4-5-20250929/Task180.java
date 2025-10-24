
import java.util.*;

public class Task180 {
    private Map<Integer, List<Integer>> adjacencyList;
    
    public Task180() {
        adjacencyList = new HashMap<>();
    }
    
    public Map<Integer, List<Integer>> reparentTree(Map<Integer, List<Integer>> tree, int newRoot) {
        if (tree == null || tree.isEmpty()) {
            return new HashMap<>();
        }
        
        if (!tree.containsKey(newRoot)) {
            return new HashMap<>();
        }
        
        Map<Integer, List<Integer>> result = new HashMap<>();
        Set<Integer> visited = new HashSet<>();
        
        buildReparentedTree(tree, newRoot, -1, visited, result);
        
        return result;
    }
    
    private void buildReparentedTree(Map<Integer, List<Integer>> tree, int current, int parent, 
                                     Set<Integer> visited, Map<Integer, List<Integer>> result) {
        visited.add(current);
        result.putIfAbsent(current, new ArrayList<>());
        
        if (!tree.containsKey(current)) {
            return;
        }
        
        for (int neighbor : tree.get(current)) {
            if (!visited.contains(neighbor)) {
                result.get(current).add(neighbor);
                buildReparentedTree(tree, neighbor, current, visited, result);
            }
        }
        
        if (parent != -1) {
            result.get(current).add(parent);
        }
    }
    
    public static void main(String[] args) {
        Task180 solution = new Task180();
        
        // Test case 1: Simple tree from example
        Map<Integer, List<Integer>> tree1 = new HashMap<>();
        tree1.put(0, Arrays.asList(1, 2, 3));
        tree1.put(1, Arrays.asList(4, 5));
        tree1.put(2, Arrays.asList(6, 7));
        tree1.put(3, Arrays.asList(8, 9));
        System.out.println("Test 1 - Reparent from 6:");
        System.out.println(solution.reparentTree(tree1, 6));
        
        // Test case 2: Single node
        Map<Integer, List<Integer>> tree2 = new HashMap<>();
        tree2.put(0, new ArrayList<>());
        System.out.println("\\nTest 2 - Single node:");
        System.out.println(solution.reparentTree(tree2, 0));
        
        // Test case 3: Linear tree
        Map<Integer, List<Integer>> tree3 = new HashMap<>();
        tree3.put(0, Arrays.asList(1));
        tree3.put(1, Arrays.asList(2));
        tree3.put(2, Arrays.asList(3));
        System.out.println("\\nTest 3 - Linear tree, reparent from 2:");
        System.out.println(solution.reparentTree(tree3, 2));
        
        // Test case 4: Reparent at root (no change)
        Map<Integer, List<Integer>> tree4 = new HashMap<>();
        tree4.put(0, Arrays.asList(1, 2));
        tree4.put(1, Arrays.asList(3));
        System.out.println("\\nTest 4 - Reparent at root:");
        System.out.println(solution.reparentTree(tree4, 0));
        
        // Test case 5: Empty tree
        Map<Integer, List<Integer>> tree5 = new HashMap<>();
        System.out.println("\\nTest 5 - Empty tree:");
        System.out.println(solution.reparentTree(tree5, 0));
    }
}
