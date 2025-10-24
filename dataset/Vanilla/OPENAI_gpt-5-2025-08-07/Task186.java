import java.util.*;

public class Task186 {
    static class ListNode {
        int val;
        ListNode next;
        ListNode(int v) { val = v; }
    }

    // Merge k sorted linked lists using a priority queue (min-heap)
    public static ListNode mergeKLists(ListNode[] lists) {
        PriorityQueue<ListNode> pq = new PriorityQueue<>(Comparator.comparingInt(a -> a.val));
        for (ListNode node : lists) {
            if (node != null) pq.offer(node);
        }
        ListNode dummy = new ListNode(0), tail = dummy;
        while (!pq.isEmpty()) {
            ListNode n = pq.poll();
            tail.next = n;
            tail = n;
            if (n.next != null) pq.offer(n.next);
        }
        tail.next = null;
        return dummy.next;
    }

    // Helpers
    static ListNode buildList(int[] arr) {
        ListNode dummy = new ListNode(0), t = dummy;
        for (int v : arr) { t.next = new ListNode(v); t = t.next; }
        return dummy.next;
    }

    static ListNode[] buildLists(int[][] arrs) {
        ListNode[] res = new ListNode[arrs.length];
        for (int i = 0; i < arrs.length; i++) res[i] = buildList(arrs[i]);
        return res;
    }

    static int[] toArray(ListNode head) {
        ArrayList<Integer> list = new ArrayList<>();
        for (ListNode p = head; p != null; p = p.next) list.add(p.val);
        int[] r = new int[list.size()];
        for (int i = 0; i < r.length; i++) r[i] = list.get(i);
        return r;
    }

    public static void main(String[] args) {
        int[][][] tests = new int[][][] {
            new int[][] { {1,4,5}, {1,3,4}, {2,6} },
            new int[][] { },
            new int[][] { { } },
            new int[][] { {-10,-5,0,5}, {1,2,3}, {-10,-10,100} },
            new int[][] { {1}, {0}, {-1,2,2}, { }, {3,3,3} }
        };
        for (int i = 0; i < tests.length; i++) {
            ListNode[] lists = buildLists(tests[i]);
            ListNode merged = mergeKLists(lists);
            System.out.println(Arrays.toString(toArray(merged)));
        }
    }
}