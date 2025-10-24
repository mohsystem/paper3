import java.util.ArrayList;
import java.util.List;

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

    // Preorder traversal: Root -> Left -> Right
    public static void preorderTraversal(TreeNode root, List<Integer> result) {
        if (root == null) {
            return;
        }
        result.add(root.val);
        preorderTraversal(root.left, result);
        preorderTraversal(root.right, result);
    }

    // Inorder traversal: Left -> Root -> Right
    public static void inorderTraversal(TreeNode root, List<Integer> result) {
        if (root == null) {
            return;
        }
        inorderTraversal(root.left, result);
        result.add(root.val);
        inorderTraversal(root.right, result);
    }

    // Postorder traversal: Left -> Right -> Root
    public static void postorderTraversal(TreeNode root, List<Integer> result) {
        if (root == null) {
            return;
        }
        postorderTraversal(root.left, result);
        postorderTraversal(root.right, result);
        result.add(root.val);
    }

    private static void runTestCase(String name, TreeNode root) {
        System.out.println("--- " + name + " ---");
        
        List<Integer> preorderResult = new ArrayList<>();
        preorderTraversal(root, preorderResult);
        System.out.println("Preorder:  " + preorderResult);

        List<Integer> inorderResult = new ArrayList<>();
        inorderTraversal(root, inorderResult);
        System.out.println("Inorder:   " + inorderResult);

        List<Integer> postorderResult = new ArrayList<>();
        postorderTraversal(root, postorderResult);
        System.out.println("Postorder: " + postorderResult);
        System.out.println();
    }

    public static void main(String[] args) {
        // Test Case 1: A balanced binary tree
        TreeNode root1 = new TreeNode(4);
        root1.left = new TreeNode(2);
        root1.right = new TreeNode(7);
        root1.left.left = new TreeNode(1);
        root1.left.right = new TreeNode(3);
        root1.right.left = new TreeNode(6);
        root1.right.right = new TreeNode(9);
        runTestCase("Test Case 1: Balanced Tree", root1);

        // Test Case 2: A left-skewed tree
        TreeNode root2 = new TreeNode(3);
        root2.left = new TreeNode(2);
        root2.left.left = new TreeNode(1);
        runTestCase("Test Case 2: Left-Skewed Tree", root2);

        // Test Case 3: A right-skewed tree
        TreeNode root3 = new TreeNode(1);
        root3.right = new TreeNode(2);
        root3.right.right = new TreeNode(3);
        runTestCase("Test Case 3: Right-Skewed Tree", root3);
        
        // Test Case 4: A single node tree
        TreeNode root4 = new TreeNode(5);
        runTestCase("Test Case 4: Single Node Tree", root4);

        // Test Case 5: An empty tree
        TreeNode root5 = null;
        runTestCase("Test Case 5: Empty Tree", root5);
    }
}