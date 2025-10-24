import java.util.*;

class Node {
    int id;
    List<Node> children;

    public Node(int id) {
        this.id = id;
        this.children = new ArrayList<>();
    }

    public void addChild(Node child) {
        this.children.add(child);
    }

    public void sortChildren() {
        if (children != null) {
            children.sort(Comparator.comparingInt(c -> c.id));
            for (Node child : children) {
                child.sortChildren();
            }
        }
    }
}

public class Task180 {

    private static Node buildTreeRecursive(int currentNodeId, int parentId, Map<Integer, List<Integer>> adj) {
        Node node = new Node(currentNodeId);
        if (adj.containsKey(currentNodeId)) {
            // Sort neighbors for deterministic output
            List<Integer> neighbors = adj.get(currentNodeId);
            Collections.sort(neighbors);
            for (int neighborId : neighbors) {
                if (neighborId != parentId) {
                    node.addChild(buildTreeRecursive(neighborId, currentNodeId, adj));
                }
            }
        }
        return node;
    }

    public static Node reparentTree(int[][] edges, int newRoot) {
        if (edges == null) {
            return null;
        }

        if (edges.length == 0) {
            return new Node(newRoot);
        }

        Map<Integer, List<Integer>> adj = new HashMap<>();
        Set<Integer> nodes = new HashSet<>();
        for (int[] edge : edges) {
            if (edge.length != 2) continue; // Invalid edge format
            adj.computeIfAbsent(edge[0], k -> new ArrayList<>()).add(edge[1]);
            adj.computeIfAbsent(edge[1], k -> new ArrayList<>()).add(edge[0]);
            nodes.add(edge[0]);
            nodes.add(edge[1]);
        }

        if (!nodes.contains(newRoot)) {
            return null; // New root is not in the tree
        }

        Node newTreeRoot = buildTreeRecursive(newRoot, -1, adj); // Use -1 as a virtual parent
        return newTreeRoot;
    }

    public static void printTree(Node node, String prefix) {
        if (node == null) {
            System.out.println("Tree is null.");
            return;
        }
        System.out.println(prefix + node.id);
        for (Node child : node.children) {
            printTree(child, prefix + "  ");
        }
    }

    public static void main(String[] args) {
        int[][][] edgesList = {
            // Test Case 1: From prompt
            {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}},
            // Test Case 2: Reparent on original root
            {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}},
            // Test Case 3: Reparent on a leaf
            {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}},
            // Test Case 4: Smaller tree
            {{1, 0}, {2, 0}, {3, 1}, {4, 1}},
            // Test Case 5: Line graph
            {{0, 1}, {1, 2}, {2, 3}}
        };
        int[] newRoots = {6, 0, 9, 1, 2};

        for (int i = 0; i < edgesList.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Reparenting on node " + newRoots[i]);
            Node newTree = reparentTree(edgesList[i], newRoots[i]);
            printTree(newTree, "");
            System.out.println();
        }
    }
}