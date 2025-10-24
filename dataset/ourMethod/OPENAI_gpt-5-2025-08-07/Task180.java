import java.util.ArrayList;
import java.util.List;

public class Task180 {

    // Tree node with integer value and list of children
    static final class Node {
        final int value;
        final List<Node> children;
        // Parent pointer used only for building; not used in result printing
        transient Node parent;

        Node(int value) {
            this.value = value;
            this.children = new ArrayList<>();
            this.parent = null;
        }

        void addChild(Node child) {
            if (child == null) {
                return;
            }
            this.children.add(child);
        }

        // String representation: value or value(child1,child2,...)
        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            toStringHelper(this, sb);
            return sb.toString();
        }

        private static void toStringHelper(Node node, StringBuilder sb) {
            sb.append(node.value);
            if (!node.children.isEmpty()) {
                sb.append('(');
                for (int i = 0; i < node.children.size(); i++) {
                    if (i > 0) sb.append(',');
                    toStringHelper(node.children.get(i), sb);
                }
                sb.append(')');
            }
        }
    }

    // Public API: Reparent a tree on target value. Returns new root or null if invalid.
    public static Node reparent(Node root, int targetValue) {
        if (root == null) {
            return null;
        }
        assignParents(root, null);
        Node target = find(root, targetValue);
        if (target == null) {
            return null;
        }
        return buildReoriented(target, null);
    }

    // Assign parent pointers for the original tree
    private static void assignParents(Node node, Node parent) {
        node.parent = parent;
        for (Node c : node.children) {
            assignParents(c, node);
        }
    }

    // Find node by value (first match in pre-order)
    private static Node find(Node node, int target) {
        if (node.value == target) {
            return node;
        }
        for (Node c : node.children) {
            Node f = find(c, target);
            if (f != null) return f;
        }
        return null;
    }

    // Build new tree oriented from 'node' as new root; prev prevents backtracking
    private static Node buildReoriented(Node node, Node prev) {
        Node clone = new Node(node.value);
        for (Node c : node.children) {
            if (c != prev) {
                clone.addChild(buildReoriented(c, node));
            }
        }
        if (node.parent != null && node.parent != prev) {
            clone.addChild(buildReoriented(node.parent, node));
        }
        return clone;
    }

    // Build the example tree:
    //        0
    //      / | \
    //     1  2  3
    //    / \ / \ / \
    //   4  5 6  7 8  9
    private static Node buildExampleTree() {
        Node n0 = new Node(0);
        Node n1 = new Node(1);
        Node n2 = new Node(2);
        Node n3 = new Node(3);
        Node n4 = new Node(4);
        Node n5 = new Node(5);
        Node n6 = new Node(6);
        Node n7 = new Node(7);
        Node n8 = new Node(8);
        Node n9 = new Node(9);

        n0.addChild(n1);
        n0.addChild(n2);
        n0.addChild(n3);

        n1.addChild(n4);
        n1.addChild(n5);

        n2.addChild(n6);
        n2.addChild(n7);

        n3.addChild(n8);
        n3.addChild(n9);

        return n0;
    }

    private static Node buildSingleNode(int value) {
        return new Node(value);
    }

    public static void main(String[] args) {
        // Test 1: Reparent on 6 (example)
        Node root = buildExampleTree();
        Node r1 = reparent(root, 6);
        System.out.println(r1 != null ? r1.toString() : "null");

        // Test 2: Reparent on 0 (root stays same)
        Node r2 = reparent(root, 0);
        System.out.println(r2 != null ? r2.toString() : "null");

        // Test 3: Reparent on 9 (leaf in another branch)
        Node r3 = reparent(root, 9);
        System.out.println(r3 != null ? r3.toString() : "null");

        // Test 4: Single node tree, reparent on that node
        Node single = buildSingleNode(42);
        Node r4 = reparent(single, 42);
        System.out.println(r4 != null ? r4.toString() : "null");

        // Test 5: Target not found
        Node r5 = reparent(root, 100);
        System.out.println(r5 != null ? r5.toString() : "null");
    }
}