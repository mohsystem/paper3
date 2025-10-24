import java.util.*;

public class Task151 {
    static class Node {
        int key;
        Node left, right;
        Node(int k) { key = k; }
    }

    private Node root;

    public boolean search(int key) {
        Node cur = root;
        while (cur != null) {
            if (key == cur.key) return true;
            if (key < cur.key) cur = cur.left; else cur = cur.right;
        }
        return false;
    }

    public boolean insert(int key) {
        if (search(key)) return false;
        root = insertRec(root, key);
        return true;
    }

    private Node insertRec(Node node, int key) {
        if (node == null) return new Node(key);
        if (key < node.key) node.left = insertRec(node.left, key);
        else if (key > node.key) node.right = insertRec(node.right, key);
        return node;
    }

    public boolean delete(int key) {
        if (!search(key)) return false;
        root = deleteRec(root, key);
        return true;
    }

    private Node deleteRec(Node node, int key) {
        if (node == null) return null;
        if (key < node.key) {
            node.left = deleteRec(node.left, key);
        } else if (key > node.key) {
            node.right = deleteRec(node.right, key);
        } else {
            if (node.left == null) return node.right;
            if (node.right == null) return node.left;
            Node succ = node.right;
            while (succ.left != null) succ = succ.left;
            node.key = succ.key;
            node.right = deleteRec(node.right, succ.key);
        }
        return node;
    }

    public int[] inorder() {
        List<Integer> res = new ArrayList<>();
        inorderRec(root, res);
        int[] arr = new int[res.size()];
        for (int i = 0; i < res.size(); i++) arr[i] = res.get(i);
        return arr;
    }

    private void inorderRec(Node node, List<Integer> res) {
        if (node == null) return;
        inorderRec(node.left, res);
        res.add(node.key);
        inorderRec(node.right, res);
    }

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        Task151 bst = new Task151();
        // Test 1: Insert values and print inorder
        int[] vals = {5, 3, 7, 2, 4, 6, 8};
        for (int v : vals) System.out.print(bst.insert(v) + " ");
        System.out.println();
        printArray(bst.inorder());

        // Test 2: Search existing value
        System.out.println(bst.search(4));

        // Test 3: Search non-existing value
        System.out.println(bst.search(10));

        // Test 4: Delete leaf (2) and node with one child (3), then print inorder
        System.out.println(bst.delete(2));
        System.out.println(bst.delete(3));
        printArray(bst.inorder());

        // Test 5: Delete node with two children (7), try duplicate insert (5), delete non-existent (42)
        System.out.println(bst.delete(7));
        System.out.println(bst.insert(5));
        System.out.println(bst.delete(42));
        printArray(bst.inorder());
    }
}