class Task151 {

    // Node class representing a single node in the BST
    static class Node {
        int key;
        Node left, right;

        public Node(int item) {
            key = item;
            left = right = null;
        }
    }

    // Root of the BST
    private Node root;

    // Constructor
    public Task151() {
        root = null;
    }

    // Public method to insert a new key
    public void insert(int key) {
        root = insertRec(root, key);
    }

    /* A recursive function to insert a new key in BST */
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

    // Public method to delete a key
    public void delete(int key) {
        root = deleteRec(root, key);
    }

    /* A recursive function to delete a key in BST */
    private Node deleteRec(Node root, int key) {
        // Base Case: If the tree is empty
        if (root == null) {
            return root;
        }

        // Recur down the tree
        if (key < root.key) {
            root.left = deleteRec(root.left, key);
        } else if (key > root.key) {
            root.right = deleteRec(root.right, key);
        }
        // If key is same as root's key, then this is the node to be deleted
        else {
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

    // Public method to search for a key
    public boolean search(int key) {
        return searchRec(root, key) != null;
    }

    /* A recursive function to search for a key in BST */
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
    
    // Helper function to print inorder traversal of the BST
    public void inorder() {
        inorderRec(root);
        System.out.println();
    }

    private void inorderRec(Node root) {
        if (root != null) {
            inorderRec(root.left);
            System.out.print(root.key + " ");
            inorderRec(root.right);
        }
    }

    public static void main(String[] args) {
        Task151 tree = new Task151();

        /*
         *        50
         *       /  \
         *      30   70
         *     / \   / \
         *    20 40 60  80
         */
        tree.insert(50);
        tree.insert(30);
        tree.insert(20);
        tree.insert(40);
        tree.insert(70);
        tree.insert(60);
        tree.insert(80);
        
        System.out.println("Initial tree (inorder):");
        tree.inorder();

        // Test Case 1: Search for an element that exists
        System.out.println("Test Case 1: Search for 40");
        System.out.println("Found 40: " + tree.search(40));

        // Test Case 2: Search for an element that does not exist
        System.out.println("\nTest Case 2: Search for 90");
        System.out.println("Found 90: " + tree.search(90));
        
        // Test Case 3: Delete a leaf node (20)
        System.out.println("\nTest Case 3: Delete 20 (leaf node)");
        tree.delete(20);
        System.out.print("Tree after deleting 20: ");
        tree.inorder();

        // Test Case 4: Delete a node with one child (30)
        System.out.println("\nTest Case 4: Delete 30 (node with one child)");
        tree.delete(30);
        System.out.print("Tree after deleting 30: ");
        tree.inorder();
        
        // Test Case 5: Delete a node with two children (50 - the root)
        System.out.println("\nTest Case 5: Delete 50 (node with two children - root)");
        tree.delete(50);
        System.out.print("Tree after deleting 50: ");
        tree.inorder();
    }
}