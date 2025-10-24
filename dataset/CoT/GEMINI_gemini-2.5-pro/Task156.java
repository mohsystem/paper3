class Task156 {
    
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

    // Preorder traversal: Root -> Left -> Right
    public static void preorderTraversal(Node node) {
        if (node == null) {
            return;
        }
        System.out.print(node.data + " ");
        preorderTraversal(node.left);
        preorderTraversal(node.right);
    }

    // Inorder traversal: Left -> Root -> Right
    public static void inorderTraversal(Node node) {
        if (node == null) {
            return;
        }
        inorderTraversal(node.left);
        System.out.print(node.data + " ");
        inorderTraversal(node.right);
    }

    // Postorder traversal: Left -> Right -> Root
    public static void postorderTraversal(Node node) {
        if (node == null) {
            return;
        }
        postorderTraversal(node.left);
        postorderTraversal(node.right);
        System.out.print(node.data + " ");
    }
    
    public static void main(String[] args) {
        // --- Test Case 1: A standard binary tree ---
        System.out.println("--- Test Case 1: Standard Tree ---");
        Node root1 = new Node(4);
        root1.left = new Node(2);
        root1.right = new Node(5);
        root1.left.left = new Node(1);
        root1.left.right = new Node(3);

        System.out.print("Preorder: ");
        preorderTraversal(root1);
        System.out.println();

        System.out.print("Inorder:  ");
        inorderTraversal(root1);
        System.out.println();

        System.out.print("Postorder:");
        postorderTraversal(root1);
        System.out.println("\n");

        // --- Test Case 2: A right-skewed tree ---
        System.out.println("--- Test Case 2: Right-Skewed Tree ---");
        Node root2 = new Node(1);
        root2.right = new Node(2);
        root2.right.right = new Node(3);

        System.out.print("Preorder: ");
        preorderTraversal(root2);
        System.out.println();

        System.out.print("Inorder:  ");
        inorderTraversal(root2);
        System.out.println();

        System.out.print("Postorder:");
        postorderTraversal(root2);
        System.out.println("\n");

        // --- Test Case 3: A left-skewed tree ---
        System.out.println("--- Test Case 3: Left-Skewed Tree ---");
        Node root3 = new Node(3);
        root3.left = new Node(2);
        root3.left.left = new Node(1);

        System.out.print("Preorder: ");
        preorderTraversal(root3);
        System.out.println();

        System.out.print("Inorder:  ");
        inorderTraversal(root3);
        System.out.println();

        System.out.print("Postorder:");
        postorderTraversal(root3);
        System.out.println("\n");
        
        // --- Test Case 4: A tree with only a root node ---
        System.out.println("--- Test Case 4: Single Node Tree ---");
        Node root4 = new Node(10);
        
        System.out.print("Preorder: ");
        preorderTraversal(root4);
        System.out.println();

        System.out.print("Inorder:  ");
        inorderTraversal(root4);
        System.out.println();

        System.out.print("Postorder:");
        postorderTraversal(root4);
        System.out.println("\n");

        // --- Test Case 5: An empty tree (null root) ---
        System.out.println("--- Test Case 5: Empty Tree ---");
        Node root5 = null;
        
        System.out.print("Preorder: ");
        preorderTraversal(root5);
        System.out.println();

        System.out.print("Inorder:  ");
        inorderTraversal(root5);
        System.out.println();

        System.out.print("Postorder:");
        postorderTraversal(root5);
        System.out.println();
    }
}