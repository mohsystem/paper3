
class Task186 {
    static class ListNode {
        int val;
        ListNode next;
        ListNode() {}
        ListNode(int val) { this.val = val; }
        ListNode(int val, ListNode next) { this.val = val; this.next = next; }
    }
    
    public static ListNode mergeKLists(ListNode[] lists) {
        if (lists == null || lists.length == 0) {
            return null;
        }
        
        return mergeKListsHelper(lists, 0, lists.length - 1);
    }
    
    private static ListNode mergeKListsHelper(ListNode[] lists, int left, int right) {
        if (left == right) {
            return lists[left];
        }
        
        if (left > right) {
            return null;
        }
        
        int mid = left + (right - left) / 2;
        ListNode l1 = mergeKListsHelper(lists, left, mid);
        ListNode l2 = mergeKListsHelper(lists, mid + 1, right);
        
        return mergeTwoLists(l1, l2);
    }
    
    private static ListNode mergeTwoLists(ListNode l1, ListNode l2) {
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
        
        current.next = (l1 != null) ? l1 : l2;
        
        return dummy.next;
    }
    
    private static ListNode createList(int[] arr) {
        if (arr == null || arr.length == 0) {
            return null;
        }
        ListNode head = new ListNode(arr[0]);
        ListNode current = head;
        for (int i = 1; i < arr.length; i++) {
            current.next = new ListNode(arr[i]);
            current = current.next;
        }
        return head;
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
        // Test case 1
        ListNode[] lists1 = new ListNode[3];
        lists1[0] = createList(new int[]{1, 4, 5});
        lists1[1] = createList(new int[]{1, 3, 4});
        lists1[2] = createList(new int[]{2, 6});
        System.out.print("Test 1: ");
        printList(mergeKLists(lists1));
        
        // Test case 2
        ListNode[] lists2 = new ListNode[0];
        System.out.print("Test 2: ");
        printList(mergeKLists(lists2));
        
        // Test case 3
        ListNode[] lists3 = new ListNode[1];
        lists3[0] = null;
        System.out.print("Test 3: ");
        printList(mergeKLists(lists3));
        
        // Test case 4
        ListNode[] lists4 = new ListNode[2];
        lists4[0] = createList(new int[]{1, 2, 3});
        lists4[1] = createList(new int[]{4, 5, 6});
        System.out.print("Test 4: ");
        printList(mergeKLists(lists4));
        
        // Test case 5
        ListNode[] lists5 = new ListNode[4];
        lists5[0] = createList(new int[]{-2, -1});
        lists5[1] = createList(new int[]{-3, 1, 4});
        lists5[2] = createList(new int[]{0, 2});
        lists5[3] = createList(new int[]{-1, 3});
        System.out.print("Test 5: ");
        printList(mergeKLists(lists5));
    }
}
