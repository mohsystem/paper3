import java.util.ArrayList;
import java.util.EmptyStackException;
import java.util.List;

class IntStack {
    private List<Integer> items;

    /**
     * Constructs an empty stack.
     */
    public IntStack() {
        this.items = new ArrayList<>();
    }

    /**
     * Pushes an item onto the top of this stack.
     * @param item the item to be pushed onto this stack.
     */
    public void push(int item) {
        items.add(item);
    }

    /**
     * Removes the object at the top of this stack and returns that object as the value of this function.
     * @return The object at the top of this stack.
     * @throws EmptyStackException if this stack is empty.
     */
    public int pop() {
        if (isEmpty()) {
            throw new EmptyStackException();
        }
        return items.remove(items.size() - 1);
    }

    /**
     * Looks at the object at the top of this stack without removing it from the stack.
     * @return The object at the top of this stack.
     * @throws EmptyStackException if this stack is empty.
     */
    public int peek() {
        if (isEmpty()) {
            throw new EmptyStackException();
        }
        return items.get(items.size() - 1);
    }

    /**
     * Tests if this stack is empty.
     * @return {@code true} if and only if this stack contains no items; {@code false} otherwise.
     */
    public boolean isEmpty() {
        return items.isEmpty();
    }

    /**
     * Returns the number of items in this stack.
     * @return The number of items in this stack.
     */
    public int size() {
        return items.size();
    }
}

public class Task148 {
    public static void main(String[] args) {
        // Test Case 1: Push and Peek
        System.out.println("--- Test Case 1: Push and Peek ---");
        IntStack stack1 = new IntStack();
        stack1.push(10);
        stack1.push(20);
        stack1.push(30);
        System.out.println("Pushed 10, 20, 30. Top element (peek): " + stack1.peek());
        System.out.println("Stack size: " + stack1.size());

        // Test Case 2: Push and Pop
        System.out.println("\n--- Test Case 2: Push and Pop ---");
        IntStack stack2 = new IntStack();
        stack2.push(10);
        stack2.push(20);
        stack2.push(30);
        System.out.println("Popped element: " + stack2.pop());
        System.out.println("Top element after pop (peek): " + stack2.peek());
        System.out.println("Stack size: " + stack2.size());

        // Test Case 3: Mixed Operations
        System.out.println("\n--- Test Case 3: Mixed Operations ---");
        IntStack stack3 = new IntStack();
        stack3.push(10);
        stack3.push(20);
        System.out.println("Popped element: " + stack3.pop());
        stack3.push(30);
        System.out.println("Top element (peek): " + stack3.peek());
        System.out.println("Stack size: " + stack3.size());

        // Test Case 4: Pop from empty stack
        System.out.println("\n--- Test Case 4: Pop from empty stack ---");
        IntStack stack4 = new IntStack();
        try {
            stack4.pop();
        } catch (EmptyStackException e) {
            System.out.println("Successfully caught expected exception: Stack is empty");
        }

        // Test Case 5: Peek from empty stack
        System.out.println("\n--- Test Case 5: Peek from empty stack ---");
        IntStack stack5 = new IntStack();
        try {
            stack5.peek();
        } catch (EmptyStackException e) {
            System.out.println("Successfully caught expected exception: Stack is empty");
        }
    }
}