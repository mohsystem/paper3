import java.util.*;

public class Task180 {

    /**
     * Reparents a tree on the given newRoot node.
     *
     * @param adjacencyList An adjacency list representing the undirected tree.
     * @param newRoot       The ID of the node to become the new root.
     * @return A map representing the new directed tree (parent -> children),
     *         ordered by a pre-order traversal from the new root.
     */
    public static Map<Integer, List<Integer>> reparentTree(Map<Integer, List<Integer>> adjacencyList, int newRoot) {
        // LinkedHashMap preserves the insertion order, which will be a pre-order traversal.
        Map<Integer, List<Integer>> newTree = new LinkedHashMap<>();
        buildNewTreeDfs(newRoot, -1, adjacencyList, newTree);
        return newTree;
    }

    private static void buildNewTreeDfs(int currentNode, int parentNode,
                                        Map<Integer, List<Integer>> oldAdj,
                                        Map<Integer, List<Integer>> newTree) {
        
        newTree.put(currentNode, new ArrayList<>());

        if (!oldAdj.containsKey(currentNode)) {
            return;
        }

        List<Integer> neighbors = new ArrayList<>(oldAdj.get(currentNode));
        Collections.sort(neighbors); // Sort for deterministic output

        for (int neighbor : neighbors) {
            if (neighbor != parentNode) {
                newTree.get(currentNode).add(neighbor);
                buildNewTreeDfs(neighbor, currentNode, oldAdj, newTree);
            }
        }
    }

    // --- Test Cases ---
    public static void main(String[] args) {
        runTestCase(1, "Example from prompt",
            new int[][]{{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}}, 6);
        runTestCase(2, "Simple line graph",
            new int[][]{{0, 1}, {1, 2}, {2, 3}, {3, 4}}, 2);
        runTestCase(3, "Star graph, reparent on root",
            new int[][]{{0, 1}, {0, 2}, {0, 3}, {0, 4}}, 0);
        runTestCase(4, "Star graph, reparent on a leaf",
            new int[][]{{0, 1}, {0, 2}, {0, 3}, {0, 4}}, 3);
        runTestCase(5, "A more complex tree",
            new int[][]{{1, 2}, {1, 3}, {1, 4}, {2, 5}, {2, 6}, {4, 7}, {4, 8}}, 4);
    }

    private static void runTestCase(int testNum, String description, int[][] edges, int newRoot) {
        System.out.println("--- Test Case " + testNum + ": " + description + " ---");

        Map<Integer, List<Integer>> adjacencyList = new HashMap<>();
        buildAdjList(adjacencyList, edges);
        
        System.out.println("Reparenting on node: " + newRoot);

        Map<Integer, List<Integer>> reparentedTree = reparentTree(adjacencyList, newRoot);
        System.out.println("Reparented Tree (Parent -> Children):");
        printMap(reparentedTree);
        System.out.println();
    }

    private static void buildAdjList(Map<Integer, List<Integer>> adj, int[][] edges) {
        Set<Integer> nodes = new HashSet<>();
        for (int[] edge : edges) {
            nodes.add(edge[0]);
            nodes.add(edge[1]);
        }
        for (int node : nodes) {
            adj.put(node, new ArrayList<>());
        }
        for (int[] edge : edges) {
            adj.get(edge[0]).add(edge[1]);
            adj.get(edge[1]).add(edge[0]);
        }
    }

    private static void printMap(Map<Integer, List<Integer>> map) {
        for (Map.Entry<Integer, List<Integer>> entry : map.entrySet()) {
            System.out.println(entry.getKey() + " -> " + entry.getValue());
        }
    }
}