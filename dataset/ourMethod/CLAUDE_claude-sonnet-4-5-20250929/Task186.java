
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
    public static ListNode mergeKLists(ListNode[] lists) {
        if (lists == null || lists.length == 0) {
            return null;
        }
        
        PriorityQueue<ListNode> minHeap = new PriorityQueue<>(
            new Comparator<ListNode>() {
                public int compare(ListNode a, ListNode b) {
                    return Integer.compare(a.val, b.val);
                }
            }
        );
        
        for (ListNode node : lists) {
            if (node != null) {
                minHeap.offer(node);
            }
        }
        
        ListNode dummy = new ListNode(0);
        ListNode current = dummy;
        
        while (!minHeap.isEmpty()) {
            ListNode smallest = minHeap.poll();
            current.next = smallest;
            current = current.next;
            
            if (smallest.next != null) {
                minHeap.offer(smallest.next);
            }
        }
        
        return dummy.next;
    }
    
    private static ListNode createList(int[] values) {
        if (values == null || values.length == 0) {
            return null;
        }
        ListNode dummy = new ListNode(0);
        ListNode current = dummy;
        for (int val : values) {
            current.next = new ListNode(val);
            current = current.next;
        }
        return dummy.next;
    }
    
    private static void printList(ListNode head) {
        System.out.print("[");
        while (head != null) {
            System.out.print(head.val);
            if (head.next != null) {
                System.out.print(",");
            }
            head = head.next;
        }
        System.out.println("]");
    }
    
    public static void main(String[] args) {
        ListNode[] lists1 = {
            createList(new int[]{1, 4, 5}),
            createList(new int[]{1, 3, 4}),
            createList(new int[]{2, 6})
        };
        printList(mergeKLists(lists1));
        
        ListNode[] lists2 = {};
        printList(mergeKLists(lists2));
        
        ListNode[] lists3 = {null};
        printList(mergeKLists(lists3));
        
        ListNode[] lists4 = {
            createList(new int[]{-2, -1, -1, 0}),
            createList(new int[]{-3})
        };
        printList(mergeKLists(lists4));
        
        ListNode[] lists5 = {
            createList(new int[]{1}),
            createList(new int[]{0})
        };
        printList(mergeKLists(lists5));
    }
}
