
import java.util.*;

public class Task156 {
    static class TreeNode {
        int value;
        TreeNode left;
        TreeNode right;
        
        TreeNode(int value) {
            this.value = value;
            this.left = null;
            this.right = null;
        }
    }
    
    public static List<Integer> preorderTraversal(TreeNode root) {
        List<Integer> result = new ArrayList<>();
        if (root == null) {
            return result;
        }
        preorderHelper(root, result);
        return result;
    }
    
    private static void preorderHelper(TreeNode node, List<Integer> result) {
        if (node == null) {
            return;
        }
        result.add(node.value);
        preorderHelper(node.left, result);
        preorderHelper(node.right, result);
    }
    
    public static List<Integer> inorderTraversal(TreeNode root) {
        List<Integer> result = new ArrayList<>();
        if (root == null) {
            return result;
        }
        inorderHelper(root, result);
        return result;
    }
    
    private static void inorderHelper(TreeNode node, List<Integer> result) {
        if (node == null) {
            return;
        }
        inorderHelper(node.left, result);
        result.add(node.value);
        inorderHelper(node.right, result);
    }
    
    public static List<Integer> postorderTraversal(TreeNode root) {
        List<Integer> result = new ArrayList<>();
        if (root == null) {
            return result;
        }
        postorderHelper(root, result);
        return result;
    }
    
    private static void postorderHelper(TreeNode node, List<Integer> result) {
        if (node == null) {
            return;
        }
        postorderHelper(node.left, result);
        postorderHelper(node.right, result);
        result.add(node.value);
    }
    
    public static void main(String[] args) {
        // Test case 1: Single node tree
        TreeNode tree1 = new TreeNode(1);
        System.out.println("Test 1 - Preorder: " + preorderTraversal(tree1));
        System.out.println("Test 1 - Inorder: " + inorderTraversal(tree1));
        System.out.println("Test 1 - Postorder: " + postorderTraversal(tree1));
        
        // Test case 2: Empty tree
        TreeNode tree2 = null;
        System.out.println("Test 2 - Preorder: " + preorderTraversal(tree2));
        System.out.println("Test 2 - Inorder: " + inorderTraversal(tree2));
        System.out.println("Test 2 - Postorder: " + postorderTraversal(tree2));
        
        // Test case 3: Left-skewed tree
        TreeNode tree3 = new TreeNode(1);
        tree3.left = new TreeNode(2);
        tree3.left.left = new TreeNode(3);
        System.out.println("Test 3 - Preorder: " + preorderTraversal(tree3));
        System.out.println("Test 3 - Inorder: " + inorderTraversal(tree3));
        System.out.println("Test 3 - Postorder: " + postorderTraversal(tree3));
        
        // Test case 4: Right-skewed tree
        TreeNode tree4 = new TreeNode(1);
        tree4.right = new TreeNode(2);
        tree4.right.right = new TreeNode(3);
        System.out.println("Test 4 - Preorder: " + preorderTraversal(tree4));
        System.out.println("Test 4 - Inorder: " + inorderTraversal(tree4));
        System.out.println("Test 4 - Postorder: " + postorderTraversal(tree4));
        
        // Test case 5: Balanced tree
        TreeNode tree5 = new TreeNode(1);
        tree5.left = new TreeNode(2);
        tree5.right = new TreeNode(3);
        tree5.left.left = new TreeNode(4);
        tree5.left.right = new TreeNode(5);
        tree5.right.left = new TreeNode(6);
        tree5.right.right = new TreeNode(7);
        System.out.println("Test 5 - Preorder: " + preorderTraversal(tree5));
        System.out.println("Test 5 - Inorder: " + inorderTraversal(tree5));
        System.out.println("Test 5 - Postorder: " + postorderTraversal(tree5));
    }
}
