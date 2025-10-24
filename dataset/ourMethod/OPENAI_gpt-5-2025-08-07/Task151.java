import java.util.ArrayList;
import java.util.List;

public class Task151 {
    // Binary Search Tree implementation with insert, delete, and search operations
    static final class BST {
        private static final class Node {
            final int key;
            Node left;
            Node right;
            Node(int key) {
                this.key = key;
            }
        }

        private Node root;

        public BST() {
            this.root = null;
        }

        public boolean insert(int key) {
            if (!isIntSafe(key)) {
                return false;
            }
            if (root == null) {
                root = new Node(key);
                return true;
            }
            Node curr = root;
            Node parent = null;
            while (curr != null) {
                if (key == curr.key) {
                    return false; // no duplicates
                }
                parent = curr;
                if (key < curr.key) {
                    curr = curr.left;
                } else {
                    curr = curr.right;
                }
            }
            if (key < parent.key) {
                parent.left = new Node(key);
            } else {
                parent.right = new Node(key);
            }
            return true;
        }

        public boolean search(int key) {
            if (!isIntSafe(key)) {
                return false;
            }
            Node curr = root;
            while (curr != null) {
                if (key == curr.key) return true;
                curr = (key < curr.key) ? curr.left : curr.right;
            }
            return false;
        }

        public boolean delete(int key) {
            if (!isIntSafe(key)) {
                return false;
            }
            boolean[] found = new boolean[]{false};
            root = deleteRec(root, key, found);
            return found[0];
        }

        private Node deleteRec(Node node, int key, boolean[] found) {
            if (node == null) return null;
            if (key < node.key) {
                node.left = deleteRec(node.left, key, found);
            } else if (key > node.key) {
                node.right = deleteRec(node.right, key, found);
            } else {
                found[0] = true;
                if (node.left == null) return node.right;
                if (node.right == null) return node.left;
                Node successor = minNode(node.right);
                Node newNode = new Node(successor.key);
                newNode.left = node.left;
                newNode.right = deleteMin(node.right);
                return newNode;
            }
            return node;
        }

        private Node minNode(Node node) {
            Node curr = node;
            while (curr.left != null) curr = curr.left;
            return curr;
        }

        private Node deleteMin(Node node) {
            if (node.left == null) return node.right;
            node.left = deleteMin(node.left);
            return node;
        }

        public List<Integer> inorder() {
            List<Integer> res = new ArrayList<>();
            inorderRec(root, res);
            return res;
        }

        private void inorderRec(Node node, List<Integer> res) {
            if (node == null) return;
            inorderRec(node.left, res);
            res.add(node.key);
            inorderRec(node.right, res);
        }

        private boolean isIntSafe(int value) {
            // All int values are acceptable for keys in this implementation.
            return true;
        }
    }

    public static void main(String[] args) {
        BST bst = new BST();

        // Test Case 1: Insert elements and print inorder
        int[] values = {50, 30, 70, 20, 40, 60, 80};
        for (int v : values) {
            System.out.println("Insert " + v + ": " + bst.insert(v));
        }
        System.out.println("Inorder after inserts: " + bst.inorder());

        // Test Case 2: Search existing and non-existing keys
        System.out.println("Search 60: " + bst.search(60));
        System.out.println("Search 25: " + bst.search(25));

        // Test Case 3: Delete a leaf node
        System.out.println("Delete 20 (leaf): " + bst.delete(20));
        System.out.println("Inorder after deleting 20: " + bst.inorder());

        // Test Case 4: Delete a node with one child
        System.out.println("Delete 30 (one child): " + bst.delete(30));
        System.out.println("Inorder after deleting 30: " + bst.inorder());

        // Test Case 5: Delete a node with two children and try duplicate insert
        System.out.println("Delete 50 (two children): " + bst.delete(50));
        System.out.println("Inorder after deleting 50: " + bst.inorder());
        System.out.println("Insert duplicate 70: " + bst.insert(70));
    }
}