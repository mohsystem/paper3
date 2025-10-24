
class Task150 {
    static class Node {
        int data;
        Node next;
        
        Node(int data) {
            this.data = data;
            this.next = null;
        }
    }
    
    static class LinkedList {
        Node head;
        
        LinkedList() {
            this.head = null;
        }
        
        void insert(int data) {
            Node newNode = new Node(data);
            if (head == null) {
                head = newNode;
            } else {
                Node current = head;
                while (current.next != null) {
                    current = current.next;
                }
                current.next = newNode;
            }
        }
        
        boolean delete(int data) {
            if (head == null) {
                return false;
            }
            
            if (head.data == data) {
                head = head.next;
                return true;
            }
            
            Node current = head;
            while (current.next != null) {
                if (current.next.data == data) {
                    current.next = current.next.next;
                    return true;
                }
                current = current.next;
            }
            return false;
        }
        
        boolean search(int data) {
            Node current = head;
            while (current != null) {
                if (current.data == data) {
                    return true;
                }
                current = current.next;
            }
            return false;
        }
        
        void display() {
            Node current = head;
            System.out.print("[");
            while (current != null) {
                System.out.print(current.data);
                if (current.next != null) {
                    System.out.print(", ");
                }
                current = current.next;
            }
            System.out.println("]");
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Basic insert and search
        System.out.println("Test Case 1: Basic insert and search");
        LinkedList list1 = new LinkedList();
        list1.insert(10);
        list1.insert(20);
        list1.insert(30);
        list1.display();
        System.out.println("Search 20: " + list1.search(20));
        System.out.println("Search 40: " + list1.search(40));
        System.out.println();
        
        // Test Case 2: Delete from middle
        System.out.println("Test Case 2: Delete from middle");
        LinkedList list2 = new LinkedList();
        list2.insert(5);
        list2.insert(15);
        list2.insert(25);
        list2.insert(35);
        System.out.print("Before delete: ");
        list2.display();
        list2.delete(15);
        System.out.print("After delete 15: ");
        list2.display();
        System.out.println();
        
        // Test Case 3: Delete head
        System.out.println("Test Case 3: Delete head");
        LinkedList list3 = new LinkedList();
        list3.insert(100);
        list3.insert(200);
        list3.insert(300);
        System.out.print("Before delete: ");
        list3.display();
        list3.delete(100);
        System.out.print("After delete head: ");
        list3.display();
        System.out.println();
        
        // Test Case 4: Delete non-existent element
        System.out.println("Test Case 4: Delete non-existent element");
        LinkedList list4 = new LinkedList();
        list4.insert(1);
        list4.insert(2);
        list4.insert(3);
        System.out.print("List: ");
        list4.display();
        System.out.println("Delete 99: " + list4.delete(99));
        System.out.println();
        
        // Test Case 5: Operations on empty list
        System.out.println("Test Case 5: Operations on empty list");
        LinkedList list5 = new LinkedList();
        System.out.println("Search in empty list: " + list5.search(10));
        System.out.println("Delete from empty list: " + list5.delete(10));
        list5.insert(50);
        System.out.print("After insert: ");
        list5.display();
    }
}
