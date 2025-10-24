
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
        private Node head;
        
        public LinkedList() {
            this.head = null;
        }
        
        // Insert at the beginning
        public void insert(int data) {
            Node newNode = new Node(data);
            newNode.next = head;
            head = newNode;
        }
        
        // Insert at the end
        public void insertAtEnd(int data) {
            Node newNode = new Node(data);
            if (head == null) {
                head = newNode;
                return;
            }
            Node current = head;
            while (current.next != null) {
                current = current.next;
            }
            current.next = newNode;
        }
        
        // Delete first occurrence of a value
        public boolean delete(int data) {
            if (head == null) {
                return false;
            }
            
            // Delete head node
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
        
        // Search for a value
        public boolean search(int data) {
            Node current = head;
            while (current != null) {
                if (current.data == data) {
                    return true;
                }
                current = current.next;
            }
            return false;
        }
        
        // Display the list
        public void display() {
            if (head == null) {
                System.out.println("List is empty");
                return;
            }
            Node current = head;
            while (current != null) {
                System.out.print(current.data + " -> ");
                current = current.next;
            }
            System.out.println("null");
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Insert at beginning and display");
        LinkedList list1 = new LinkedList();
        list1.insert(10);
        list1.insert(20);
        list1.insert(30);
        list1.display();
        
        System.out.println("\\nTest Case 2: Search operations");
        LinkedList list2 = new LinkedList();
        list2.insert(5);
        list2.insert(15);
        list2.insert(25);
        System.out.println("Search 15: " + list2.search(15));
        System.out.println("Search 100: " + list2.search(100));
        
        System.out.println("\\nTest Case 3: Delete operations");
        LinkedList list3 = new LinkedList();
        list3.insert(1);
        list3.insert(2);
        list3.insert(3);
        list3.insert(4);
        System.out.println("Before delete:");
        list3.display();
        list3.delete(3);
        System.out.println("After deleting 3:");
        list3.display();
        
        System.out.println("\\nTest Case 4: Insert at end");
        LinkedList list4 = new LinkedList();
        list4.insertAtEnd(100);
        list4.insertAtEnd(200);
        list4.insertAtEnd(300);
        list4.display();
        
        System.out.println("\\nTest Case 5: Operations on empty list");
        LinkedList list5 = new LinkedList();
        System.out.println("Search in empty list: " + list5.search(10));
        System.out.println("Delete from empty list: " + list5.delete(10));
        list5.display();
    }
}
