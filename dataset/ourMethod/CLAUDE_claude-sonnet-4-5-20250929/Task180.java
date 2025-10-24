
import java.util.*;

public class Task180 {
    static class Tree {
        private final Map<Integer, List<Integer>> adjacencyList;
        
        public Tree(Map<Integer, List<Integer>> adjacencyList) {
            if (adjacencyList == null) {
                throw new IllegalArgumentException("Adjacency list cannot be null");
            }
            this.adjacencyList = new HashMap<>();
            for (Map.Entry<Integer, List<Integer>> entry : adjacencyList.entrySet()) {
                if (entry.getKey() == null || entry.getValue() == null) {
                    throw new IllegalArgumentException("Null keys or values not allowed");
                }
                this.adjacencyList.put(entry.getKey(), new ArrayList<>(entry.getValue()));
            }
        }
        
        public Tree reparent(int newRoot) {
            if (!adjacencyList.containsKey(newRoot)) {
                throw new IllegalArgumentException("Node not in tree");
            }
            
            Map<Integer, List<Integer>> newAdjacencyList = new HashMap<>();
            Set<Integer> visited = new HashSet<>();
            
            buildReparentedTree(newRoot, -1, visited, newAdjacencyList);
            
            return new Tree(newAdjacencyList);
        }
        
        private void buildReparentedTree(int current, int parent, Set<Integer> visited, 
                                        Map<Integer, List<Integer>> newAdjacencyList) {
            if (visited.contains(current)) {
                return;
            }
            visited.add(current);
            
            newAdjacencyList.putIfAbsent(current, new ArrayList<>());
            
            List<Integer> neighbors = adjacencyList.getOrDefault(current, new ArrayList<>());
            for (int neighbor : neighbors) {
                if (neighbor != parent && !visited.contains(neighbor)) {
                    newAdjacencyList.get(current).add(neighbor);
                    buildReparentedTree(neighbor, current, visited, newAdjacencyList);
                }
            }
        }
        
        public Map<Integer, List<Integer>> getAdjacencyList() {
            Map<Integer, List<Integer>> copy = new HashMap<>();
            for (Map.Entry<Integer, List<Integer>> entry : adjacencyList.entrySet()) {
                copy.put(entry.getKey(), new ArrayList<>(entry.getValue()));
            }
            return copy;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple tree reparenting
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
        
        Tree t1 = new Tree(tree1);
        Tree reparented1 = t1.reparent(6);
        System.out.println("Test 1 - Reparent on 6: " + reparented1.getAdjacencyList());
        
        // Test case 2: Single node
        Map<Integer, List<Integer>> tree2 = new HashMap<>();
        tree2.put(0, new ArrayList<>());
        Tree t2 = new Tree(tree2);
        Tree reparented2 = t2.reparent(0);
        System.out.println("Test 2 - Single node: " + reparented2.getAdjacencyList());
        
        // Test case 3: Linear tree
        Map<Integer, List<Integer>> tree3 = new HashMap<>();
        tree3.put(0, Arrays.asList(1));
        tree3.put(1, Arrays.asList(0, 2));
        tree3.put(2, Arrays.asList(1));
        Tree t3 = new Tree(tree3);
        Tree reparented3 = t3.reparent(2);
        System.out.println("Test 3 - Linear tree reparent on 2: " + reparented3.getAdjacencyList());
        
        // Test case 4: Reparent on original root
        Tree reparented4 = t1.reparent(0);
        System.out.println("Test 4 - Reparent on original root: " + reparented4.getAdjacencyList());
        
        // Test case 5: Small tree
        Map<Integer, List<Integer>> tree5 = new HashMap<>();
        tree5.put(0, Arrays.asList(1, 2));
        tree5.put(1, Arrays.asList(0));
        tree5.put(2, Arrays.asList(0));
        Tree t5 = new Tree(tree5);
        Tree reparented5 = t5.reparent(1);
        System.out.println("Test 5 - Small tree reparent on 1: " + reparented5.getAdjacencyList());
    }
}
