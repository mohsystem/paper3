
import java.util.EmptyStackException;

class Task148 {
    private static class Stack {
        private Node top;
        private int size;
        
        private class Node {
            int data;
            Node next;
            
            Node(int data) {
                this.data = data;
                this.next = null;
            }
        }
        
        public Stack() {
            this.top = null;
            this.size = 0;
        }
        
        public void push(int data) {
            Node newNode = new Node(data);
            newNode.next = top;
            top = newNode;
            size++;
        }
        
        public int pop() {
            if (isEmpty()) {
                throw new EmptyStackException();
            }
            int data = top.data;
            top = top.next;
            size--;
            return data;
        }
        
        public int peek() {
            if (isEmpty()) {
                throw new EmptyStackException();
            }
            return top.data;
        }
        
        public boolean isEmpty() {
            return top == null;
        }
        
        public int getSize() {
            return size;
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Basic push and peek
        System.out.println("Test Case 1: Basic push and peek");
        Stack stack1 = new Stack();
        stack1.push(10);
        stack1.push(20);
        stack1.push(30);
        System.out.println("Peek: " + stack1.peek());
        System.out.println("Size: " + stack1.getSize());
        System.out.println();
        
        // Test Case 2: Pop operations
        System.out.println("Test Case 2: Pop operations");
        Stack stack2 = new Stack();
        stack2.push(5);
        stack2.push(15);
        stack2.push(25);
        System.out.println("Pop: " + stack2.pop());
        System.out.println("Pop: " + stack2.pop());
        System.out.println("Peek: " + stack2.peek());
        System.out.println("Size: " + stack2.getSize());
        System.out.println();
        
        // Test Case 3: Push and pop multiple elements
        System.out.println("Test Case 3: Push and pop multiple elements");
        Stack stack3 = new Stack();
        for (int i = 1; i <= 5; i++) {
            stack3.push(i * 10);
        }
        System.out.println("Peek: " + stack3.peek());
        while (!stack3.isEmpty()) {
            System.out.println("Pop: " + stack3.pop());
        }
        System.out.println("Is Empty: " + stack3.isEmpty());
        System.out.println();
        
        // Test Case 4: Single element operations
        System.out.println("Test Case 4: Single element operations");
        Stack stack4 = new Stack();
        stack4.push(100);
        System.out.println("Peek: " + stack4.peek());
        System.out.println("Pop: " + stack4.pop());
        System.out.println("Is Empty: " + stack4.isEmpty());
        System.out.println();
        
        // Test Case 5: Mixed operations
        System.out.println("Test Case 5: Mixed operations");
        Stack stack5 = new Stack();
        stack5.push(7);
        stack5.push(14);
        System.out.println("Peek: " + stack5.peek());
        stack5.push(21);
        System.out.println("Pop: " + stack5.pop());
        stack5.push(28);
        System.out.println("Peek: " + stack5.peek());
        System.out.println("Size: " + stack5.getSize());
    }
}
