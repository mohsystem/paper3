import java.util.PriorityQueue;
import java.util.Comparator;

// Definition for singly-linked list.
class ListNode {
    int val;
    ListNode next;
    ListNode() {}
    ListNode(int val) { this.val = val; }
    ListNode(int val, ListNode next) { this.val = val; this.next = next; }
}

public class Task186 {

    public ListNode mergeKLists(ListNode[] lists) {
        if (lists == null || lists.length == 0) {
            return null;
        }

        PriorityQueue<ListNode> pq = new PriorityQueue<>(Comparator.comparingInt(a -> a.val));

        for (ListNode node : lists) {
            if (node != null) {
                pq.add(node);
            }
        }

        ListNode dummy = new ListNode(-1);
        ListNode tail = dummy;

        while (!pq.isEmpty()) {
            ListNode node = pq.poll();
            tail.next = node;
            tail = tail.next;
            if (node.next != null) {
                pq.add(node.next);
            }
        }
        return dummy.next;
    }

    // Helper function to create a linked list from an array
    public static ListNode createLinkedList(int[] arr) {
        if (arr == null || arr.length == 0) {
            return null;
        }
        ListNode dummy = new ListNode(-1);
        ListNode current = dummy;
        for (int val : arr) {
            current.next = new ListNode(val);
            current = current.next;
        }
        return dummy.next;
    }

    // Helper function to print a linked list
    public static void printLinkedList(ListNode head) {
        if (head == null) {
            System.out.println("[]");
            return;
        }
        System.out.print("[");
        ListNode current = head;
        while (current != null) {
            System.out.print(current.val);
            if (current.next != null) {
                System.out.print(", ");
            }
            current = current.next;
        }
        System.out.println("]");
    }
    
    // Helper function to free memory (for illustration, not strictly needed in Java)
    public static void deleteLinkedList(ListNode head) {
        // Java's garbage collector handles this automatically.
        // This is just to have a consistent structure with C/C++.
    }

    public static void main(String[] args) {
        Task186 solution = new Task186();

        // Test Case 1: Example 1
        ListNode[] lists1 = new ListNode[]{
            createLinkedList(new int[]{1, 4, 5}),
            createLinkedList(new int[]{1, 3, 4}),
            createLinkedList(new int[]{2, 6})
        };
        System.out.println("Test Case 1:");
        ListNode result1 = solution.mergeKLists(lists1);
        printLinkedList(result1);
        deleteLinkedList(result1);


        // Test Case 2: Example 2
        ListNode[] lists2 = new ListNode[]{};
        System.out.println("\nTest Case 2:");
        ListNode result2 = solution.mergeKLists(lists2);
        printLinkedList(result2);
        deleteLinkedList(result2);

        // Test Case 3: Example 3
        ListNode[] lists3 = new ListNode[]{
            createLinkedList(new int[]{})
        };
        System.out.println("\nTest Case 3:");
        ListNode result3 = solution.mergeKLists(lists3);
        printLinkedList(result3);
        deleteLinkedList(result3);


        // Test Case 4: lists with null and single elements
        ListNode[] lists4 = new ListNode[]{
            createLinkedList(new int[]{1}),
            null,
            createLinkedList(new int[]{0})
        };
        System.out.println("\nTest Case 4:");
        ListNode result4 = solution.mergeKLists(lists4);
        printLinkedList(result4);
        deleteLinkedList(result4);
        
        // Test Case 5: lists with negative numbers
        ListNode[] lists5 = new ListNode[]{
             createLinkedList(new int[]{-2, -1, 0}),
             createLinkedList(new int[]{-3, 1, 2}),
             createLinkedList(new int[]{-5, 5, 10})
        };
        System.out.println("\nTest Case 5:");
        ListNode result5 = solution.mergeKLists(lists5);
        printLinkedList(result5);
        deleteLinkedList(result5);
    }
}