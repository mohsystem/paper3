class Task151 {

    // Node class for the BST
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

    // Public method to insert a key
    public void insert(int key) {
        root = insertRec(root, key);
    }

    // A recursive helper function to insert a new key in BST
    private Node insertRec(Node root, int key) {
        // If the tree is empty, return a new node
        if (root == null) {
            root = new Node(key);
            return root;
        }

        // Otherwise, recur down the tree
        if (key < root.key) {
            root.left = insertRec(root.left, key);
        } else if (key > root.key) {
            root.right = insertRec(root.right, key);
        }

        // Return the (unchanged) node pointer
        return root;
    }

    // Public method to search for a key
    public boolean search(int key) {
        return searchRec(root, key) != null;
    }
    
    // A recursive helper function to search for a key
    private Node searchRec(Node root, int key) {
        // Base Cases: root is null or key is present at root
        if (root == null || root.key == key) {
            return root;
        }

        // Key is greater than root's key
        if (root.key < key) {
            return searchRec(root.right, key);
        }

        // Key is smaller than root's key
        return searchRec(root.left, key);
    }

    // Public method to delete a key
    public void delete(int key) {
        root = deleteRec(root, key);
    }

    // A recursive helper function to delete a key
    private Node deleteRec(Node root, int key) {
        // Base case: If the tree is empty
        if (root == null) {
            return root;
        }

        // Recur down the tree
        if (key < root.key) {
            root.left = deleteRec(root.left, key);
        } else if (key > root.key) {
            root.right = deleteRec(root.right, key);
        } else { // If key is same as root's key, then this is the node to be deleted
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

    // Helper function to print inorder traversal of the BST
    private void inorderRec(Node root) {
        if (root != null) {
            inorderRec(root.left);
            System.out.print(root.key + " ");
            inorderRec(root.right);
        }
    }

    public void inorder() {
        inorderRec(root);
        System.out.println();
    }

    public static void main(String[] args) {
        // --- Test Case 1: Insertion and Search ---
        System.out.println("--- Test Case 1: Insertion and Search ---");
        Task151 tree = new Task151();
        tree.insert(50);
        tree.insert(30);
        tree.insert(20);
        tree.insert(40);
        tree.insert(70);
        tree.insert(60);
        tree.insert(80);
        System.out.print("Inorder traversal: ");
        tree.inorder();
        System.out.println("Search for 60: " + (tree.search(60) ? "Found" : "Not Found"));
        System.out.println("Search for 90: " + (tree.search(90) ? "Found" : "Not Found"));
        System.out.println();

        // --- Test Case 2: Deleting a leaf node (20) ---
        System.out.println("--- Test Case 2: Deleting a leaf node (20) ---");
        System.out.println("Deleting 20...");
        tree.delete(20);
        System.out.print("Inorder traversal: ");
        tree.inorder();
        System.out.println();
        
        // --- Test Case 3: Deleting a node with one child (30) ---
        System.out.println("--- Test Case 3: Deleting a node with one child (30) ---");
        System.out.println("Deleting 30...");
        tree.delete(30);
        System.out.print("Inorder traversal: ");
        tree.inorder();
        System.out.println();

        // --- Test Case 4: Deleting a node with two children (50) ---
        System.out.println("--- Test Case 4: Deleting a node with two children (50) ---");
        System.out.println("Deleting 50 (root)...");
        tree.delete(50);
        System.out.print("Inorder traversal: ");
        tree.inorder();
        System.out.println();

        // --- Test Case 5: Deleting another node and searching for a deleted key ---
        System.out.println("--- Test Case 5: Deleting another node and checking ---");
        System.out.println("Deleting 70...");
        tree.delete(70);
        System.out.print("Inorder traversal: ");
        tree.inorder();
        System.out.println("Search for 50: " + (tree.search(50) ? "Found" : "Not Found"));
    }
}