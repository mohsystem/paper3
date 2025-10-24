import java.util.Arrays;

public class Task150 {
    private static final class Node {
        final int val;
        Node next;
        Node(int v) { this.val = v; this.next = null; }
    }

    private Node head;
    private Node tail;
    private int size;

    public Task150() {
        this.head = null;
        this.tail = null;
        this.size = 0;
    }

    // Insert at tail to preserve insertion order
    public boolean insert(int value) {
        Node n = new Node(value);
        if (head == null) {
            head = n;
            tail = n;
        } else {
            tail.next = n;
            tail = n;
        }
        size++;
        return true;
    }

    // Delete first occurrence of value
    public boolean delete(int value) {
        Node prev = null;
        Node curr = head;
        while (curr != null) {
            if (curr.val == value) {
                if (prev == null) {
                    head = curr.next;
                } else {
                    prev.next = curr.next;
                }
                if (curr == tail) {
                    tail = prev;
                }
                curr.next = null; // help GC
                size--;
                return true;
            }
            prev = curr;
            curr = curr.next;
        }
        return false;
    }

    public boolean search(int value) {
        Node curr = head;
        while (curr != null) {
            if (curr.val == value) return true;
            curr = curr.next;
        }
        return false;
    }

    public int size() {
        return size;
    }

    public int[] toArray() {
        int[] arr = new int[size];
        int i = 0;
        Node curr = head;
        while (curr != null) {
            arr[i++] = curr.val;
            curr = curr.next;
        }
        return arr;
    }

    private static void printArray(int[] arr) {
        System.out.println(Arrays.toString(arr));
    }

    public static void main(String[] args) {
        // Test Case 1
        Task150 list1 = new Task150();
        list1.insert(1);
        list1.insert(2);
        list1.insert(3);
        System.out.println("TC1 search 2: " + list1.search(2));
        System.out.println("TC1 delete 2: " + list1.delete(2));
        System.out.println("TC1 search 2: " + list1.search(2));
        System.out.print("TC1 list: ");
        printArray(list1.toArray());

        // Test Case 2: delete head
        Task150 list2 = new Task150();
        list2.insert(10);
        list2.insert(20);
        System.out.println("TC2 delete 10: " + list2.delete(10));
        System.out.print("TC2 list: ");
        printArray(list2.toArray());

        // Test Case 3: delete tail
        Task150 list3 = new Task150();
        list3.insert(7);
        list3.insert(8);
        list3.insert(9);
        System.out.println("TC3 delete 9: " + list3.delete(9));
        System.out.print("TC3 list: ");
        printArray(list3.toArray());

        // Test Case 4: delete non-existent
        Task150 list4 = new Task150();
        list4.insert(5);
        System.out.println("TC4 delete 6: " + list4.delete(6));
        System.out.print("TC4 list: ");
        printArray(list4.toArray());

        // Test Case 5: search in empty list
        Task150 list5 = new Task150();
        System.out.println("TC5 search 42: " + list5.search(42));
        System.out.print("TC5 list: ");
        printArray(list5.toArray());
    }
}