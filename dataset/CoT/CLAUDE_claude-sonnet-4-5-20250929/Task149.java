
import java.util.NoSuchElementException;

class Task149 {
    private static class Queue<T> {
        private Node<T> front;
        private Node<T> rear;
        private int size;
        
        private static class Node<T> {
            T data;
            Node<T> next;
            
            Node(T data) {
                this.data = data;
                this.next = null;
            }
        }
        
        public Queue() {
            this.front = null;
            this.rear = null;
            this.size = 0;
        }
        
        public void enqueue(T item) {
            if (item == null) {
                throw new IllegalArgumentException("Cannot enqueue null item");
            }
            
            Node<T> newNode = new Node<>(item);
            
            if (isEmpty()) {
                front = newNode;
                rear = newNode;
            } else {
                rear.next = newNode;
                rear = newNode;
            }
            size++;
        }
        
        public T dequeue() {
            if (isEmpty()) {
                throw new NoSuchElementException("Queue is empty");
            }
            
            T data = front.data;
            front = front.next;
            
            if (front == null) {
                rear = null;
            }
            
            size--;
            return data;
        }
        
        public T peek() {
            if (isEmpty()) {
                throw new NoSuchElementException("Queue is empty");
            }
            return front.data;
        }
        
        public boolean isEmpty() {
            return front == null;
        }
        
        public int size() {
            return size;
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Basic enqueue and dequeue operations
        System.out.println("Test Case 1: Basic operations");
        Queue<Integer> queue1 = new Queue<>();
        queue1.enqueue(10);
        queue1.enqueue(20);
        queue1.enqueue(30);
        System.out.println("Peek: " + queue1.peek());
        System.out.println("Dequeue: " + queue1.dequeue());
        System.out.println("Peek after dequeue: " + queue1.peek());
        System.out.println();
        
        // Test Case 2: String queue
        System.out.println("Test Case 2: String queue");
        Queue<String> queue2 = new Queue<>();
        queue2.enqueue("Hello");
        queue2.enqueue("World");
        queue2.enqueue("Java");
        System.out.println("Size: " + queue2.size());
        System.out.println("Dequeue: " + queue2.dequeue());
        System.out.println("Dequeue: " + queue2.dequeue());
        System.out.println();
        
        // Test Case 3: Empty queue check
        System.out.println("Test Case 3: Empty queue operations");
        Queue<Integer> queue3 = new Queue<>();
        System.out.println("Is empty: " + queue3.isEmpty());
        queue3.enqueue(100);
        System.out.println("Is empty after enqueue: " + queue3.isEmpty());
        queue3.dequeue();
        System.out.println("Is empty after dequeue: " + queue3.isEmpty());
        System.out.println();
        
        // Test Case 4: Multiple operations
        System.out.println("Test Case 4: Multiple operations");
        Queue<Integer> queue4 = new Queue<>();
        for (int i = 1; i <= 5; i++) {
            queue4.enqueue(i * 10);
        }
        System.out.println("Size: " + queue4.size());
        while (!queue4.isEmpty()) {
            System.out.println("Dequeue: " + queue4.dequeue());
        }
        System.out.println();
        
        // Test Case 5: Error handling
        System.out.println("Test Case 5: Error handling");
        Queue<Integer> queue5 = new Queue<>();
        try {
            queue5.dequeue();
        } catch (NoSuchElementException e) {
            System.out.println("Caught exception: " + e.getMessage());
        }
        try {
            queue5.enqueue(null);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught exception: " + e.getMessage());
        }
    }
}
