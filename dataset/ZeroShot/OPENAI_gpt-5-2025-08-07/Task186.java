import java.util.*;

public class Task186 {
    static class ListNode {
        int val;
        ListNode next;
        ListNode(int v) { this.val = v; }
    }

    public static ListNode mergeKLists(ListNode[] lists) {
        if (lists == null || lists.length == 0) return null;
        PriorityQueue<ListNode> pq = new PriorityQueue<>(lists.length, (a, b) -> Integer.compare(a.val, b.val));
        for (ListNode node : lists) {
            if (node != null) pq.offer(node);
        }
        ListNode dummy = new ListNode(0);
        ListNode tail = dummy;
        while (!pq.isEmpty()) {
            ListNode node = pq.poll();
            tail.next = node;
            tail = node;
            if (node.next != null) pq.offer(node.next);
        }
        return dummy.next;
    }

    // Helpers
    static ListNode buildList(int[] arr) {
        ListNode dummy = new ListNode(0), cur = dummy;
        for (int v : arr) {
            cur.next = new ListNode(v);
            cur = cur.next;
        }
        return dummy.next;
    }

    static int[] toArray(ListNode head) {
        ArrayList<Integer> list = new ArrayList<>();
        for (ListNode cur = head; cur != null; cur = cur.next) list.add(cur.val);
        int[] res = new int[list.size()];
        for (int i = 0; i < list.size(); i++) res[i] = list.get(i);
        return res;
    }

    static void printArr(int[] arr) {
        System.out.print("[");
        for (int i = 0; i < arr.length; i++) {
            if (i > 0) System.out.print(",");
            System.out.print(arr[i]);
        }
        System.out.println("]");
    }

    public static void main(String[] args) {
        // Test 1
        ListNode[] t1 = new ListNode[] {
            buildList(new int[]{1,4,5}),
            buildList(new int[]{1,3,4}),
            buildList(new int[]{2,6})
        };
        printArr(toArray(mergeKLists(t1)));

        // Test 2
        ListNode[] t2 = new ListNode[] {};
        printArr(toArray(mergeKLists(t2)));

        // Test 3
        ListNode[] t3 = new ListNode[] { buildList(new int[]{}) };
        printArr(toArray(mergeKLists(t3)));

        // Test 4
        ListNode[] t4 = new ListNode[] {
            buildList(new int[]{-10,-5,0,5}),
            buildList(new int[]{-6,-3,2,2,7}),
            buildList(new int[]{})
        };
        printArr(toArray(mergeKLists(t4)));

        // Test 5
        ListNode[] t5 = new ListNode[] {
            buildList(new int[]{1}),
            buildList(new int[]{}),
            buildList(new int[]{}),
            buildList(new int[]{0}),
            buildList(new int[]{1,1,1}),
            buildList(new int[]{-1,2})
        };
        printArr(toArray(mergeKLists(t5)));
    }
}