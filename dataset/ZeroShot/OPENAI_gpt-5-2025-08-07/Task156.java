import java.util.*;

public class Task156 {
    static class TreeNode {
        int val;
        TreeNode left, right;
        TreeNode(int v) { this.val = v; }
    }

    public static List<Integer> dfs(TreeNode root, String order) {
        if (order == null) throw new IllegalArgumentException("order cannot be null");
        String ord = order.toLowerCase(Locale.ROOT).trim();
        switch (ord) {
            case "preorder":
                return preorder(root);
            case "inorder":
                return inorder(root);
            case "postorder":
                return postorder(root);
            default:
                throw new IllegalArgumentException("Unsupported order: " + order);
        }
    }

    private static List<Integer> preorder(TreeNode root) {
        List<Integer> out = new ArrayList<>();
        if (root == null) return out;
        Deque<TreeNode> st = new ArrayDeque<>();
        st.push(root);
        while (!st.isEmpty()) {
            TreeNode n = st.pop();
            out.add(n.val);
            if (n.right != null) st.push(n.right);
            if (n.left != null) st.push(n.left);
        }
        return out;
    }

    private static List<Integer> inorder(TreeNode root) {
        List<Integer> out = new ArrayList<>();
        Deque<TreeNode> st = new ArrayDeque<>();
        TreeNode curr = root;
        while (curr != null || !st.isEmpty()) {
            while (curr != null) {
                st.push(curr);
                curr = curr.left;
            }
            TreeNode n = st.pop();
            out.add(n.val);
            curr = n.right;
        }
        return out;
    }

    private static List<Integer> postorder(TreeNode root) {
        List<Integer> out = new ArrayList<>();
        if (root == null) return out;
        Deque<TreeNode> st = new ArrayDeque<>();
        TreeNode curr = root, last = null;
        while (curr != null || !st.isEmpty()) {
            if (curr != null) {
                st.push(curr);
                curr = curr.left;
            } else {
                TreeNode peek = st.peek();
                if (peek.right != null && last != peek.right) {
                    curr = peek.right;
                } else {
                    out.add(peek.val);
                    last = st.pop();
                }
            }
        }
        return out;
    }

    private static void printList(String label, List<Integer> list) {
        System.out.print(label + ": [");
        for (int i = 0; i < list.size(); i++) {
            if (i > 0) System.out.print(", ");
            System.out.print(list.get(i));
        }
        System.out.println("]");
    }

    public static void main(String[] args) {
        // Test 1: Empty tree
        TreeNode t1 = null;
        printList("Test1 - Preorder (empty)", dfs(t1, "preorder"));

        // Test 2: Single node
        TreeNode t2 = new TreeNode(42);
        printList("Test2 - Inorder (single)", dfs(t2, "inorder"));

        // Test 3: Balanced tree
        TreeNode t3 = new TreeNode(4);
        t3.left = new TreeNode(2);
        t3.right = new TreeNode(6);
        t3.left.left = new TreeNode(1);
        t3.left.right = new TreeNode(3);
        t3.right.left = new TreeNode(5);
        t3.right.right = new TreeNode(7);
        printList("Test3 - Inorder (balanced)", dfs(t3, "inorder"));

        // Test 4: Left-skewed
        TreeNode t4 = new TreeNode(5);
        t4.left = new TreeNode(4);
        t4.left.left = new TreeNode(3);
        t4.left.left.left = new TreeNode(2);
        t4.left.left.left.left = new TreeNode(1);
        printList("Test4 - Postorder (left-skewed)", dfs(t4, "postorder"));

        // Test 5: Mixed tree
        TreeNode t5 = new TreeNode(8);
        t5.left = new TreeNode(3);
        t5.right = new TreeNode(10);
        t5.left.left = new TreeNode(1);
        t5.left.right = new TreeNode(6);
        t5.left.right.left = new TreeNode(4);
        t5.left.right.right = new TreeNode(7);
        t5.right.right = new TreeNode(14);
        t5.right.right.left = new TreeNode(13);
        printList("Test5 - Preorder (mixed)", dfs(t5, "preorder"));
    }
}