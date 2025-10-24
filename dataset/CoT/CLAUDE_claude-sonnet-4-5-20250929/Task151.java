
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
    
    private Node root;
    
    public Task151() {
        this.root = null;
    }
    
    public void insert(int data) {
        root = insertRec(root, data);
    }
    
    private Node insertRec(Node root, int data) {
        if (root == null) {
            return new Node(data);
        }
        
        if (data < root.data) {
            root.left = insertRec(root.left, data);
        } else if (data > root.data) {
            root.right = insertRec(root.right, data);
        }
        
        return root;
    }
    
    public boolean search(int data) {
        return searchRec(root, data);
    }
    
    private boolean searchRec(Node root, int data) {
        if (root == null) {
            return false;
        }
        
        if (root.data == data) {
            return true;
        }
        
        if (data < root.data) {
            return searchRec(root.left, data);
        } else {
            return searchRec(root.right, data);
        }
    }
    
    public void delete(int data) {
        root = deleteRec(root, data);
    }
    
    private Node deleteRec(Node root, int data) {
        if (root == null) {
            return null;
        }
        
        if (data < root.data) {
            root.left = deleteRec(root.left, data);
        } else if (data > root.data) {
            root.right = deleteRec(root.right, data);
        } else {
            if (root.left == null) {
                return root.right;
            } else if (root.right == null) {
                return root.left;
            }
            
            root.data = minValue(root.right);
            root.right = deleteRec(root.right, root.data);
        }
        
        return root;
    }
    
    private int minValue(Node root) {
        int minValue = root.data;
        while (root.left != null) {
            minValue = root.left.data;
            root = root.left;
        }
        return minValue;
    }
    
    public void inorder() {
        inorderRec(root);
        System.out.println();
    }
    
    private void inorderRec(Node root) {
        if (root != null) {
            inorderRec(root.left);
            System.out.print(root.data + " ");
            inorderRec(root.right);
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Basic insertion and search
        System.out.println("Test Case 1: Basic insertion and search");
        Task151 bst1 = new Task151();
        bst1.insert(50);
        bst1.insert(30);
        bst1.insert(70);
        bst1.insert(20);
        bst1.insert(40);
        System.out.print("Inorder traversal: ");
        bst1.inorder();
        System.out.println("Search 40: " + bst1.search(40));
        System.out.println("Search 60: " + bst1.search(60));
        System.out.println();
        
        // Test Case 2: Delete leaf node
        System.out.println("Test Case 2: Delete leaf node");
        Task151 bst2 = new Task151();
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
        Task151 bst3 = new Task151();
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
        Task151 bst4 = new Task151();
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
        
        // Test Case 5: Empty tree operations
        System.out.println("Test Case 5: Empty tree operations");
        Task151 bst5 = new Task151();
        System.out.println("Search in empty tree: " + bst5.search(10));
        bst5.delete(10);
        System.out.print("Inorder of empty tree: ");
        bst5.inorder();
    }
}
