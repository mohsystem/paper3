public class Task151 {

    // Node class for the BST
    static class Node {
        int key;
        Node left, right;

        public Node(int item) {
            key = item;
            left = right = null;
        }
    }

    // Binary Search Tree class
    static class BinarySearchTree {
        Node root;

        BinarySearchTree() {
            root = null;
        }

        /**
         * Public method to insert a new key.
         * @param key The key to insert.
         */
        public void insert(int key) {
            root = insertRec(root, key);
        }

        /**
         * A recursive function to insert a new key in BST.
         * @param root The current node in the recursion.
         * @param key The key to insert.
         * @return The node pointer (possibly new).
         */
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

        /**
         * Public method to delete a key.
         * @param key The key to delete.
         */
        public void deleteNode(int key) {
            root = deleteRec(root, key);
        }

        /**
         * A recursive function to delete a key from BST.
         * @param root The current node in the recursion.
         * @param key The key to delete.
         * @return The new root of the subtree.
         */
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

        /**
         * Helper function to find the minimum value in a subtree.
         * @param root The root of the subtree.
         * @return The minimum value.
         */
        private int minValue(Node root) {
            int minv = root.key;
            while (root.left != null) {
                minv = root.left.key;
                root = root.left;
            }
            return minv;
        }

        /**
         * Public method to search for a key.
         * @param key The key to search for.
         * @return true if the key is found, false otherwise.
         */
        public boolean search(int key) {
            return searchRec(root, key) != null;
        }

        /**
         * A recursive function to search for a key in BST.
         * @param root The current node in the recursion.
         * @param key The key to search for.
         * @return The node containing the key, or null.
         */
        private Node searchRec(Node root, int key) {
            if (root == null || root.key == key) {
                return root;
            }
            if (key < root.key) {
                return searchRec(root.left, key);
            }
            return searchRec(root.right, key);
        }
    }

    public static void main(String[] args) {
        BinarySearchTree bst = new BinarySearchTree();

        // Test Case 1: Insert and search
        System.out.println("Test Case 1: Insert and Search");
        bst.insert(50);
        bst.insert(30);
        bst.insert(70);
        System.out.println("Search for 30: " + bst.search(30)); // Expected: true
        System.out.println("Search for 100: " + bst.search(100)); // Expected: false
        System.out.println("---");

        // Test Case 2: Delete a leaf node
        System.out.println("Test Case 2: Delete a leaf node");
        bst.insert(20);
        System.out.println("Search for 20 before delete: " + bst.search(20)); // Expected: true
        bst.deleteNode(20);
        System.out.println("Search for 20 after delete: " + bst.search(20)); // Expected: false
        System.out.println("---");

        // Test Case 3: Delete a node with one child
        System.out.println("Test Case 3: Delete a node with one child");
        bst.insert(40);
        System.out.println("Search for 30 before delete: " + bst.search(30)); // Expected: true
        System.out.println("Search for 40 before delete: " + bst.search(40)); // Expected: true
        bst.deleteNode(30);
        System.out.println("Search for 30 after delete: " + bst.search(30)); // Expected: false
        System.out.println("Search for 40 after delete: " + bst.search(40)); // Expected: true
        System.out.println("---");

        // Test Case 4: Delete a node with two children
        System.out.println("Test Case 4: Delete a node with two children");
        bst.insert(60);
        bst.insert(80);
        System.out.println("Search for 70 before delete: " + bst.search(70)); // Expected: true
        bst.deleteNode(70);
        System.out.println("Search for 70 after delete: " + bst.search(70)); // Expected: false
        System.out.println("Search for 80 (new subtree root): " + bst.search(80)); // Expected: true
        System.out.println("---");
        
        // Test Case 5: Delete root node
        System.out.println("Test Case 5: Delete root node");
        System.out.println("Search for 50 before delete: " + bst.search(50)); // Expected: true
        bst.deleteNode(50);
        System.out.println("Search for 50 after delete: " + bst.search(50)); // Expected: false
        // The new root should be the inorder successor of 50, which was 60.
        System.out.println("Search for 60 (new root): " + bst.search(60)); // Expected: true
        System.out.println("---");
    }
}