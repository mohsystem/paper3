
class Task151 {
    static class Node {
        int data;
        Node left, right;
        
        Node(int value) {
            data = value;
            left = right = null;
        }
    }
    
    static class BST {
        Node root;
        
        BST() {
            root = null;
        }
        
        Node insert(Node node, int value) {
            if (node == null) {
                return new Node(value);
            }
            
            if (value < node.data) {
                node.left = insert(node.left, value);
            } else if (value > node.data) {
                node.right = insert(node.right, value);
            }
            
            return node;
        }
        
        void insert(int value) {
            root = insert(root, value);
        }
        
        Node search(Node node, int value) {
            if (node == null || node.data == value) {
                return node;
            }
            
            if (value < node.data) {
                return search(node.left, value);
            }
            
            return search(node.right, value);
        }
        
        boolean search(int value) {
            return search(root, value) != null;
        }
        
        Node findMin(Node node) {
            while (node.left != null) {
                node = node.left;
            }
            return node;
        }
        
        Node delete(Node node, int value) {
            if (node == null) {
                return null;
            }
            
            if (value < node.data) {
                node.left = delete(node.left, value);
            } else if (value > node.data) {
                node.right = delete(node.right, value);
            } else {
                if (node.left == null) {
                    return node.right;
                } else if (node.right == null) {
                    return node.left;
                }
                
                Node minNode = findMin(node.right);
                node.data = minNode.data;
                node.right = delete(node.right, minNode.data);
            }
            
            return node;
        }
        
        void delete(int value) {
            root = delete(root, value);
        }
        
        void inorder(Node node) {
            if (node != null) {
                inorder(node.left);
                System.out.print(node.data + " ");
                inorder(node.right);
            }
        }
        
        void inorder() {
            inorder(root);
            System.out.println();
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Basic insert and search
        System.out.println("Test Case 1: Basic insert and search");
        BST bst1 = new BST();
        bst1.insert(50);
        bst1.insert(30);
        bst1.insert(70);
        bst1.insert(20);
        bst1.insert(40);
        System.out.print("Inorder: ");
        bst1.inorder();
        System.out.println("Search 40: " + bst1.search(40));
        System.out.println("Search 60: " + bst1.search(60));
        System.out.println();
        
        // Test Case 2: Delete leaf node
        System.out.println("Test Case 2: Delete leaf node");
        BST bst2 = new BST();
        bst2.insert(50);
        bst2.insert(30);
        bst2.insert(70);
        bst2.insert(20);
        System.out.print("Before delete: ");
        bst2.inorder();
        bst2.delete(20);
        System.out.print("After delete 20: ");
        bst2.inorder();
        System.out.println();
        
        // Test Case 3: Delete node with one child
        System.out.println("Test Case 3: Delete node with one child");
        BST bst3 = new BST();
        bst3.insert(50);
        bst3.insert(30);
        bst3.insert(70);
        bst3.insert(60);
        System.out.print("Before delete: ");
        bst3.inorder();
        bst3.delete(70);
        System.out.print("After delete 70: ");
        bst3.inorder();
        System.out.println();
        
        // Test Case 4: Delete node with two children
        System.out.println("Test Case 4: Delete node with two children");
        BST bst4 = new BST();
        bst4.insert(50);
        bst4.insert(30);
        bst4.insert(70);
        bst4.insert(20);
        bst4.insert(40);
        bst4.insert(60);
        bst4.insert(80);
        System.out.print("Before delete: ");
        bst4.inorder();
        bst4.delete(50);
        System.out.print("After delete 50: ");
        bst4.inorder();
        System.out.println();
        
        // Test Case 5: Multiple operations
        System.out.println("Test Case 5: Multiple operations");
        BST bst5 = new BST();
        bst5.insert(15);
        bst5.insert(10);
        bst5.insert(20);
        bst5.insert(8);
        bst5.insert(12);
        bst5.insert(17);
        bst5.insert(25);
        System.out.print("Initial tree: ");
        bst5.inorder();
        System.out.println("Search 12: " + bst5.search(12));
        bst5.delete(15);
        System.out.print("After delete 15: ");
        bst5.inorder();
        bst5.insert(18);
        System.out.print("After insert 18: ");
        bst5.inorder();
    }
}
