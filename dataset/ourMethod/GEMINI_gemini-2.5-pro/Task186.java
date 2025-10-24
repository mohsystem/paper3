import java.util.Comparator;
import java.util.PriorityQueue;

class ListNode {
    int val;
    ListNode next;
    ListNode() {}
    ListNode(int val) { this.val = val; }
    ListNode(int val, ListNode next) { this.val = val; this.next = next; }
}

public class Task186 {

    /**
     * Merges k sorted linked lists into one sorted linked list.
     * @param lists An array of k sorted linked lists.
     * @return The head of the merged sorted linked list.
     */
    public ListNode mergeKLists(ListNode[] lists) {
        if (lists == null || lists.length == 0) {
            return null;
        }

        // A min-heap to store the head of each list.
        // The comparator ensures we always get the node with the smallest value.
        PriorityQueue<ListNode> minHeap = new PriorityQueue<>(Comparator.comparingInt(a -> a.val));

        // Add the head of each non-empty list to the heap.
        for (ListNode head : lists) {
            if (head != null) {
                minHeap.add(head);
            }
        }

        // A dummy node to simplify list construction.
        ListNode dummy = new ListNode(-1);
        ListNode current = dummy;

        // Process nodes from the heap until it's empty.
        while (!minHeap.isEmpty()) {
            // Get the node with the smallest value.
            ListNode minNode = minHeap.poll();
            
            // Append it to the result list.
            current.next = minNode;
            current = current.next;

            // If the extracted node has a next element, add it to the heap.
            if (minNode.next != null) {
                minHeap.add(minNode.next);
            }
        }

        return dummy.next;
    }

    // Helper function to create a linked list from an array for testing.
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

    // Helper function to print a linked list for testing.
    public static void printLinkedList(ListNode head) {
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
        ListNode[] lists1 = {
            createLinkedList(new int[]{1, 4, 5}),
            createLinkedList(new int[]{1, 3, 4}),
            createLinkedList(new int[]{2, 6})
        };
        System.out.println("Test Case 1:");
        printLinkedList(solution.mergeKLists(lists1));

        // Test Case 2
        ListNode[] lists2 = {};
        System.out.println("Test Case 2:");
        printLinkedList(solution.mergeKLists(lists2));

        // Test Case 3
        ListNode[] lists3 = {createLinkedList(new int[]{})};
        System.out.println("Test Case 3:");
        printLinkedList(solution.mergeKLists(lists3));

        // Test Case 4
        ListNode[] lists4 = {
            createLinkedList(new int[]{}),
            createLinkedList(new int[]{1})
        };
        System.out.println("Test Case 4:");
        printLinkedList(solution.mergeKLists(lists4));

        // Test Case 5
        ListNode[] lists5 = {
            createLinkedList(new int[]{9}),
            createLinkedList(new int[]{1, 5, 10}),
            createLinkedList(new int[]{2, 3, 7, 12})
        };
        System.out.println("Test Case 5:");
        printLinkedList(solution.mergeKLists(lists5));
    }
}