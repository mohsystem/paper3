import java.util.*;

public class Task186 {
    static final class ListNode {
        final int val;
        ListNode next;
        ListNode(int v) { this.val = v; }
    }

    public static ListNode mergeKLists(ListNode[] lists) {
        if (lists == null) return null;
        PriorityQueue<ListNode> pq = new PriorityQueue<>(Comparator.comparingInt(a -> a.val));
        for (ListNode head : lists) {
            if (head != null) {
                pq.offer(head);
            }
        }
        ListNode dummy = new ListNode(0);
        ListNode tail = dummy;
        while (!pq.isEmpty()) {
            ListNode node = pq.poll();
            tail.next = node;
            tail = node;
            if (node.next != null) {
                pq.offer(node.next);
            }
        }
        return dummy.next;
    }

    // Helpers
    public static ListNode buildList(int[] arr) {
        if (arr == null || arr.length == 0) return null;
        ListNode dummy = new ListNode(0);
        ListNode cur = dummy;
        for (int v : arr) {
            cur.next = new ListNode(v);
            cur = cur.next;
        }
        return dummy.next;
    }

    public static int[] toArray(ListNode head) {
        ArrayList<Integer> res = new ArrayList<>();
        ListNode cur = head;
        while (cur != null) {
            res.add(cur.val);
            cur = cur.next;
        }
        int[] out = new int[res.size()];
        for (int i = 0; i < res.size(); i++) out[i] = res.get(i);
        return out;
    }

    private static String arrayToString(int[] a) {
        if (a == null) return "null";
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int i = 0; i < a.length; i++) {
            if (i > 0) sb.append(",");
            sb.append(a[i]);
        }
        sb.append("]");
        return sb.toString();
    }

    public static void main(String[] args) {
        // Test 1: Example 1
        ListNode[] t1 = new ListNode[] {
            buildList(new int[]{1,4,5}),
            buildList(new int[]{1,3,4}),
            buildList(new int[]{2,6})
        };
        System.out.println(arrayToString(toArray(mergeKLists(t1))));

        // Test 2: Example 2 - empty list of lists
        ListNode[] t2 = new ListNode[] { };
        System.out.println(arrayToString(toArray(mergeKLists(t2))));

        // Test 3: Example 3 - lists = [[]]
        ListNode[] t3 = new ListNode[] { buildList(new int[]{}) };
        System.out.println(arrayToString(toArray(mergeKLists(t3))));

        // Test 4: negatives and duplicates
        ListNode[] t4 = new ListNode[] {
            buildList(new int[]{-2,-1,3}),
            buildList(new int[]{1,1,1}),
            buildList(new int[]{2})
        };
        System.out.println(arrayToString(toArray(mergeKLists(t4))));

        // Test 5: single list
        ListNode[] t5 = new ListNode[] {
            buildList(new int[]{0,5,10})
        };
        System.out.println(arrayToString(toArray(mergeKLists(t5))));
    }
}