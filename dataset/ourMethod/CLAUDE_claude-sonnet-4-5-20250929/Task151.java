
import java.util.ArrayList;
import java.util.List;

public class Task151 {
    private static class Node {
        private final int value;
        private Node left;
        private Node right;

        public Node(int value) {
            this.value = value;
            this.left = null;
            this.right = null;
        }

        public int getValue() {
            return value;
        }

        public Node getLeft() {
            return left;
        }

        public void setLeft(Node left) {
            this.left = left;
        }

        public Node getRight() {
            return right;
        }

        public void setRight(Node right) {
            this.right = right;
        }
    }

    private static class BST {
        private Node root;
        private int size;
        private static final int MAX_SIZE = 100000;

        public BST() {
            this.root = null;
            this.size = 0;
        }

        public boolean insert(int value) {
            if (size >= MAX_SIZE) {
                return false;
            }
            if (root == null) {
                root = new Node(value);
                size++;
                return true;
            }
            return insertHelper(root, value);
        }

        private boolean insertHelper(Node node, int value) {
            if (value == node.getValue()) {
                return false;
            }
            if (value < node.getValue()) {
                if (node.getLeft() == null) {
                    node.setLeft(new Node(value));
                    size++;
                    return true;
                }
                return insertHelper(node.getLeft(), value);
            } else {
                if (node.getRight() == null) {
                    node.setRight(new Node(value));
                    size++;
                    return true;
                }
                return insertHelper(node.getRight(), value);
            }
        }

        public boolean search(int value) {
            return searchHelper(root, value);
        }

        private boolean searchHelper(Node node, int value) {
            if (node == null) {
                return false;
            }
            if (value == node.getValue()) {
                return true;
            }
            if (value < node.getValue()) {
                return searchHelper(node.getLeft(), value);
            } else {
                return searchHelper(node.getRight(), value);
            }
        }

        public boolean delete(int value) {
            if (root == null) {
                return false;
            }
            int oldSize = size;
            root = deleteHelper(root, value);
            return size < oldSize;
        }

        private Node deleteHelper(Node node, int value) {
            if (node == null) {
                return null;
            }

            if (value < node.getValue()) {
                node.setLeft(deleteHelper(node.getLeft(), value));
                return node;
            } else if (value > node.getValue()) {
                node.setRight(deleteHelper(node.getRight(), value));
                return node;
            } else {
                size--;
                if (node.getLeft() == null && node.getRight() == null) {
                    return null;
                }
                if (node.getLeft() == null) {
                    return node.getRight();
                }
                if (node.getRight() == null) {
                    return node.getLeft();
                }
                Node minNode = findMin(node.getRight());
                Node newNode = new Node(minNode.getValue());
                size++;
                newNode.setRight(deleteHelper(node.getRight(), minNode.getValue()));
                newNode.setLeft(node.getLeft());
                return newNode;
            }
        }

        private Node findMin(Node node) {
            while (node.getLeft() != null) {
                node = node.getLeft();
            }
            return node;
        }

        public List<Integer> inorderTraversal() {
            List<Integer> result = new ArrayList<>();
            inorderHelper(root, result);
            return result;
        }

        private void inorderHelper(Node node, List<Integer> result) {
            if (node == null) {
                return;
            }
            inorderHelper(node.getLeft(), result);
            result.add(node.getValue());
            inorderHelper(node.getRight(), result);
        }
    }

    public static void main(String[] args) {
        System.out.println("Test Case 1: Basic insert and search");
        BST bst1 = new BST();
        bst1.insert(50);
        bst1.insert(30);
        bst1.insert(70);
        System.out.println("Search 30: " + bst1.search(30));
        System.out.println("Search 100: " + bst1.search(100));
        System.out.println("Inorder: " + bst1.inorderTraversal());

        System.out.println("\\nTest Case 2: Delete leaf node");
        BST bst2 = new BST();
        bst2.insert(50);
        bst2.insert(30);
        bst2.insert(70);
        bst2.delete(30);
        System.out.println("After deleting 30: " + bst2.inorderTraversal());
        System.out.println("Search 30: " + bst2.search(30));

        System.out.println("\\nTest Case 3: Delete node with one child");
        BST bst3 = new BST();
        bst3.insert(50);
        bst3.insert(30);
        bst3.insert(70);
        bst3.insert(60);
        bst3.delete(70);
        System.out.println("After deleting 70: " + bst3.inorderTraversal());

        System.out.println("\\nTest Case 4: Delete node with two children");
        BST bst4 = new BST();
        bst4.insert(50);
        bst4.insert(30);
        bst4.insert(70);
        bst4.insert(20);
        bst4.insert(40);
        bst4.insert(60);
        bst4.insert(80);
        bst4.delete(50);
        System.out.println("After deleting 50: " + bst4.inorderTraversal());

        System.out.println("\\nTest Case 5: Multiple operations");
        BST bst5 = new BST();
        bst5.insert(15);
        bst5.insert(10);
        bst5.insert(20);
        bst5.insert(8);
        bst5.insert(12);
        bst5.insert(25);
        System.out.println("Initial tree: " + bst5.inorderTraversal());
        System.out.println("Search 12: " + bst5.search(12));
        bst5.delete(20);
        System.out.println("After deleting 20: " + bst5.inorderTraversal());
        System.out.println("Search 20: " + bst5.search(20));
    }
}
