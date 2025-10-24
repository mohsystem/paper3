import java.util.*;

public class Task156 {

    public static class TreeNode {
        int val;
        TreeNode left, right;
        TreeNode(int v) { this.val = v; }
        TreeNode(int v, TreeNode l, TreeNode r) { this.val = v; this.left = l; this.right = r; }
    }

    // Public API: performs DFS traversal based on order: "preorder", "inorder", "postorder"
    public static List<Integer> dfs(TreeNode root, String order) {
        if (order == null) throw new IllegalArgumentException("order cannot be null");
        switch (order.toLowerCase()) {
            case "preorder": return preorder(root);
            case "inorder": return inorder(root);
            case "postorder": return postorder(root);
            default: throw new IllegalArgumentException("Unknown order: " + order);
        }
    }

    // Specific traversals
    public static List<Integer> preorder(TreeNode root) {
        List<Integer> res = new ArrayList<>();
        preorderHelper(root, res);
        return res;
    }
    private static void preorderHelper(TreeNode node, List<Integer> res) {
        if (node == null) return;
        res.add(node.val);
        preorderHelper(node.left, res);
        preorderHelper(node.right, res);
    }

    public static List<Integer> inorder(TreeNode root) {
        List<Integer> res = new ArrayList<>();
        inorderHelper(root, res);
        return res;
    }
    private static void inorderHelper(TreeNode node, List<Integer> res) {
        if (node == null) return;
        inorderHelper(node.left, res);
        res.add(node.val);
        inorderHelper(node.right, res);
    }

    public static List<Integer> postorder(TreeNode root) {
        List<Integer> res = new ArrayList<>();
        postorderHelper(root, res);
        return res;
    }
    private static void postorderHelper(TreeNode node, List<Integer> res) {
        if (node == null) return;
        postorderHelper(node.left, res);
        postorderHelper(node.right, res);
        res.add(node.val);
    }

    private static void printList(String label, List<Integer> lst) {
        System.out.println(label + ": " + lst);
    }

    public static void main(String[] args) {
        // Test 1: Balanced tree
        TreeNode root1 = new TreeNode(1,
                new TreeNode(2, new TreeNode(4), new TreeNode(5)),
                new TreeNode(3, new TreeNode(6), new TreeNode(7)));
        printList("Test1 Preorder", dfs(root1, "preorder"));  // [1,2,4,5,3,6,7]

        // Test 2: Single node
        TreeNode root2 = new TreeNode(10);
        printList("Test2 Inorder", dfs(root2, "inorder"));    // [10]

        // Test 3: Left-skewed
        TreeNode root3 = new TreeNode(5);
        root3.left = new TreeNode(4);
        root3.left.left = new TreeNode(3);
        root3.left.left.left = new TreeNode(2);
        root3.left.left.left.left = new TreeNode(1);
        printList("Test3 Postorder", dfs(root3, "postorder")); // [1,2,3,4,5]

        // Test 4: Right-skewed
        TreeNode root4 = new TreeNode(1);
        root4.right = new TreeNode(2);
        root4.right.right = new TreeNode(3);
        root4.right.right.right = new TreeNode(4);
        root4.right.right.right.right = new TreeNode(5);
        printList("Test4 Preorder", dfs(root4, "preorder"));   // [1,2,3,4,5]

        // Test 5: Empty tree
        TreeNode root5 = null;
        printList("Test5 Inorder (empty)", dfs(root5, "inorder")); // []
    }
}