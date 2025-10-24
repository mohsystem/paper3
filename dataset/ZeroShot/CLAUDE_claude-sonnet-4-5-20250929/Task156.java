
import java.util.*;

class TreeNode {
    int val;
    TreeNode left;
    TreeNode right;
    
    TreeNode(int val) {
        this.val = val;
        this.left = null;
        this.right = null;
    }
}

public class Task156 {
    
    public static List<Integer> preorderTraversal(TreeNode root) {
        List<Integer> result = new ArrayList<>();
        preorderHelper(root, result);
        return result;
    }
    
    private static void preorderHelper(TreeNode node, List<Integer> result) {
        if (node == null) return;
        result.add(node.val);
        preorderHelper(node.left, result);
        preorderHelper(node.right, result);
    }
    
    public static List<Integer> inorderTraversal(TreeNode root) {
        List<Integer> result = new ArrayList<>();
        inorderHelper(root, result);
        return result;
    }
    
    private static void inorderHelper(TreeNode node, List<Integer> result) {
        if (node == null) return;
        inorderHelper(node.left, result);
        result.add(node.val);
        inorderHelper(node.right, result);
    }
    
    public static List<Integer> postorderTraversal(TreeNode root) {
        List<Integer> result = new ArrayList<>();
        postorderHelper(root, result);
        return result;
    }
    
    private static void postorderHelper(TreeNode node, List<Integer> result) {
        if (node == null) return;
        postorderHelper(node.left, result);
        postorderHelper(node.right, result);
        result.add(node.val);
    }
    
    public static void main(String[] args) {
        // Test Case 1: Simple tree
        TreeNode tree1 = new TreeNode(1);
        tree1.right = new TreeNode(2);
        tree1.right.left = new TreeNode(3);
        System.out.println("Test Case 1:");
        System.out.println("Preorder: " + preorderTraversal(tree1));
        System.out.println("Inorder: " + inorderTraversal(tree1));
        System.out.println("Postorder: " + postorderTraversal(tree1));
        
        // Test Case 2: Balanced tree
        TreeNode tree2 = new TreeNode(1);
        tree2.left = new TreeNode(2);
        tree2.right = new TreeNode(3);
        tree2.left.left = new TreeNode(4);
        tree2.left.right = new TreeNode(5);
        System.out.println("\\nTest Case 2:");
        System.out.println("Preorder: " + preorderTraversal(tree2));
        System.out.println("Inorder: " + inorderTraversal(tree2));
        System.out.println("Postorder: " + postorderTraversal(tree2));
        
        // Test Case 3: Single node
        TreeNode tree3 = new TreeNode(42);
        System.out.println("\\nTest Case 3:");
        System.out.println("Preorder: " + preorderTraversal(tree3));
        System.out.println("Inorder: " + inorderTraversal(tree3));
        System.out.println("Postorder: " + postorderTraversal(tree3));
        
        // Test Case 4: Empty tree
        TreeNode tree4 = null;
        System.out.println("\\nTest Case 4:");
        System.out.println("Preorder: " + preorderTraversal(tree4));
        System.out.println("Inorder: " + inorderTraversal(tree4));
        System.out.println("Postorder: " + postorderTraversal(tree4));
        
        // Test Case 5: Left skewed tree
        TreeNode tree5 = new TreeNode(5);
        tree5.left = new TreeNode(4);
        tree5.left.left = new TreeNode(3);
        tree5.left.left.left = new TreeNode(2);
        tree5.left.left.left.left = new TreeNode(1);
        System.out.println("\\nTest Case 5:");
        System.out.println("Preorder: " + preorderTraversal(tree5));
        System.out.println("Inorder: " + inorderTraversal(tree5));
        System.out.println("Postorder: " + postorderTraversal(tree5));
    }
}
