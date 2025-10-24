
import java.util.*;

class Task151 {
    static class Node {
        int data;
        Node left, right;
        
        Node(int data) {
            this.data = data;
            this.left = null;
            this.right = null;
        }
    }
    
    static class BinarySearchTree {
        Node root;
        
        BinarySearchTree() {
            this.root = null;
        }
        
        public Node insert(Node root, int data) {
            if (root == null) {
                return new Node(data);
            }
            
            if (data < root.data) {
                root.left = insert(root.left, data);
            } else if (data > root.data) {
                root.right = insert(root.right, data);
            }
            
            return root;
        }
        
        public void insert(int data) {
            root = insert(root, data);
        }
        
        public boolean search(Node root, int data) {
            if (root == null) {
                return false;
            }
            
            if (root.data == data) {
                return true;
            }
            
            if (data < root.data) {
                return search(root.left, data);
            } else {
                return search(root.right, data);
            }
        }
        
        public boolean search(int data) {
            return search(root, data);
        }
        
        public Node delete(Node root, int data) {
            if (root == null) {
                return null;
            }
            
            if (data < root.data) {
                root.left = delete(root.left, data);
            } else if (data > root.data) {
                root.right = delete(root.right, data);
            } else {
                // Node to be deleted found
                
                // Case 1: No child or one child
                if (root.left == null) {
                    return root.right;
                } else if (root.right == null) {
                    return root.left;
                }
                
                // Case 2: Two children
                // Find inorder successor (smallest in right subtree)
                Node successor = findMin(root.right);
                root.data = successor.data;
                root.right = delete(root.right, successor.data);
            }
            
            return root;
        }
        
        public void delete(int data) {
            root = delete(root, data);
        }
        
        private Node findMin(Node root) {
            while (root.left != null) {
                root = root.left;
            }
            return root;
        }
        
        public void inorder(Node root, List<Integer> result) {
            if (root != null) {
                inorder(root.left, result);
                result.add(root.data);
                inorder(root.right, result);
            }
        }
        
        public List<Integer> inorder() {
            List<Integer> result = new ArrayList<>();
            inorder(root, result);
            return result;
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Basic insert and search
        System.out.println("Test Case 1: Basic insert and search");
        BinarySearchTree bst1 = new BinarySearchTree();
        bst1.insert(50);
        bst1.insert(30);
        bst1.insert(70);
        bst1.insert(20);
        bst1.insert(40);
        System.out.println("Search 40: " + bst1.search(40));
        System.out.println("Search 60: " + bst1.search(60));
        System.out.println("Inorder: " + bst1.inorder());
        System.out.println();
        
        // Test Case 2: Delete leaf node
        System.out.println("Test Case 2: Delete leaf node");
        BinarySearchTree bst2 = new BinarySearchTree();
        bst2.insert(50);
        bst2.insert(30);
        bst2.insert(70);
        bst2.insert(20);
        bst2.insert(40);
        bst2.delete(20);
        System.out.println("After deleting 20: " + bst2.inorder());
        System.out.println("Search 20: " + bst2.search(20));
        System.out.println();
        
        // Test Case 3: Delete node with one child
        System.out.println("Test Case 3: Delete node with one child");
        BinarySearchTree bst3 = new BinarySearchTree();
        bst3.insert(50);
        bst3.insert(30);
        bst3.insert(70);
        bst3.insert(60);
        bst3.delete(70);
        System.out.println("After deleting 70: " + bst3.inorder());
        System.out.println();
        
        // Test Case 4: Delete node with two children
        System.out.println("Test Case 4: Delete node with two children");
        BinarySearchTree bst4 = new BinarySearchTree();
        bst4.insert(50);
        bst4.insert(30);
        bst4.insert(70);
        bst4.insert(20);
        bst4.insert(40);
        bst4.insert(60);
        bst4.insert(80);
        bst4.delete(50);
        System.out.println("After deleting 50: " + bst4.inorder());
        System.out.println();
        
        // Test Case 5: Complex operations
        System.out.println("Test Case 5: Complex operations");
        BinarySearchTree bst5 = new BinarySearchTree();
        int[] values = {15, 10, 20, 8, 12, 17, 25};
        for (int val : values) {
            bst5.insert(val);
        }
        System.out.println("Initial tree: " + bst5.inorder());
        System.out.println("Search 12: " + bst5.search(12));
        bst5.delete(15);
        System.out.println("After deleting 15: " + bst5.inorder());
        bst5.insert(14);
        System.out.println("After inserting 14: " + bst5.inorder());
    }
}
