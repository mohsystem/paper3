public class Task148 {

    static class Stack {
        private int arr[];
        private int top;
        private int capacity;

        // Constructor to initialize stack
        Stack(int size) {
            arr = new int[size];
            capacity = size;
            top = -1;
        }

        // Utility function to add an element x to the stack
        public void push(int x) {
            if (isFull()) {
                System.out.println("Stack Overflow");
                return;
            }
            System.out.println("Pushing " + x);
            arr[++top] = x;
        }

        // Utility function to pop a top element from the stack
        public int pop() {
            if (isEmpty()) {
                System.out.println("Stack Underflow");
                return Integer.MIN_VALUE;
            }
            int item = arr[top--];
            System.out.println("Popping " + item);
            return item;
        }

        // Utility function to return top element in a stack
        public int peek() {
            if (isEmpty()) {
                System.out.println("Stack is empty");
                return Integer.MIN_VALUE;
            }
            return arr[top];
        }

        // Utility function to check if the stack is full
        public Boolean isFull() {
            return top == capacity - 1;
        }

        // Utility function to check if the stack is empty
        public Boolean isEmpty() {
            return top == -1;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Stack Test ---");
        Stack stack = new Stack(3);

        // Test Case 1: Push elements
        stack.push(10);
        stack.push(20);
        stack.push(30);

        // Test Case 2: Peek at the top element
        System.out.println("Top element is: " + stack.peek());

        // Test Case 3: Pop an element
        stack.pop();
        System.out.println("Top element after pop is: " + stack.peek());

        // Test Case 4: Push to a full stack (Overflow)
        stack.push(40); // This should be fine
        stack.push(50); // This should cause an overflow

        // Test Case 5: Pop all elements and check for underflow
        stack.pop();
        stack.pop();
        stack.pop();
        stack.pop(); // This should cause an underflow
    }
}