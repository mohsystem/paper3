
import java.util.NoSuchElementException;

class Task149 {
    private static class Node {
        int data;
        Node next;
        
        Node(int data) {
            this.data = data;
            this.next = null;
        }
    }
    
    private Node front;
    private Node rear;
    private int size;
    
    public Task149() {
        this.front = null;
        this.rear = null;
        this.size = 0;
    }
    
    public void enqueue(int data) {
        Node newNode = new Node(data);
        if (rear == null) {
            front = rear = newNode;
        } else {
            rear.next = newNode;
            rear = newNode;
        }
        size++;
    }
    
    public int dequeue() {
        if (isEmpty()) {
            throw new NoSuchElementException("Queue is empty");
        }
        int data = front.data;
        front = front.next;
        if (front == null) {
            rear = null;
        }
        size--;
        return data;
    }
    
    public int peek() {
        if (isEmpty()) {
            throw new NoSuchElementException("Queue is empty");
        }
        return front.data;
    }
    
    public boolean isEmpty() {
        return front == null;
    }
    
    public int getSize() {
        return size;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic enqueue and dequeue
        System.out.println("Test Case 1: Basic enqueue and dequeue");
        Task149 queue1 = new Task149();
        queue1.enqueue(10);
        queue1.enqueue(20);
        queue1.enqueue(30);
        System.out.println("Dequeue: " + queue1.dequeue());
        System.out.println("Peek: " + queue1.peek());
        System.out.println("Size: " + queue1.getSize());
        System.out.println();
        
        // Test case 2: Enqueue and peek without dequeue
        System.out.println("Test Case 2: Enqueue and peek");
        Task149 queue2 = new Task149();
        queue2.enqueue(5);
        queue2.enqueue(15);
        System.out.println("Peek: " + queue2.peek());
        System.out.println("Peek again: " + queue2.peek());
        System.out.println("Size: " + queue2.getSize());
        System.out.println();
        
        // Test case 3: Multiple operations
        System.out.println("Test Case 3: Multiple operations");
        Task149 queue3 = new Task149();
        queue3.enqueue(1);
        queue3.enqueue(2);
        System.out.println("Dequeue: " + queue3.dequeue());
        queue3.enqueue(3);
        queue3.enqueue(4);
        System.out.println("Dequeue: " + queue3.dequeue());
        System.out.println("Peek: " + queue3.peek());
        System.out.println("Size: " + queue3.getSize());
        System.out.println();
        
        // Test case 4: Empty queue
        System.out.println("Test Case 4: Empty queue check");
        Task149 queue4 = new Task149();
        System.out.println("Is empty: " + queue4.isEmpty());
        queue4.enqueue(100);
        System.out.println("Is empty: " + queue4.isEmpty());
        queue4.dequeue();
        System.out.println("Is empty after dequeue: " + queue4.isEmpty());
        System.out.println();
        
        // Test case 5: Large number of operations
        System.out.println("Test Case 5: Large number of operations");
        Task149 queue5 = new Task149();
        for (int i = 1; i <= 5; i++) {
            queue5.enqueue(i * 10);
        }
        System.out.println("Size: " + queue5.getSize());
        System.out.println("Dequeue: " + queue5.dequeue());
        System.out.println("Dequeue: " + queue5.dequeue());
        System.out.println("Peek: " + queue5.peek());
        System.out.println("Size: " + queue5.getSize());
    }
}
