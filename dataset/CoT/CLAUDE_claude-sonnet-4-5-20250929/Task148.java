
import java.util.EmptyStackException;

class Task148 {
    private static final int DEFAULT_CAPACITY = 10;
    private int[] stackArray;
    private int top;
    private int capacity;

    public Task148() {
        this(DEFAULT_CAPACITY);
    }

    public Task148(int capacity) {
        if (capacity <= 0) {
            throw new IllegalArgumentException("Capacity must be positive");
        }
        this.capacity = capacity;
        this.stackArray = new int[capacity];
        this.top = -1;
    }

    public void push(int value) {
        if (isFull()) {
            resize();
        }
        stackArray[++top] = value;
    }

    public int pop() {
        if (isEmpty()) {
            throw new EmptyStackException();
        }
        return stackArray[top--];
    }

    public int peek() {
        if (isEmpty()) {
            throw new EmptyStackException();
        }
        return stackArray[top];
    }

    public boolean isEmpty() {
        return top == -1;
    }

    public boolean isFull() {
        return top == capacity - 1;
    }

    public int size() {
        return top + 1;
    }

    private void resize() {
        int newCapacity = capacity * 2;
        int[] newArray = new int[newCapacity];
        System.arraycopy(stackArray, 0, newArray, 0, capacity);
        stackArray = newArray;
        capacity = newCapacity;
    }

    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Basic Push and Pop ===");
        Task148 stack1 = new Task148();
        stack1.push(10);
        stack1.push(20);
        stack1.push(30);
        System.out.println("Peek: " + stack1.peek());
        System.out.println("Pop: " + stack1.pop());
        System.out.println("Pop: " + stack1.pop());
        System.out.println("Size: " + stack1.size());

        System.out.println("\\n=== Test Case 2: Empty Stack Check ===");
        Task148 stack2 = new Task148();
        System.out.println("Is Empty: " + stack2.isEmpty());
        stack2.push(5);
        System.out.println("Is Empty: " + stack2.isEmpty());
        stack2.pop();
        System.out.println("Is Empty after pop: " + stack2.isEmpty());

        System.out.println("\\n=== Test Case 3: Multiple Operations ===");
        Task148 stack3 = new Task148();
        for (int i = 1; i <= 5; i++) {
            stack3.push(i * 10);
        }
        System.out.println("Stack size: " + stack3.size());
        System.out.println("Peek: " + stack3.peek());
        while (!stack3.isEmpty()) {
            System.out.println("Pop: " + stack3.pop());
        }

        System.out.println("\\n=== Test Case 4: Stack Resize ===");
        Task148 stack4 = new Task148(2);
        stack4.push(1);
        stack4.push(2);
        stack4.push(3);
        stack4.push(4);
        System.out.println("Size after resize: " + stack4.size());
        System.out.println("Peek: " + stack4.peek());

        System.out.println("\\n=== Test Case 5: Error Handling ===");
        Task148 stack5 = new Task148();
        try {
            stack5.pop();
        } catch (EmptyStackException e) {
            System.out.println("Caught exception: Cannot pop from empty stack");
        }
        try {
            stack5.peek();
        } catch (EmptyStackException e) {
            System.out.println("Caught exception: Cannot peek empty stack");
        }
    }
}
