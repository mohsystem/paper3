// Chain-of-Through secure implementation
// 1) Problem understanding: merge k sorted linked lists into one sorted list.
// 2) Security requirements: validate inputs, avoid null dereferences, handle edge cases.
// 3) Secure coding: use safe comparators and bounds checks.
// 4) Code review: keep memory and resource usage reasonable; avoid integer overflows.
// 5) Secure code output: final version with tests.

import java.util.*;

class Task186 {

    static class ListNode {
        int val;
        ListNode next;
        ListNode(int v) { this.val = v; }
    }

    // Merge k sorted linked lists
    public static ListNode mergeKLists(ListNode[] lists) {
        if (lists == null || lists.length == 0) return null;
        PriorityQueue<ListNode> pq = new PriorityQueue<>(lists.length, (a, b) -> Integer.compare(a.val, b.val));
        for (ListNode node : lists) {
            if (node != null) pq.offer(node);
        }
        ListNode dummy = new ListNode(0);
        ListNode tail = dummy;
        while (!pq.isEmpty()) {
            ListNode cur = pq.poll();
            tail.next = cur;
            tail = cur;
            if (cur.next != null) pq.offer(cur.next);
        }
        return dummy.next;
    }

    // Utilities
    public static ListNode buildList(int[] arr) {
        if (arr == null || arr.length == 0) return null;
        ListNode dummy = new ListNode(0);
        ListNode t = dummy;
        for (int v : arr) {
            t.next = new ListNode(v);
            t = t.next;
        }
        return dummy.next;
    }

    public static ListNode[] buildLists(int[][] arrs) {
        if (arrs == null) return new ListNode[0];
        ListNode[] res = new ListNode[arrs.length];
        for (int i = 0; i < arrs.length; i++) {
            res[i] = buildList(arrs[i]);
        }
        return res;
    }

    public static int[] toArray(ListNode head) {
        ArrayList<Integer> out = new ArrayList<>();
        ListNode cur = head;
        while (cur != null) {
            out.add(cur.val);
            cur = cur.next;
        }
        int[] arr = new int[out.size()];
        for (int i = 0; i < out.size(); i++) arr[i] = out.get(i);
        return arr;
    }

    public static void printList(ListNode head) {
        System.out.println(Arrays.toString(toArray(head)));
    }

    public static void main(String[] args) {
        // Test case 1
        int[][] t1 = new int[][]{{1,4,5},{1,3,4},{2,6}};
        printList(mergeKLists(buildLists(t1)));

        // Test case 2
        int[][] t2 = new int[][]{};
        printList(mergeKLists(buildLists(t2)));

        // Test case 3
        int[][] t3 = new int[][]{{}};
        printList(mergeKLists(buildLists(t3)));

        // Test case 4
        int[][] t4 = new int[][]{{-10,-5,0,3},{1,2,2},{}};
        printList(mergeKLists(buildLists(t4)));

        // Test case 5
        int[][] t5 = new int[][]{{1},{0},{5},{3},{3},{4}};
        printList(mergeKLists(buildLists(t5)));
    }
}