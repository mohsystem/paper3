
import java.util.*;

class Task180 {
    static class TreeNode {
        int value;
        List<TreeNode> children;
        TreeNode parent;
        
        TreeNode(int value) {
            this.value = value;
            this.children = new ArrayList<>();
            this.parent = null;
        }
    }
    
    public static TreeNode reparentTree(TreeNode root, int targetValue) {
        if (root == null) {
            return null;
        }
        
        TreeNode targetNode = findNode(root, targetValue);
        if (targetNode == null) {
            return root;
        }
        
        if (targetNode == root) {
            return root;
        }
        
        List<TreeNode> pathToRoot = new ArrayList<>();
        TreeNode current = targetNode;
        while (current != null) {
            pathToRoot.add(current);
            current = current.parent;
        }
        
        for (int i = 0; i < pathToRoot.size() - 1; i++) {
            TreeNode child = pathToRoot.get(i);
            TreeNode parent = pathToRoot.get(i + 1);
            
            parent.children.remove(child);
            child.children.add(parent);
            parent.parent = child;
        }
        
        targetNode.parent = null;
        return targetNode;
    }
    
    private static TreeNode findNode(TreeNode root, int value) {
        if (root == null) {
            return null;
        }
        if (root.value == value) {
            return root;
        }
        for (TreeNode child : root.children) {
            TreeNode found = findNode(child, value);
            if (found != null) {
                return found;
            }
        }
        return null;
    }
    
    private static TreeNode buildTree(int[][] edges) {
        if (edges == null || edges.length == 0) {
            return null;
        }
        
        Map<Integer, TreeNode> nodes = new HashMap<>();
        Set<Integer> children = new HashSet<>();
        
        for (int[] edge : edges) {
            if (edge == null || edge.length != 2) continue;
            
            nodes.putIfAbsent(edge[0], new TreeNode(edge[0]));
            nodes.putIfAbsent(edge[1], new TreeNode(edge[1]));
            
            TreeNode parent = nodes.get(edge[0]);
            TreeNode child = nodes.get(edge[1]);
            
            parent.children.add(child);
            child.parent = parent;
            children.add(edge[1]);
        }
        
        for (Integer key : nodes.keySet()) {
            if (!children.contains(key)) {
                return nodes.get(key);
            }
        }
        
        return nodes.isEmpty() ? null : nodes.values().iterator().next();
    }
    
    private static void printTree(TreeNode root, String prefix, boolean isTail) {
        if (root == null) return;
        System.out.println(prefix + (isTail ? "└── " : "├── ") + root.value);
        for (int i = 0; i < root.children.size(); i++) {
            printTree(root.children.get(i), 
                     prefix + (isTail ? "    " : "│   "), 
                     i == root.children.size() - 1);
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic tree reparenting
        int[][] edges1 = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {2, 7}, {3, 8}, {3, 9}};
        TreeNode root1 = buildTree(edges1);
        System.out.println("Test 1 - Original tree:");
        printTree(root1, "", true);
        TreeNode newRoot1 = reparentTree(root1, 6);
        System.out.println("\\nTest 1 - Reparented on node 6:");
        printTree(newRoot1, "", true);
        
        // Test case 2: Reparent on root (should remain same)
        int[][] edges2 = {{0, 1}, {0, 2}};
        TreeNode root2 = buildTree(edges2);
        System.out.println("\\n\\nTest 2 - Reparent on root:");
        TreeNode newRoot2 = reparentTree(root2, 0);
        printTree(newRoot2, "", true);
        
        // Test case 3: Single node tree
        int[][] edges3 = {};
        TreeNode root3 = new TreeNode(0);
        System.out.println("\\n\\nTest 3 - Single node:");
        TreeNode newRoot3 = reparentTree(root3, 0);
        printTree(newRoot3, "", true);
        
        // Test case 4: Linear tree
        int[][] edges4 = {{0, 1}, {1, 2}, {2, 3}};
        TreeNode root4 = buildTree(edges4);
        System.out.println("\\n\\nTest 4 - Linear tree reparented on leaf:");
        TreeNode newRoot4 = reparentTree(root4, 3);
        printTree(newRoot4, "", true);
        
        // Test case 5: Invalid target node
        int[][] edges5 = {{0, 1}, {0, 2}};
        TreeNode root5 = buildTree(edges5);
        System.out.println("\\n\\nTest 5 - Invalid target (returns original):");
        TreeNode newRoot5 = reparentTree(root5, 99);
        printTree(newRoot5, "", true);
    }
}
