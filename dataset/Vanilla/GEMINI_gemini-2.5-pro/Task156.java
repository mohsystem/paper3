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

    // Preorder Traversal: Root -> Left -> Right
    public List<Integer> preorderTraversal(TreeNode root) {
        List<Integer> result = new ArrayList<>();
        preorderHelper(root, result);
        return result;
    }

    private void preorderHelper(TreeNode node, List<Integer> result) {
        if (node == null) {
            return;
        }
        result.add(node.val);
        preorderHelper(node.left, result);
        preorderHelper(node.right, result);
    }

    // Inorder Traversal: Left -> Root -> Right
    public List<Integer> inorderTraversal(TreeNode root) {
        List<Integer> result = new ArrayList<>();
        inorderHelper(root, result);
        return result;
    }

    private void inorderHelper(TreeNode node, List<Integer> result) {
        if (node == null) {
            return;
        }
        inorderHelper(node.left, result);
        result.add(node.val);
        inorderHelper(node.right, result);
    }

    // Postorder Traversal: Left -> Right -> Root
    public List<Integer> postorderTraversal(TreeNode root) {
        List<Integer> result = new ArrayList<>();
        postorderHelper(root, result);
        return result;
    }

    private void postorderHelper(TreeNode node, List<Integer> result) {
        if (node == null) {
            return;
        }
        postorderHelper(node.left, result);
        postorderHelper(node.right, result);
        result.add(node.val);
    }

    public static void main(String[] args) {
        Task156 traversals = new Task156();

        // --- Test Case 1: Standard Tree ---
        System.out.println("--- Test Case 1 ---");
        TreeNode root1 = new TreeNode(1);
        root1.left = new TreeNode(2);
        root1.right = new TreeNode(3);
        root1.left.left = new TreeNode(4);
        root1.left.right = new TreeNode(5);
        System.out.println("Preorder:  " + traversals.preorderTraversal(root1));
        System.out.println("Inorder:   " + traversals.inorderTraversal(root1));
        System.out.println("Postorder: " + traversals.postorderTraversal(root1));
        System.out.println();

        // --- Test Case 2: Null Tree ---
        System.out.println("--- Test Case 2 ---");
        TreeNode root2 = null;
        System.out.println("Preorder:  " + traversals.preorderTraversal(root2));
        System.out.println("Inorder:   " + traversals.inorderTraversal(root2));
        System.out.println("Postorder: " + traversals.postorderTraversal(root2));
        System.out.println();

        // --- Test Case 3: Single Node Tree ---
        System.out.println("--- Test Case 3 ---");
        TreeNode root3 = new TreeNode(10);
        System.out.println("Preorder:  " + traversals.preorderTraversal(root3));
        System.out.println("Inorder:   " + traversals.inorderTraversal(root3));
        System.out.println("Postorder: " + traversals.postorderTraversal(root3));
        System.out.println();
        
        // --- Test Case 4: Left-skewed Tree ---
        System.out.println("--- Test Case 4 ---");
        TreeNode root4 = new TreeNode(4);
        root4.left = new TreeNode(3);
        root4.left.left = new TreeNode(2);
        root4.left.left.left = new TreeNode(1);
        System.out.println("Preorder:  " + traversals.preorderTraversal(root4));
        System.out.println("Inorder:   " + traversals.inorderTraversal(root4));
        System.out.println("Postorder: " + traversals.postorderTraversal(root4));
        System.out.println();

        // --- Test Case 5: Complex Tree ---
        System.out.println("--- Test Case 5 ---");
        TreeNode root5 = new TreeNode(10);
        root5.left = new TreeNode(5);
        root5.right = new TreeNode(15);
        root5.left.left = new TreeNode(3);
        root5.left.right = new TreeNode(7);
        root5.right.right = new TreeNode(18);
        System.out.println("Preorder:  " + traversals.preorderTraversal(root5));
        System.out.println("Inorder:   " + traversals.inorderTraversal(root5));
        System.out.println("Postorder: " + traversals.postorderTraversal(root5));
        System.out.println();
    }
}