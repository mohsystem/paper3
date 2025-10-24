import java.util.PriorityQueue;
import java.util.Comparator;

class ListNode {
    int val;
    ListNode next;
    ListNode() {}
    ListNode(int val) { this.val = val; }
    ListNode(int val, ListNode next) { this.val = val; this.next = next; }
}

public class Task186 {

    /**
     * Merges k sorted linked lists into one single sorted linked list.
     * @param lists An array of sorted linked lists.
     * @return The head of the merged sorted linked list.
     */
    public ListNode mergeKLists(ListNode[] lists) {
        if (lists == null || lists.length == 0) {
            return null;
        }

        PriorityQueue<ListNode> minHeap = new PriorityQueue<>(Comparator.comparingInt(a -> a.val));

        for (ListNode list : lists) {
            if (list != null) {
                minHeap.add(list);
            }
        }

        ListNode dummy = new ListNode(0);
        ListNode tail = dummy;

        while (!minHeap.isEmpty()) {
            ListNode node = minHeap.poll();
            tail.next = node;
            tail = tail.next;

            if (node.next != null) {
                minHeap.add(node.next);
            }
        }

        return dummy.next;
    }

    // Helper method to create a linked list from an array of integers
    public static ListNode createList(int[] arr) {
        if (arr == null || arr.length == 0) {
            return null;
        }
        ListNode dummy = new ListNode(0);
        ListNode current = dummy;
        for (int val : arr) {
            current.next = new ListNode(val);
            current = current.next;
        }
        return dummy.next;
    }

    // Helper method to print a linked list
    public static void printList(ListNode head) {
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

    public static void main(String[] args) {
        Task186 solution = new Task186();

        // Test Case 1
        System.out.println("Test Case 1:");
        ListNode[] lists1 = {
            createList(new int[]{1, 4, 5}),
            createList(new int[]{1, 3, 4}),
            createList(new int[]{2, 6})
        };
        System.out.print("Input: ");
        for (ListNode l : lists1) printList(l);
        ListNode result1 = solution.mergeKLists(lists1);
        System.out.print("Output: ");
        printList(result1);
        System.out.println();

        // Test Case 2
        System.out.println("Test Case 2:");
        ListNode[] lists2 = {};
        System.out.println("Input: []");
        ListNode result2 = solution.mergeKLists(lists2);
        System.out.print("Output: ");
        printList(result2);
        System.out.println();
        
        // Test Case 3
        System.out.println("Test Case 3:");
        ListNode[] lists3 = {createList(new int[]{})};
        System.out.println("Input: [[]]");
        ListNode result3 = solution.mergeKLists(lists3);
        System.out.print("Output: ");
        printList(result3);
        System.out.println();

        // Test Case 4
        System.out.println("Test Case 4:");
        ListNode[] lists4 = {
            createList(new int[]{}),
            createList(new int[]{1})
        };
        System.out.print("Input: ");
        for (ListNode l : lists4) printList(l);
        ListNode result4 = solution.mergeKLists(lists4);
        System.out.print("Output: ");
        printList(result4);
        System.out.println();
        
        // Test Case 5
        System.out.println("Test Case 5:");
        ListNode[] lists5 = {
            createList(new int[]{10, 20}),
            createList(new int[]{5, 15}),
            createList(new int[]{1, 2, 22})
        };
        System.out.print("Input: ");
        for (ListNode l : lists5) printList(l);
        ListNode result5 = solution.mergeKLists(lists5);
        System.out.print("Output: ");
        printList(result5);
        System.out.println();
    }
}