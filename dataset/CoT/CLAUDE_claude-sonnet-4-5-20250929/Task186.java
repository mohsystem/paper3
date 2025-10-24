
import java.util.*;

class Task186 {
    static class ListNode {
        int val;
        ListNode next;
        ListNode(int val) { this.val = val; }
    }
    
    public static ListNode mergeKLists(ListNode[] lists) {
        if (lists == null || lists.length == 0) {
            return null;
        }
        
        PriorityQueue<ListNode> pq = new PriorityQueue<>((a, b) -> a.val - b.val);
        
        for (ListNode node : lists) {
            if (node != null) {
                pq.offer(node);
            }
        }
        
        ListNode dummy = new ListNode(0);
        ListNode current = dummy;
        
        while (!pq.isEmpty()) {
            ListNode node = pq.poll();
            current.next = node;
            current = current.next;
            
            if (node.next != null) {
                pq.offer(node.next);
            }
        }
        
        return dummy.next;
    }
    
    private static ListNode createList(int[] arr) {
        if (arr == null || arr.length == 0) return null;
        ListNode dummy = new ListNode(0);
        ListNode current = dummy;
        for (int val : arr) {
            current.next = new ListNode(val);
            current = current.next;
        }
        return dummy.next;
    }
    
    private static void printList(ListNode head) {
        System.out.print("[");
        while (head != null) {
            System.out.print(head.val);
            if (head.next != null) System.out.print(",");
            head = head.next;
        }
        System.out.println("]");
    }
    
    public static void main(String[] args) {
        // Test case 1
        ListNode[] lists1 = {createList(new int[]{1,4,5}), createList(new int[]{1,3,4}), createList(new int[]{2,6})};
        System.out.print("Test 1: ");
        printList(mergeKLists(lists1));
        
        // Test case 2
        ListNode[] lists2 = {};
        System.out.print("Test 2: ");
        printList(mergeKLists(lists2));
        
        // Test case 3
        ListNode[] lists3 = {null};
        System.out.print("Test 3: ");
        printList(mergeKLists(lists3));
        
        // Test case 4
        ListNode[] lists4 = {createList(new int[]{1,2,3}), createList(new int[]{4,5,6})};
        System.out.print("Test 4: ");
        printList(mergeKLists(lists4));
        
        // Test case 5
        ListNode[] lists5 = {createList(new int[]{-2,-1,0}), createList(new int[]{-3}), createList(new int[]{1,2})};
        System.out.print("Test 5: ");
        printList(mergeKLists(lists5));
    }
}
