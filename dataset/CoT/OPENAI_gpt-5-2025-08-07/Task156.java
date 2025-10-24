import java.util.*;

/*
 Chain-of-Through process:
 1) Problem understanding: Implement DFS (preorder, inorder, postorder) on a binary tree. Input: tree root and traversal type string. Output: list of visited node values.
 2) Security requirements: Validate traversal input, avoid recursion depth issues by using iterative methods, handle nulls safely.
 3) Secure coding generation: Use defensive checks, immutable return structures, and avoid unsafe casts.
 4) Code review: Ensure no unbounded recursion, handle invalid inputs gracefully, prevent NPE.
 5) Secure code output: Final code uses iterative traversals, sanitized inputs, and robust tree building.
*/
public class Task156 {
    // Node definition
    static class Node {
        final int val;
        Node left, right;
        Node(int v) { this.val = v; }
    }

    // Normalize traversal order securely
    private static String normalizeOrder(String order) {
        if (order == null) return "preorder";
        String o = order.trim().toLowerCase(Locale.ROOT);
        switch (o) {
            case "preorder":
            case "inorder":
            case "postorder":
                return o;
            default:
                return "preorder";
        }
    }

    // Iterative DFS traversal
    public static List<Integer> dfs(Node root, String order) {
        String ord = normalizeOrder(order);
        List<Integer> result = new ArrayList<>();
        if (root == null) return result;

        switch (ord) {
            case "preorder": {
                Deque<Node> st = new ArrayDeque<>();
                st.push(root);
                while (!st.isEmpty()) {
                    Node n = st.pop();
                    result.add(n.val);
                    if (n.right != null) st.push(n.right);
                    if (n.left != null) st.push(n.left);
                }
                break;
            }
            case "inorder": {
                Deque<Node> st = new ArrayDeque<>();
                Node curr = root;
                while (curr != null || !st.isEmpty()) {
                    while (curr != null) {
                        st.push(curr);
                        curr = curr.left;
                    }
                    Node n = st.pop();
                    result.add(n.val);
                    curr = n.right;
                }
                break;
            }
            case "postorder": {
                Deque<Node> st = new ArrayDeque<>();
                Node curr = root;
                Node lastVisited = null;
                while (curr != null || !st.isEmpty()) {
                    if (curr != null) {
                        st.push(curr);
                        curr = curr.left;
                    } else {
                        Node peek = st.peek();
                        if (peek.right != null && lastVisited != peek.right) {
                            curr = peek.right;
                        } else {
                            result.add(peek.val);
                            lastVisited = st.pop();
                        }
                    }
                }
                break;
            }
        }
        return result;
    }

    // Build tree from level-order array (null indicates missing node)
    public static Node buildTreeFromLevelOrder(Integer[] arr) {
        if (arr == null || arr.length == 0 || arr[0] == null) return null;
        Node root = new Node(arr[0]);
        Queue<Node> q = new ArrayDeque<>();
        q.add(root);
        int i = 1;
        while (i < arr.length && !q.isEmpty()) {
            Node cur = q.poll();
            if (cur != null) {
                if (i < arr.length) {
                    Integer lv = arr[i++];
                    if (lv != null) {
                        cur.left = new Node(lv);
                        q.add(cur.left);
                    }
                }
                if (i < arr.length) {
                    Integer rv = arr[i++];
                    if (rv != null) {
                        cur.right = new Node(rv);
                        q.add(cur.right);
                    }
                }
            }
        }
        return root;
    }

    private static String listToString(List<Integer> list) {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int i = 0; i < list.size(); i++) {
            if (i > 0) sb.append(", ");
            sb.append(list.get(i));
        }
        sb.append("]");
        return sb.toString();
    }

    // 5 test cases
    public static void main(String[] args) {
        Integer[][] tests = new Integer[][]{
            {1, 2, 3, 4, 5, null, 6},
            {1, null, 2, 3},
            {},
            {10, 5, 15, null, 7, 12, 20},
            {42}
        };
        String[] orders = {"preorder", "inorder", "postorder"};
        int caseNo = 1;
        for (Integer[] test : tests) {
            Node root = buildTreeFromLevelOrder(test);
            System.out.println("Case " + caseNo + ":");
            for (String ord : orders) {
                List<Integer> res = dfs(root, ord);
                System.out.println(" " + ord + ": " + listToString(res));
            }
            caseNo++;
        }
    }
}