
import java.util.Arrays;

public class Task149 {
    private static final int DEFAULT_CAPACITY = 10;
    private static final int MAX_CAPACITY = 1000000;
    
    private int[] data;
    private int front;
    private int rear;
    private int size;
    private int capacity;
    
    public Task149() {
        this(DEFAULT_CAPACITY);
    }
    
    public Task149(int initialCapacity) {
        if (initialCapacity <= 0 || initialCapacity > MAX_CAPACITY) {
            throw new IllegalArgumentException("Invalid capacity: " + initialCapacity);
        }
        this.capacity = initialCapacity;
        this.data = new int[capacity];
        this.front = 0;
        this.rear = -1;
        this.size = 0;
    }
    
    public boolean enqueue(int value) {
        if (size >= capacity) {
            if (capacity >= MAX_CAPACITY) {
                return false;
            }
            resize();
        }
        rear = (rear + 1) % capacity;
        data[rear] = value;
        size++;
        return true;
    }
    
    public Integer dequeue() {
        if (isEmpty()) {
            return null;
        }
        int value = data[front];
        front = (front + 1) % capacity;
        size--;
        return value;
    }
    
    public Integer peek() {
        if (isEmpty()) {
            return null;
        }
        return data[front];
    }
    
    public boolean isEmpty() {
        return size == 0;
    }
    
    public int getSize() {
        return size;
    }
    
    private void resize() {
        long newCapacityLong = (long) capacity * 2;
        if (newCapacityLong > MAX_CAPACITY) {
            newCapacityLong = MAX_CAPACITY;
        }
        int newCapacity = (int) newCapacityLong;
        
        if (newCapacity <= capacity) {
            return;
        }
        
        int[] newData = new int[newCapacity];
        for (int i = 0; i < size; i++) {
            newData[i] = data[(front + i) % capacity];
        }
        data = newData;
        front = 0;
        rear = size - 1;
        capacity = newCapacity;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic enqueue and dequeue
        System.out.println("Test 1: Basic operations");
        Task149 queue1 = new Task149(5);
        queue1.enqueue(10);
        queue1.enqueue(20);
        queue1.enqueue(30);
        System.out.println("Peek: " + queue1.peek());
        System.out.println("Dequeue: " + queue1.dequeue());
        System.out.println("Dequeue: " + queue1.dequeue());
        System.out.println("Size: " + queue1.getSize());
        System.out.println();
        
        // Test case 2: Empty queue operations
        System.out.println("Test 2: Empty queue");
        Task149 queue2 = new Task149();
        System.out.println("Peek empty: " + queue2.peek());
        System.out.println("Dequeue empty: " + queue2.dequeue());
        System.out.println("Is empty: " + queue2.isEmpty());
        System.out.println();
        
        // Test case 3: Queue with resize
        System.out.println("Test 3: Resize operations");
        Task149 queue3 = new Task149(3);
        for (int i = 1; i <= 5; i++) {
            queue3.enqueue(i * 10);
        }
        System.out.println("Size after enqueue: " + queue3.getSize());
        while (!queue3.isEmpty()) {
            System.out.println("Dequeue: " + queue3.dequeue());
        }
        System.out.println();
        
        // Test case 4: Circular behavior
        System.out.println("Test 4: Circular behavior");
        Task149 queue4 = new Task149(3);
        queue4.enqueue(1);
        queue4.enqueue(2);
        queue4.dequeue();
        queue4.enqueue(3);
        queue4.enqueue(4);
        System.out.println("Peek: " + queue4.peek());
        System.out.println("Size: " + queue4.getSize());
        System.out.println();
        
        // Test case 5: Multiple operations
        System.out.println("Test 5: Mixed operations");
        Task149 queue5 = new Task149();
        queue5.enqueue(100);
        System.out.println("Peek: " + queue5.peek());
        queue5.enqueue(200);
        queue5.enqueue(300);
        System.out.println("Dequeue: " + queue5.dequeue());
        queue5.enqueue(400);
        System.out.println("Size: " + queue5.getSize());
        System.out.println("Peek: " + queue5.peek());
    }
}
