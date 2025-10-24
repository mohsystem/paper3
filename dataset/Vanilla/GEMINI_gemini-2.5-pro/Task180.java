import java.util.*;

public class Task180 {

    static class Node {
        int value;
        List<Node> children;

        Node(int value) {
            this.value = value;
            this.children = new ArrayList<>();
        }

        Node addChild(Node child) {
            this.children.add(child);
            return this;
        }
    }

    public static Node reparentTree(Node oldRoot, int newRootValue) {
        if (oldRoot == null) {
            return null;
        }

        Map<Integer, List<Integer>> adj = new HashMap<>();
        Set<Integer> nodes = new HashSet<>();
        buildAdjacencyList(oldRoot, adj, nodes);

        if (!nodes.contains(newRootValue)) {
            // newRootValue is not in the tree
            return null;
        }

        return buildNewTreeFromAdjacencyList(newRootValue, adj);
    }

    private static void buildAdjacencyList(Node node, Map<Integer, List<Integer>> adj, Set<Integer> nodes) {
        if (node == null) {
            return;
        }
        nodes.add(node.value);
        adj.putIfAbsent(node.value, new ArrayList<>());
        for (Node child : node.children) {
            adj.putIfAbsent(child.value, new ArrayList<>());
            adj.get(node.value).add(child.value);
            adj.get(child.value).add(node.value);
            buildAdjacencyList(child, adj, nodes);
        }
    }

    private static Node buildNewTreeFromAdjacencyList(int rootValue, Map<Integer, List<Integer>> adj) {
        Node newRoot = new Node(rootValue);
        Queue<Node> queue = new LinkedList<>();
        queue.add(newRoot);
        Set<Integer> visited = new HashSet<>();
        visited.add(rootValue);

        while (!queue.isEmpty()) {
            Node currentNode = queue.poll();
            if (adj.containsKey(currentNode.value)) {
                // Sort neighbors to have deterministic output for tests
                List<Integer> neighbors = adj.get(currentNode.value);
                Collections.sort(neighbors);
                for (int neighborValue : neighbors) {
                    if (!visited.contains(neighborValue)) {
                        visited.add(neighborValue);
                        Node childNode = new Node(neighborValue);
                        currentNode.children.add(childNode);
                        queue.add(childNode);
                    }
                }
            }
        }
        return newRoot;
    }

    public static void printTree(Node node, String prefix, boolean isTail) {
        if (node == null) return;
        System.out.println(prefix + (isTail ? "└── " : "├── ") + node.value);
        for (int i = 0; i < node.children.size() - 1; i++) {
            printTree(node.children.get(i), prefix + (isTail ? "    " : "│   "), false);
        }
        if (node.children.size() > 0) {
            printTree(node.children.get(node.children.size() - 1), prefix + (isTail ? "    " : "│   "), true);
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Example from prompt
        System.out.println("--- Test Case 1: Re-parenting on node 6 ---");
        Node root1 = new Node(0);
        Node n1 = new Node(1);
        Node n2 = new Node(2);
        Node n3 = new Node(3);
        n1.addChild(new Node(4)).addChild(new Node(5));
        n2.addChild(new Node(6)).addChild(new Node(7));
        n3.addChild(new Node(8)).addChild(new Node(9));
        root1.addChild(n1).addChild(n2).addChild(n3);
        
        System.out.println("Original tree (rooted at 0):");
        printTree(root1, "", true);
        
        Node newRoot1 = reparentTree(root1, 6);
        System.out.println("\nNew tree (re-rooted at 6):");
        printTree(newRoot1, "", true);
        System.out.println("\n" + "=".repeat(40));

        // Test Case 2: Re-rooting at the current root (0)
        System.out.println("--- Test Case 2: Re-parenting on current root 0 ---");
        System.out.println("Original tree (rooted at 0):");
        printTree(root1, "", true);
        
        Node newRoot2 = reparentTree(root1, 0);
        System.out.println("\nNew tree (re-rooted at 0):");
        printTree(newRoot2, "", true);
        System.out.println("\n" + "=".repeat(40));

        // Test Case 3: Re-rooting at a leaf node (9)
        System.out.println("--- Test Case 3: Re-parenting on leaf node 9 ---");
        System.out.println("Original tree (rooted at 0):");
        printTree(root1, "", true);
        
        Node newRoot3 = reparentTree(root1, 9);
        System.out.println("\nNew tree (re-rooted at 9):");
        printTree(newRoot3, "", true);
        System.out.println("\n" + "=".repeat(40));
        
        // Test Case 4: Linear tree
        System.out.println("--- Test Case 4: Linear tree ---");
        Node root4 = new Node(0);
        root4.addChild(new Node(1).addChild(new Node(2).addChild(new Node(3))));
        System.out.println("Original tree (rooted at 0):");
        printTree(root4, "", true);

        Node newRoot4 = reparentTree(root4, 2);
        System.out.println("\nNew tree (re-rooted at 2):");
        printTree(newRoot4, "", true);
        System.out.println("\n" + "=".repeat(40));

        // Test Case 5: Tree with a single node
        System.out.println("--- Test Case 5: Single node tree ---");
        Node root5 = new Node(42);
        System.out.println("Original tree (rooted at 42):");
        printTree(root5, "", true);

        Node newRoot5 = reparentTree(root5, 42);
        System.out.println("\nNew tree (re-rooted at 42):");
        printTree(newRoot5, "", true);
        System.out.println("\n" + "=".repeat(40));
    }
}