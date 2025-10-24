import java.util.PriorityQueue;

public class Task186 {

    // Definition for singly-linked list.
    public static class ListNode {
        int val;
        ListNode next;
        ListNode() {}
        ListNode(int val) { this.val = val; }
        ListNode(int val, ListNode next) { this.val = val; this.next = next; }
    }

    /**
     * Merges k sorted linked lists into one single sorted linked list.
     * This implementation uses a min-heap (PriorityQueue) to efficiently
     * find the minimum node among the heads of all lists.
     *
     * @param lists An array of sorted ListNode heads.
     * @return The head of the merged sorted linked list.
     */
    public ListNode mergeKLists(ListNode[] lists) {
        if (lists == null || lists.length == 0) {
            return null;
        }

        // A min-heap to store the head nodes of the k lists.
        // The comparator sorts nodes by their value.
        PriorityQueue<ListNode> pq = new PriorityQueue<>(lists.length, (a, b) -> a.val - b.val);

        // Add the head of each non-empty list to the priority queue.
        for (ListNode head : lists) {
            if (head != null) {
                pq.add(head);
            }
        }

        // A dummy node to simplify the construction of the result list.
        ListNode dummy = new ListNode(-1);
        ListNode tail = dummy;

        // Process nodes from the priority queue until it's empty.
        while (!pq.isEmpty()) {
            // Get the node with the smallest value.
            ListNode minNode = pq.poll();
            
            // Append it to the result list.
            tail.next = minNode;
            tail = tail.next;

            // If the extracted node has a next element, add it to the queue.
            if (minNode.next != null) {
                pq.add(minNode.next);
            }
        }

        return dummy.next;
    }

    // Main method with test cases
    public static void main(String[] args) {
        Task186 solution = new Task186();

        // Helper function to create a list from an array
        java.util.function.Function<int[], ListNode> createList = (arr) -> {
            if (arr == null || arr.length == 0) return null;
            ListNode dummy = new ListNode(-1);
            ListNode current = dummy;
            for (int val : arr) {
                current.next = new ListNode(val);
                current = current.next;
            }
            return dummy.next;
        };

        // Helper function to print a list
        java.util.function.Consumer<ListNode> printList = (head) -> {
            ListNode current = head;
            while (current != null) {
                System.out.print(current.val + " -> ");
                current = current.next;
            }
            System.out.println("NULL");
        };

        System.out.println("--- Java Tests ---");
        // Test Case 1: Example 1
        ListNode[] lists1 = {
            createList.apply(new int[]{1, 4, 5}),
            createList.apply(new int[]{1, 3, 4}),
            createList.apply(new int[]{2, 6})
        };
        System.out.print("Test Case 1: ");
        printList.accept(solution.mergeKLists(lists1));

        // Test Case 2: Example 2 (empty array)
        ListNode[] lists2 = {};
        System.out.print("Test Case 2: ");
        printList.accept(solution.mergeKLists(lists2));

        // Test Case 3: Example 3 (array with one empty list)
        ListNode[] lists3 = {createList.apply(new int[]{})};
        System.out.print("Test Case 3: ");
        printList.accept(solution.mergeKLists(lists3));

        // Test Case 4: Mix of empty and non-empty lists
        ListNode[] lists4 = {
            createList.apply(new int[]{1, 2, 3}),
            createList.apply(new int[]{}),
            createList.apply(new int[]{4, 5})
        };
        System.out.print("Test Case 4: ");
        printList.accept(solution.mergeKLists(lists4));

        // Test Case 5: Edge case with small lists
        ListNode[] lists5 = {
            createList.apply(new int[]{1}),
            createList.apply(new int[]{0})
        };
        System.out.print("Test Case 5: ");
        printList.accept(solution.mergeKLists(lists5));
    }
}