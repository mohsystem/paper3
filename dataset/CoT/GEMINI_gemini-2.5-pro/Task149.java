import java.util.NoSuchElementException;

public class Task149 {

    // Node class for the linked list
    private static class Node {
        int data;
        Node next;

        Node(int data) {
            this.data = data;
            this.next = null;
        }
    }

    private Node front, rear;

    public Task149() {
        this.front = this.rear = null;
    }

    // Method to add an item to the queue.
    public void enqueue(int item) {
        Node newNode = new Node(item);

        // If queue is empty, then new node is front and rear both
        if (this.rear == null) {
            this.front = this.rear = newNode;
            return;
        }

        // Add the new node at the end of queue and change rear
        this.rear.next = newNode;
        this.rear = newNode;
    }

    // Method to remove an item from queue.
    public int dequeue() {
        // If queue is empty, throw an exception
        if (this.front == null) {
            throw new NoSuchElementException("Queue is empty. Cannot dequeue.");
        }

        // Store previous front and move front one node ahead
        Node temp = this.front;
        this.front = this.front.next;

        // If front becomes NULL, then change rear also as NULL
        if (this.front == null) {
            this.rear = null;
        }
        return temp.data;
    }

    // Method to get the front of queue.
    public int peek() {
        // If queue is empty, throw an exception
        if (this.front == null) {
            throw new NoSuchElementException("Queue is empty. Cannot peek.");
        }
        return this.front.data;
    }

    // Method to check if the queue is empty.
    public boolean isEmpty() {
        return this.front == null;
    }

    public static void main(String[] args) {
        System.out.println("Java Queue Implementation Test");
        Task149 queue = new Task149();

        // Test Case 1: Enqueue elements
        System.out.println("Test Case 1: Enqueue 10, 20, 30");
        queue.enqueue(10);
        queue.enqueue(20);
        queue.enqueue(30);
        System.out.println("Front element is: " + queue.peek());

        // Test Case 2: Dequeue an element
        System.out.println("\nTest Case 2: Dequeue");
        System.out.println("Dequeued element: " + queue.dequeue());
        System.out.println("Front element is now: " + queue.peek());

        // Test Case 3: Enqueue another element
        System.out.println("\nTest Case 3: Enqueue 40");
        queue.enqueue(40);
        System.out.println("Front element is: " + queue.peek());

        // Test Case 4: Dequeue all elements
        System.out.println("\nTest Case 4: Dequeue all elements");
        System.out.println("Dequeued element: " + queue.dequeue()); // 20
        System.out.println("Dequeued element: " + queue.dequeue()); // 30
        System.out.println("Dequeued element: " + queue.dequeue()); // 40
        System.out.println("Is queue empty? " + queue.isEmpty());

        // Test Case 5: Underflow condition
        System.out.println("\nTest Case 5: Underflow condition");
        try {
            queue.peek();
        } catch (NoSuchElementException e) {
            System.out.println("Caught expected exception on peek: " + e.getMessage());
        }
        try {
            queue.dequeue();
        } catch (NoSuchElementException e) {
            System.out.println("Caught expected exception on dequeue: " + e.getMessage());
        }
    }
}