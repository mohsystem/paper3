public class Task151 {

    // Node class representing a node in the BST
    static class Node {
        int key;
        Node left, right;

        public Node(int item) {
            key = item;
            left = right = null;
        }
    }

    // Root of the BST
    Node root;

    // Constructor
    public Task151() {
        root = null;
    }

    // Public search method
    public boolean search(int key) {
        return searchRec(root, key) != null;
    }

    // A recursive search function
    private Node searchRec(Node root, int key) {
        if (root == null || root.key == key) {
            return root;
        }
        if (root.key > key) {
            return searchRec(root.left, key);
        }
        return searchRec(root.right, key);
    }

    // Public insert method
    public void insert(int key) {
        root = insertRec(root, key);
    }

    // A recursive function to insert a new key in BST
    private Node insertRec(Node root, int key) {
        if (root == null) {
            root = new Node(key);
            return root;
        }
        if (key < root.key) {
            root.left = insertRec(root.left, key);
        } else if (key > root.key) {
            root.right = insertRec(root.right, key);
        }
        return root;
    }

    // Public delete method
    public void delete(int key) {
        root = deleteRec(root, key);
    }

    // A recursive function to delete a key in BST
    private Node deleteRec(Node root, int key) {
        if (root == null) {
            return root;
        }
        if (key < root.key) {
            root.left = deleteRec(root.left, key);
        } else if (key > root.key) {
            root.right = deleteRec(root.right, key);
        } else {
            // Node with only one child or no child
            if (root.left == null) {
                return root.right;
            } else if (root.right == null) {
                return root.left;
            }

            // Node with two children: Get the inorder successor (smallest in the right subtree)
            root.key = minValue(root.right);

            // Delete the inorder successor
            root.right = deleteRec(root.right, root.key);
        }
        return root;
    }

    // Helper function to find the minimum value in a subtree
    private int minValue(Node root) {
        int minv = root.key;
        while (root.left != null) {
            minv = root.left.key;
            root = root.left;
        }
        return minv;
    }

    // Public method for inorder traversal
    public void inorder() {
        inorderRec(root);
        System.out.println();
    }

    // A utility function to do inorder traversal of BST
    private void inorderRec(Node root) {
        if (root != null) {
            inorderRec(root.left);
            System.out.print(root.key + " ");
            inorderRec(root.right);
        }
    }

    public static void main(String[] args) {
        Task151 bst = new Task151();

        // Test Case 1: Insertion
        System.out.println("--- Test Case 1: Insertion ---");
        int[] keysToInsert = {50, 30, 20, 40, 70, 60, 80};
        for (int key : keysToInsert) {
            bst.insert(key);
        }
        System.out.print("Inorder traversal of the initial BST: ");
        bst.inorder(); // Expected output: 20 30 40 50 60 70 80

        // Test Case 2: Search
        System.out.println("\n--- Test Case 2: Search ---");
        System.out.println("Search for 60: " + (bst.search(60) ? "Found" : "Not Found")); // Expected: Found
        System.out.println("Search for 90: " + (bst.search(90) ? "Found" : "Not Found")); // Expected: Not Found

        // Test Case 3: Delete a leaf node
        System.out.println("\n--- Test Case 3: Delete a leaf node (20) ---");
        bst.delete(20);
        System.out.print("Inorder traversal after deleting 20: ");
        bst.inorder(); // Expected output: 30 40 50 60 70 80

        // Test Case 4: Delete a node with one child
        System.out.println("\n--- Test Case 4: Delete a node with one child (30) ---");
        bst.delete(30);
        System.out.print("Inorder traversal after deleting 30: ");
        bst.inorder(); // Expected output: 40 50 60 70 80

        // Test Case 5: Delete a node with two children
        System.out.println("\n--- Test Case 5: Delete a node with two children (50) ---");
        bst.delete(50);
        System.out.print("Inorder traversal after deleting 50: ");
        bst.inorder(); // Expected output: 40 60 70 80
    }
}