import java.util.Objects;

public class Task156 {

    // Node class for the binary tree
    static class Node {
        int data;
        Node left;
        Node right;

        Node(int data) {
            this.data = data;
            this.left = null;
            this.right = null;
        }
    }

    /**
     * Performs a preorder traversal of the binary tree.
     * Root -> Left -> Right
     * @param node The root node of the tree/subtree.
     */
    public static void preorderTraversal(Node node) {
        if (node == null) {
            return;
        }
        System.out.print(node.data + " ");
        preorderTraversal(node.left);
        preorderTraversal(node.right);
    }

    /**
     * Performs an inorder traversal of the binary tree.
     * Left -> Root -> Right
     * @param node The root node of the tree/subtree.
     */
    public static void inorderTraversal(Node node) {
        if (node == null) {
            return;
        }
        inorderTraversal(node.left);
        System.out.print(node.data + " ");
        inorderTraversal(node.right);
    }

    /**
     * Performs a postorder traversal of the binary tree.
     * Left -> Right -> Root
     * @param node The root node of the tree/subtree.
     */
    public static void postorderTraversal(Node node) {
        if (node == null) {
            return;
        }
        postorderTraversal(node.left);
        postorderTraversal(node.right);
        System.out.print(node.data + " ");
    }

    public static void main(String[] args) {
        // Test Case 1: A complete binary tree
        System.out.println("--- Test Case 1: Complete Binary Tree ---");
        Node root1 = new Node(1);
        root1.left = new Node(2);
        root1.right = new Node(3);
        root1.left.left = new Node(4);
        root1.left.right = new Node(5);
        root1.right.left = new Node(6);
        root1.right.right = new Node(7);
        
        System.out.print("Preorder:  ");
        preorderTraversal(root1);
        System.out.println();
        System.out.print("Inorder:   ");
        inorderTraversal(root1);
        System.out.println();
        System.out.print("Postorder: ");
        postorderTraversal(root1);
        System.out.println("\n");

        // Test Case 2: A right-skewed tree
        System.out.println("--- Test Case 2: Right-Skewed Tree ---");
        Node root2 = new Node(10);
        root2.right = new Node(20);
        root2.right.right = new Node(30);
        root2.right.right.right = new Node(40);

        System.out.print("Preorder:  ");
        preorderTraversal(root2);
        System.out.println();
        System.out.print("Inorder:   ");
        inorderTraversal(root2);
        System.out.println();
        System.out.print("Postorder: ");
        postorderTraversal(root2);
        System.out.println("\n");

        // Test Case 3: A left-skewed tree
        System.out.println("--- Test Case 3: Left-Skewed Tree ---");
        Node root3 = new Node(50);
        root3.left = new Node(40);
        root3.left.left = new Node(30);
        root3.left.left.left = new Node(20);

        System.out.print("Preorder:  ");
        preorderTraversal(root3);
        System.out.println();
        System.out.print("Inorder:   ");
        inorderTraversal(root3);
        System.out.println();
        System.out.print("Postorder: ");
        postorderTraversal(root3);
        System.out.println("\n");
        
        // Test Case 4: An empty tree (null root)
        System.out.println("--- Test Case 4: Empty Tree ---");
        Node root4 = null;
        
        System.out.print("Preorder:  ");
        preorderTraversal(root4);
        System.out.println("(empty)");
        System.out.print("Inorder:   ");
        inorderTraversal(root4);
        System.out.println("(empty)");
        System.out.print("Postorder: ");
        postorderTraversal(root4);
        System.out.println("(empty)\n");

        // Test Case 5: A tree with only one node
        System.out.println("--- Test Case 5: Single Node Tree ---");
        Node root5 = new Node(100);

        System.out.print("Preorder:  ");
        preorderTraversal(root5);
        System.out.println();
        System.out.print("Inorder:   ");
        inorderTraversal(root5);
        System.out.println();
        System.out.print("Postorder: ");
        postorderTraversal(root5);
        System.out.println("\n");
    }
}