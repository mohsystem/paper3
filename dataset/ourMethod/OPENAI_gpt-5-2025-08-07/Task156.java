import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task156 {

    static final class TreeNode {
        final int val;
        TreeNode left;
        TreeNode right;
        TreeNode(int v) { this.val = v; }
    }

    public static TreeNode buildTree(int[] values, boolean[] present) {
        if (values == null || present == null) {
            throw new IllegalArgumentException("values/present cannot be null");
        }
        if (values.length != present.length) {
            throw new IllegalArgumentException("values and present arrays must have same length");
        }
        int n = values.length;
        if (n == 0) return null;
        if (!present[0]) return null;

        TreeNode[] nodes = new TreeNode[n];
        for (int i = 0; i < n; i++) {
            if (present[i]) {
                nodes[i] = new TreeNode(values[i]);
            }
        }
        for (int i = 0; i < n; i++) {
            if (nodes[i] == null) continue;
            int li = 2 * i + 1;
            int ri = 2 * i + 2;
            if (li < n && nodes[li] != null) nodes[i].left = nodes[li];
            if (ri < n && nodes[ri] != null) nodes[i].right = nodes[ri];
        }
        return nodes[0];
    }

    public static List<Integer> traverse(TreeNode root, String order) {
        if (order == null) throw new IllegalArgumentException("order cannot be null");
        String o = order.toLowerCase();
        List<Integer> result = new ArrayList<>();
        switch (o) {
            case "preorder":
                preorder(root, result);
                break;
            case "inorder":
                inorder(root, result);
                break;
            case "postorder":
                postorder(root, result);
                break;
            default:
                throw new IllegalArgumentException("Unsupported order: " + order);
        }
        return result;
    }

    private static void preorder(TreeNode node, List<Integer> out) {
        if (node == null) return;
        out.add(node.val);
        preorder(node.left, out);
        preorder(node.right, out);
    }

    private static void inorder(TreeNode node, List<Integer> out) {
        if (node == null) return;
        inorder(node.left, out);
        out.add(node.val);
        inorder(node.right, out);
    }

    private static void postorder(TreeNode node, List<Integer> out) {
        if (node == null) return;
        postorder(node.left, out);
        postorder(node.right, out);
        out.add(node.val);
    }

    public static String formatList(List<Integer> list) {
        StringBuilder sb = new StringBuilder();
        sb.append('[');
        for (int i = 0; i < list.size(); i++) {
            if (i > 0) sb.append(',');
            sb.append(list.get(i));
        }
        sb.append(']');
        return sb.toString();
    }

    public static void main(String[] args) {
        int[][] testsValues = new int[][]{
            {1,2,3,4,5,6,7},
            {1,2,3,0,5,0,7},
            {42},
            {},
            {1,0,2,0,0,0,3}
        };
        boolean[][] testsPresent = new boolean[][]{
            {true,true,true,true,true,true,true},
            {true,true,true,false,true,false,true},
            {true},
            {},
            {true,false,true,false,false,false,true}
        };

        String[] orders = new String[]{"preorder", "inorder", "postorder"};

        for (int t = 0; t < testsValues.length; t++) {
            System.out.println("Test #" + (t + 1));
            try {
                TreeNode root = buildTree(testsValues[t], testsPresent[t]);
                for (String ord : orders) {
                    List<Integer> res = traverse(root, ord);
                    System.out.println(ord + ": " + formatList(res));
                }
            } catch (IllegalArgumentException ex) {
                System.out.println("Error: " + ex.getMessage());
            }
            System.out.println();
        }
    }
}