
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
        TreeNode root1 = new TreeNode(1);
        root1.left = new TreeNode(2);
        root1.right = new TreeNode(3);
        System.out.println("Test Case 1:");
        System.out.println("Preorder: " + preorderTraversal(root1));
        System.out.println("Inorder: " + inorderTraversal(root1));
        System.out.println("Postorder: " + postorderTraversal(root1));
        System.out.println();
        
        // Test Case 2: Left-skewed tree
        TreeNode root2 = new TreeNode(1);
        root2.left = new TreeNode(2);
        root2.left.left = new TreeNode(3);
        System.out.println("Test Case 2:");
        System.out.println("Preorder: " + preorderTraversal(root2));
        System.out.println("Inorder: " + inorderTraversal(root2));
        System.out.println("Postorder: " + postorderTraversal(root2));
        System.out.println();
        
        // Test Case 3: Right-skewed tree
        TreeNode root3 = new TreeNode(1);
        root3.right = new TreeNode(2);
        root3.right.right = new TreeNode(3);
        System.out.println("Test Case 3:");
        System.out.println("Preorder: " + preorderTraversal(root3));
        System.out.println("Inorder: " + inorderTraversal(root3));
        System.out.println("Postorder: " + postorderTraversal(root3));
        System.out.println();
        
        // Test Case 4: Complete binary tree
        TreeNode root4 = new TreeNode(1);
        root4.left = new TreeNode(2);
        root4.right = new TreeNode(3);
        root4.left.left = new TreeNode(4);
        root4.left.right = new TreeNode(5);
        root4.right.left = new TreeNode(6);
        root4.right.right = new TreeNode(7);
        System.out.println("Test Case 4:");
        System.out.println("Preorder: " + preorderTraversal(root4));
        System.out.println("Inorder: " + inorderTraversal(root4));
        System.out.println("Postorder: " + postorderTraversal(root4));
        System.out.println();
        
        // Test Case 5: Empty tree
        TreeNode root5 = null;
        System.out.println("Test Case 5:");
        System.out.println("Preorder: " + preorderTraversal(root5));
        System.out.println("Inorder: " + inorderTraversal(root5));
        System.out.println("Postorder: " + postorderTraversal(root5));
    }
}
