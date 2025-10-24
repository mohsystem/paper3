
import java.util.*;

class Task156 {
    static class TreeNode {
        int val;
        TreeNode left;
        TreeNode right;
        
        TreeNode(int val) {
            if (val < Integer.MIN_VALUE || val > Integer.MAX_VALUE) {
                throw new IllegalArgumentException("Invalid node value");
            }
            this.val = val;
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
        result.add(node.val);
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
        result.add(node.val);
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
        result.add(node.val);
    }
    
    public static void main(String[] args) {
        // Test Case 1: Single node tree
        TreeNode test1 = new TreeNode(1);
        System.out.println("Test 1 - Preorder: " + preorderTraversal(test1));
        System.out.println("Test 1 - Inorder: " + inorderTraversal(test1));
        System.out.println("Test 1 - Postorder: " + postorderTraversal(test1));
        
        // Test Case 2: Balanced tree
        TreeNode test2 = new TreeNode(1);
        test2.left = new TreeNode(2);
        test2.right = new TreeNode(3);
        test2.left.left = new TreeNode(4);
        test2.left.right = new TreeNode(5);
        System.out.println("\\nTest 2 - Preorder: " + preorderTraversal(test2));
        System.out.println("Test 2 - Inorder: " + inorderTraversal(test2));
        System.out.println("Test 2 - Postorder: " + postorderTraversal(test2));
        
        // Test Case 3: Left-skewed tree
        TreeNode test3 = new TreeNode(1);
        test3.left = new TreeNode(2);
        test3.left.left = new TreeNode(3);
        System.out.println("\\nTest 3 - Preorder: " + preorderTraversal(test3));
        System.out.println("Test 3 - Inorder: " + inorderTraversal(test3));
        System.out.println("Test 3 - Postorder: " + postorderTraversal(test3));
        
        // Test Case 4: Right-skewed tree
        TreeNode test4 = new TreeNode(1);
        test4.right = new TreeNode(2);
        test4.right.right = new TreeNode(3);
        System.out.println("\\nTest 4 - Preorder: " + preorderTraversal(test4));
        System.out.println("Test 4 - Inorder: " + inorderTraversal(test4));
        System.out.println("Test 4 - Postorder: " + postorderTraversal(test4));
        
        // Test Case 5: Empty tree
        TreeNode test5 = null;
        System.out.println("\\nTest 5 - Preorder: " + preorderTraversal(test5));
        System.out.println("Test 5 - Inorder: " + inorderTraversal(test5));
        System.out.println("Test 5 - Postorder: " + postorderTraversal(test5));
    }
}
