
class ListNode {
    int val;
    ListNode next;
    ListNode() {}
    ListNode(int val) { this.val = val; }
    ListNode(int val, ListNode next) { this.val = val; this.next = next; }
}

class Task186 {
    public ListNode mergeKLists(ListNode[] lists) {
        if (lists == null || lists.length == 0) {
            return null;
        }
        
        return mergeKListsHelper(lists, 0, lists.length - 1);
    }
    
    private ListNode mergeKListsHelper(ListNode[] lists, int left, int right) {
        if (left == right) {
            return lists[left];
        }
        
        if (left < right) {
            int mid = left + (right - left) / 2;
            ListNode l1 = mergeKListsHelper(lists, left, mid);
            ListNode l2 = mergeKListsHelper(lists, mid + 1, right);
            return mergeTwoLists(l1, l2);
        }
        
        return null;
    }
    
    private ListNode mergeTwoLists(ListNode l1, ListNode l2) {
        ListNode dummy = new ListNode(0);
        ListNode current = dummy;
        
        while (l1 != null && l2 != null) {
            if (l1.val <= l2.val) {
                current.next = l1;
                l1 = l1.next;
            } else {
                current.next = l2;
                l2 = l2.next;
            }
            current = current.next;
        }
        
        if (l1 != null) {
            current.next = l1;
        }
        if (l2 != null) {
            current.next = l2;
        }
        
        return dummy.next;
    }
    
    private static ListNode createList(int[] arr) {
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
        Task186 solution = new Task186();
        
        // Test case 1
        ListNode[] lists1 = {
            createList(new int[]{1, 4, 5}),
            createList(new int[]{1, 3, 4}),
            createList(new int[]{2, 6})
        };
        System.out.print("Test 1: ");
        printList(solution.mergeKLists(lists1));
        
        // Test case 2
        ListNode[] lists2 = {};
        System.out.print("Test 2: ");
        printList(solution.mergeKLists(lists2));
        
        // Test case 3
        ListNode[] lists3 = {null};
        System.out.print("Test 3: ");
        printList(solution.mergeKLists(lists3));
        
        // Test case 4
        ListNode[] lists4 = {
            createList(new int[]{1, 2, 3}),
            createList(new int[]{4, 5, 6})
        };
        System.out.print("Test 4: ");
        printList(solution.mergeKLists(lists4));
        
        // Test case 5
        ListNode[] lists5 = {
            createList(new int[]{-2, -1, 0}),
            createList(new int[]{-3}),
            createList(new int[]{1, 2})
        };
        System.out.print("Test 5: ");
        printList(solution.mergeKLists(lists5));
    }
}
