import java.util.ArrayList;
import java.util.List;

public class Task151 {

    // Chain-of-Through Step 1: Define purpose and operations (BST with insert, delete, search)
    // Chain-of-Through Step 2-5: Apply secure coding practices (no external inputs, handle nulls, avoid duplicates)

    static final class BST {
        private static final class Node {
            int key;
            Node left, right;
            Node(int k) { this.key = k; }
        }

        private static final class BoolRef {
            boolean val;
            BoolRef(boolean v) { this.val = v; }
        }

        private Node root;

        public BST() {
            this.root = null;
        }

        public boolean insert(int key) {
            BoolRef changed = new BoolRef(false);
            root = insertRec(root, key, changed);
            return changed.val;
        }

        private Node insertRec(Node node, int key, BoolRef changed) {
            if (node == null) {
                changed.val = true;
                return new Node(key);
            }
            if (key < node.key) {
                node.left = insertRec(node.left, key, changed);
            } else if (key > node.key) {
                node.right = insertRec(node.right, key, changed);
            } else {
                // duplicate key: ignore insert
                changed.val = false;
            }
            return node;
        }

        public boolean search(int key) {
            Node cur = root;
            while (cur != null) {
                if (key == cur.key) return true;
                if (key < cur.key) cur = cur.left; else cur = cur.right;
            }
            return false;
        }

        public boolean delete(int key) {
            BoolRef deleted = new BoolRef(false);
            root = deleteRec(root, key, deleted);
            return deleted.val;
        }

        private Node deleteRec(Node node, int key, BoolRef deleted) {
            if (node == null) return null;
            if (key < node.key) {
                node.left = deleteRec(node.left, key, deleted);
            } else if (key > node.key) {
                node.right = deleteRec(node.right, key, deleted);
            } else {
                deleted.val = true;
                if (node.left == null) return node.right;
                if (node.right == null) return node.left;
                // two children: replace with inorder successor
                Node succParent = node;
                Node succ = node.right;
                while (succ.left != null) {
                    succParent = succ;
                    succ = succ.left;
                }
                node.key = succ.key;
                if (succParent.left == succ) {
                    succParent.left = deleteRec(succ, succ.key, new BoolRef(false));
                } else {
                    succParent.right = deleteRec(succ, succ.key, new BoolRef(false));
                }
            }
            return node;
        }

        public List<Integer> inorder() {
            ArrayList<Integer> res = new ArrayList<>();
            inorderRec(root, res);
            return res;
        }

        private void inorderRec(Node node, List<Integer> out) {
            if (node == null) return;
            inorderRec(node.left, out);
            out.add(node.key);
            inorderRec(node.right, out);
        }
    }

    private static void printList(String prefix, List<Integer> list) {
        StringBuilder sb = new StringBuilder(prefix);
        sb.append("[");
        for (int i = 0; i < list.size(); i++) {
            if (i > 0) sb.append(", ");
            sb.append(list.get(i));
        }
        sb.append("]");
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        // Test case 1: Insert and inorder
        BST bst = new BST();
        int[] values = {50, 30, 20, 40, 70, 60, 80};
        for (int v : values) bst.insert(v);
        printList("Inorder after inserts: ", bst.inorder());

        // Test case 2: Search existing
        System.out.println("Search 40: " + bst.search(40));

        // Test case 3: Search non-existing
        System.out.println("Search 25: " + bst.search(25));

        // Test case 4: Delete leaf (20)
        System.out.println("Delete 20: " + bst.delete(20));
        printList("Inorder after deleting 20: ", bst.inorder());

        // Test case 5: Delete node with one child (30) and two children (50)
        System.out.println("Delete 30: " + bst.delete(30));
        System.out.println("Delete 50: " + bst.delete(50));
        printList("Inorder after deleting 30 and 50: ", bst.inorder());
    }
}