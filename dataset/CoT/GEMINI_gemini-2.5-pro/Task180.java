import java.util.*;
import java.util.stream.Collectors;

public class Task180 {

    /**
     * Reparents a tree on a selected node.
     *
     * @param originalAdj The original undirected tree represented as an adjacency list.
     * @param newRoot     The node to become the new root.
     * @return A new directed tree (adjacency list) rooted at newRoot.
     */
    public static Map<Integer, List<Integer>> reparentTree(Map<Integer, List<Integer>> originalAdj, int newRoot) {
        Map<Integer, List<Integer>> newAdj = new TreeMap<>();
        if (!originalAdj.containsKey(newRoot)) {
            return newAdj; // Or throw an exception
        }

        for (Integer node : originalAdj.keySet()) {
            newAdj.put(node, new ArrayList<>());
        }

        buildNewTree(newRoot, -1, originalAdj, newAdj);
        
        // Sort children for consistent output
        for (List<Integer> children : newAdj.values()) {
            Collections.sort(children);
        }

        return newAdj;
    }

    private static void buildNewTree(int currentNode, int parentNode, Map<Integer, List<Integer>> originalAdj, Map<Integer, List<Integer>> newAdj) {
        if (!originalAdj.containsKey(currentNode)) {
            return;
        }
        for (int neighbor : originalAdj.get(currentNode)) {
            if (neighbor != parentNode) {
                newAdj.get(currentNode).add(neighbor);
                buildNewTree(neighbor, currentNode, originalAdj, newAdj);
            }
        }
    }
    
    // Helper to build an undirected graph from edges
    public static Map<Integer, List<Integer>> buildGraphFromEdges(int[][] edges) {
        Map<Integer, List<Integer>> adj = new TreeMap<>();
        Set<Integer> nodes = new HashSet<>();
        for (int[] edge : edges) {
            nodes.add(edge[0]);
            nodes.add(edge[1]);
        }
        for(int node : nodes) {
            adj.put(node, new ArrayList<>());
        }
        for (int[] edge : edges) {
            adj.get(edge[0]).add(edge[1]);
            adj.get(edge[1]).add(edge[0]);
        }
        return adj;
    }

    public static void main(String[] args) {
        // Test Case 1: Example from prompt
        int[][] edges1 = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}};
        int newRoot1 = 6;
        Map<Integer, List<Integer>> graph1 = buildGraphFromEdges(edges1);
        System.out.println("Test Case 1: Reparent on " + newRoot1);
        System.out.println("Original Tree: " + graph1);
        System.out.println("Reparented Tree: " + reparentTree(graph1, newRoot1));
        System.out.println();

        // Test Case 2: Line graph
        int[][] edges2 = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
        int newRoot2 = 2;
        Map<Integer, List<Integer>> graph2 = buildGraphFromEdges(edges2);
        System.out.println("Test Case 2: Reparent on " + newRoot2);
        System.out.println("Original Tree: " + graph2);
        System.out.println("Reparented Tree: " + reparentTree(graph2, newRoot2));
        System.out.println();

        // Test Case 3: Root is already the 'natural' root
        int newRoot3 = 0;
        System.out.println("Test Case 3: Reparent on " + newRoot3);
        System.out.println("Original Tree: " + graph1);
        System.out.println("Reparented Tree: " + reparentTree(graph1, newRoot3));
        System.out.println();

        // Test Case 4: Leaf node as new root
        int newRoot4 = 9;
        System.out.println("Test Case 4: Reparent on " + newRoot4);
        System.out.println("Original Tree: " + graph1);
        System.out.println("Reparented Tree: " + reparentTree(graph1, newRoot4));
        System.out.println();
        
        // Test Case 5: Small tree
        int[][] edges5 = {{1, 0}, {0, 2}};
        int newRoot5 = 1;
        Map<Integer, List<Integer>> graph5 = buildGraphFromEdges(edges5);
        System.out.println("Test Case 5: Reparent on " + newRoot5);
        System.out.println("Original Tree: " + graph5);
        System.out.println("Reparented Tree: " + reparentTree(graph5, newRoot5));
        System.out.println();
    }
}