import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;

public class Task151 {
    static class Node {
        int key;
        Node left, right;
        Node(int k) { this.key = k; }
    }

    static class BST {
        private Node root;

        public boolean insert(int key) {
            if (root == null) {
                root = new Node(key);
                return true;
            }
            Node parent = null;
            Node curr = root;
            while (curr != null) {
                parent = curr;
                if (key == curr.key) return false; // no duplicates
                if (key < curr.key) curr = curr.left;
                else curr = curr.right;
            }
            if (key < parent.key) parent.left = new Node(key);
            else parent.right = new Node(key);
            return true;
        }

        public boolean search(int key) {
            Node curr = root;
            while (curr != null) {
                if (key == curr.key) return true;
                curr = (key < curr.key) ? curr.left : curr.right;
            }
            return false;
        }

        public boolean delete(int key) {
            Node parent = null;
            Node curr = root;
            while (curr != null && curr.key != key) {
                parent = curr;
                curr = (key < curr.key) ? curr.left : curr.right;
            }
            if (curr == null) return false;

            if (curr.left != null && curr.right != null) {
                Node succParent = curr;
                Node succ = curr.right;
                while (succ.left != null) {
                    succParent = succ;
                    succ = succ.left;
                }
                curr.key = succ.key;
                parent = succParent;
                curr = succ;
            }

            Node child = (curr.left != null) ? curr.left : curr.right;
            if (parent == null) {
                root = child;
            } else if (parent.left == curr) {
                parent.left = child;
            } else {
                parent.right = child;
            }
            return true;
        }

        public List<Integer> inorder() {
            List<Integer> result = new ArrayList<>();
            Deque<Node> stack = new ArrayDeque<>();
            Node curr = root;
            while (curr != null || !stack.isEmpty()) {
                while (curr != null) {
                    stack.push(curr);
                    curr = curr.left;
                }
                curr = stack.pop();
                result.add(curr.key);
                curr = curr.right;
            }
            return result;
        }
    }

    public static void main(String[] args) {
        BST tree = new BST();
        int[] initial = {5, 3, 7, 2, 4, 6, 8};
        for (int x : initial) tree.insert(x);

        System.out.println("Search 4 (expect true): " + tree.search(4));
        System.out.println("Search 10 (expect false): " + tree.search(10));

        tree.delete(2); // delete leaf
        System.out.println("After deleting 2 (leaf): " + tree.inorder());

        tree.delete(3); // delete node with one child
        System.out.println("After deleting 3 (one child): " + tree.inorder());

        tree.delete(5); // delete node with two children
        System.out.println("After deleting 5 (two children): " + tree.inorder());
    }
}